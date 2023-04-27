/*
 * Copyright (c) 2022 Antti Tiihala
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * misc/floppy.c
 *      The floppy disk driver
 */

#include <dancy.h>

static event_t floppy_event;
static uint32_t floppy_state;
static mtx_t floppy_mtx;

struct floppy_table {
	int sector_size;
	int cylinders;
	int heads;
	int sectors_per_track;

	int rate;
	int w_gpl;
	int w_srt;
	int w_hut;
};

static const struct floppy_table table_default[] = {
	{  512, 80, 2, 18, 0x00, 0x1B, 12, 15 },
	{  512, 80, 2,  9, 0x02, 0x2A, 13, 15 }
};

static struct {
	int type;
	int media_changed;
	uint32_t motor_ticks;
	struct floppy_table table;
} drive_data[2];

static int drive_data_lock;

static uint8_t fdc0_dor_value = 0x0C;
#define FDC0_CURRENT_DSEL ((int)(fdc0_dor_value & 1))

static const uint16_t fdc0_dor = 0x3F2;
static const uint16_t fdc0_msr = 0x3F4;
static const uint16_t fdc0_dsr = 0x3F4;
static const uint16_t fdc0_dat = 0x3F5;
static const uint16_t fdc0_ccr = 0x3F7;
static const uint16_t fdc0_dir = 0x3F7;

static void start_motor_and_select(int dsel);

static int cmos(int reg)
{
	uint8_t reg_with_nmi_enabled = (uint8_t)(reg & 0x7F);

	cpu_out8(0x70, reg_with_nmi_enabled);
	delay(1000);

	return (int)cpu_in8(0x71);
}

static void short_delay(void)
{
	cpu_halt(1);
}

static int floppy_task(void *arg)
{
	void *lock_local = &drive_data_lock;

	while (!arg) {
		unsigned int value;

		spin_enter(&lock_local);
		value = fdc0_dor_value;

		if ((value & 0x10) != 0) {
			if (!(timer_ticks - drive_data[0].motor_ticks < 5000))
				value &= ~(0x10u);
		}

		if ((value & 0x20) != 0) {
			if (!(timer_ticks - drive_data[1].motor_ticks < 5000))
				value &= ~(0x20u);
		}

		if (fdc0_dor_value != value) {
			fdc0_dor_value = (uint8_t)value;
			cpu_out8(fdc0_dor, fdc0_dor_value);
		}

		spin_leave(&lock_local);
		task_sleep(2000);
	}

	return 0;
}

static void irq6_func(int irq, void *arg)
{
	(void)irq;
	(void)arg;

	event_signal(floppy_event);
}

static void prepare_wait_irq6(void)
{
	event_reset(floppy_event);
}

static void wait_irq6(void)
{
	event_wait(floppy_event, 5000);
}

static int fdc0_receive_byte(void)
{
	int i;

	for (i = 0; i < 1000; i++) {
		int value = (int)(cpu_in8(fdc0_msr) & 0xD0);

		if (value == 0xD0)
			return (int)cpu_in8(fdc0_dat);
		if (value == 0x80)
			break;

		short_delay();
	}

	return -1;
}

static void fdc0_send_byte(int byte)
{
	int i;

	for (i = 0; i < 1000; i++) {
		int value = (int)(cpu_in8(fdc0_msr) & 0xC0);

		if (value == 0x80) {
			cpu_out8(fdc0_dat, (uint8_t)byte);
			break;
		}
		if (value == 0xC0)
			(void)fdc0_receive_byte();

		short_delay();
	}

	short_delay();
}

static int fdc0_media_changed(void)
{
	/*
	 * The caller is responsible for starting up the motor.
	 */
	if ((cpu_in8(fdc0_dir) & 0x80) != 0)
		drive_data[FDC0_CURRENT_DSEL].media_changed = 1;

	return drive_data[FDC0_CURRENT_DSEL].media_changed;
}

static unsigned int fdc0_sense_interrupt_status(void)
{
	int r0, r1;

	fdc0_send_byte(0x08);

	r0 = fdc0_receive_byte();
	r1 = fdc0_receive_byte();

	if (r0 < 0 || r1 < 0)
		return 0;

	return (unsigned int)r0 | (unsigned int)(r1 << 8);
}

static void fdc0_recalibrate(void)
{
	int i;

	start_motor_and_select(FDC0_CURRENT_DSEL);

	for (i = 0; i < 4; i++) {
		prepare_wait_irq6();
		fdc0_send_byte(0x07);
		fdc0_send_byte((int)(fdc0_dor_value & 0x03));
		wait_irq6();

		if ((fdc0_sense_interrupt_status() & 0x20) != 0)
			break;
	}
}

static void fdc0_configure(void)
{
	int dsel = FDC0_CURRENT_DSEL;

	cpu_out8(fdc0_dsr, (uint8_t)drive_data[dsel].table.rate);
	cpu_out8(fdc0_ccr, (uint8_t)drive_data[dsel].table.rate);

	/*
	 * Send the SPECIFY command.
	 */
	{
		int w_srt = drive_data[dsel].table.w_srt;
		int w_hut = drive_data[dsel].table.w_hut;
		const int w_hlt = 5;

		fdc0_send_byte(0x03);
		fdc0_send_byte((w_srt << 4) | w_hut);
		fdc0_send_byte(w_hlt << 1);
	}

	/*
	 * Send the CONFIGURE command.
	 */
	{
		const int w_fifothr = 8;

		fdc0_send_byte(0x13);
		fdc0_send_byte(0);
		fdc0_send_byte(0x50 | w_fifothr);
		fdc0_send_byte(0);
	}
}

static void fdc0_reset(void)
{
	int i;

	cpu_out8(fdc0_dor, 0);
	short_delay();

	prepare_wait_irq6();
	cpu_out8(fdc0_dor, fdc0_dor_value);
	wait_irq6();

	for (i = 0; i < 4; i++)
		fdc0_sense_interrupt_status();

	fdc0_configure();
}

static void fdc0_seek(int c)
{
	start_motor_and_select(FDC0_CURRENT_DSEL);

	prepare_wait_irq6();

	fdc0_send_byte(0x0F);
	fdc0_send_byte(FDC0_CURRENT_DSEL);
	fdc0_send_byte(c);

	wait_irq6();
	fdc0_sense_interrupt_status();
}

static int fdc0_read_write(int c, int h, int s, int write_mode)
{
	int dsel = FDC0_CURRENT_DSEL;
	int w_eot = drive_data[dsel].table.sectors_per_track;
	int w_gpl = drive_data[dsel].table.w_gpl;
	int w_n = 2;
	int i, st0, st1;

	start_motor_and_select(dsel);

	prepare_wait_irq6();

	if (write_mode)
		fdc0_send_byte(0x45);
	else
		fdc0_send_byte(0x46);

	fdc0_send_byte((h << 2) | dsel);
	fdc0_send_byte(c);
	fdc0_send_byte(h);
	fdc0_send_byte(s);

	for (i = 0; i < 8; i++) {
		if ((128 << i) == drive_data[dsel].table.sector_size) {
			w_n = i;
			break;
		}
	}

	fdc0_send_byte(w_n);
	fdc0_send_byte((s < w_eot) ? w_eot : s);
	fdc0_send_byte(w_gpl);
	fdc0_send_byte(0xFF);

	wait_irq6();

	st0 = fdc0_receive_byte();
	st1 = fdc0_receive_byte();

	for (i = 2; i < 7; i++)
		(void)fdc0_receive_byte();

	fdc0_sense_interrupt_status();

	if ((st0 & 0xC0) != 0) {
		if ((st1 & 0x01) != 0)
			return DE_ADDRESS_MARK;
		if ((st1 & 0x02) != 0)
			return DE_READ_ONLY;
		if ((st1 & 0x04) != 0)
			return DE_EMPTY;
		if ((st1 & 0x10) != 0)
			return DE_OVERRUN;
		if ((st1 & 0x20) != 0)
			return DE_CHECKSUM;

		return write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;
	}

	return 0;
}

static int fdc0_read(int c, int h, int s, size_t size, void *buffer)
{
	phys_addr_t addr;
	int r;

	if ((addr = dma_set_floppy_read(size)) == 0)
		return DE_MEMORY;

	memset((void *)addr, 0, size);

	if (fdc0_media_changed())
		return DE_MEDIA_CHANGED;

	if ((r = fdc0_read_write(c, h, s, 0)) != 0)
		return r;

	memcpy(buffer, (const void *)addr, size);

	return 0;
}

static int fdc0_write(int c, int h, int s, size_t size, const void *buffer)
{
	phys_addr_t addr;
	int r;

	if ((addr = dma_set_floppy_write(size)) == 0)
		return DE_MEMORY;

	memcpy((void *)addr, buffer, size);

	if (fdc0_media_changed())
		return DE_MEDIA_CHANGED;

	if ((r = fdc0_read_write(c, h, s, 1)) != 0)
		return r;

	return 0;
}

static void read_bios_parameter_block(int dsel, unsigned char *buffer)
{
	int sector_size, cylinders, heads, sectors_per_track;

	if (LE16(&buffer[510]) != 0xAA55)
		return;

	sector_size = (int)LE16(&buffer[11]);
	sectors_per_track = (int)LE16(&buffer[24]);
	heads = (int)LE16(&buffer[26]);

	if (sector_size != 512)
		return;

	if (sectors_per_track < 8 || sectors_per_track > 63)
		return;

	if (heads < 1 || heads > 2)
		return;

	cylinders = ((int)LE16(&buffer[19]) / heads) / sectors_per_track;

	if (cylinders < 40 || cylinders > 127)
		return;

	drive_data[dsel].table.sector_size = sector_size;
	drive_data[dsel].table.cylinders = cylinders;
	drive_data[dsel].table.heads = heads;
	drive_data[dsel].table.sectors_per_track = sectors_per_track;
}

static int prepare_transfer(int dsel)
{
	static unsigned char buffer[1024];
	int i, r = 0;

	if (floppy_state < 1 || dsel < 0 || dsel > 1)
		return DE_UNSUPPORTED;

	if (!drive_data[dsel].type)
		return DE_UNSUPPORTED;

	start_motor_and_select(dsel);

	if (floppy_state == 1 || fdc0_media_changed()) {
		size_t count = sizeof(table_default) / sizeof(*table_default);
		size_t s = sizeof(struct floppy_table);
		int r_directive = 0;

		int debut = !(drive_data[dsel].type & 0x1000);
		drive_data[dsel].type |= 0x1000;

		/*
		 * Try different floppy parameters. If the first 1024
		 * bytes can be read, assume that everything is OK.
		 *
		 * The first error code is the best approximation if
		 * all reads seem to fail.
		 */
		for (i = 0; i < (int)count; i++) {
			memcpy(&drive_data[dsel].table, &table_default[i], s);

			fdc0_reset();
			fdc0_recalibrate();

			fdc0_seek(1), fdc0_seek(0);

			drive_data[dsel].media_changed = 0;

			if ((r = fdc0_read(0, 0, 1, 1024, &buffer[0])) == 0)
				break;

			if (!r_directive)
				r_directive = r;

			if ((r = fdc0_read(0, 0, 1, 1024, &buffer[0])) == 0)
				break;
		}

		if (r) {
			memset(&drive_data[dsel].table, 0, s);
			drive_data[dsel].media_changed = 1;
			return r_directive;
		}

		read_bios_parameter_block(dsel, &buffer[0]);

		/*
		 * Do not report the "media changed" error when
		 * using the drive for the first time.
		 */
		if (!debut)
			r = DE_MEDIA_CHANGED;

		floppy_state = 2;
	}

	return r;
}

static int do_transfer(int offset, size_t *size, addr_t addr, int write_mode)
{
	uint8_t *buffer = (uint8_t *)addr;
	size_t requested_size = *size;
	size_t transfer_size = 0;

	int dsel = FDC0_CURRENT_DSEL;
	int r = 0;

	int sector_size = drive_data[dsel].table.sector_size;
	int cylinders = drive_data[dsel].table.cylinders;
	int heads = drive_data[dsel].table.heads;
	int sectors_per_track = drive_data[dsel].table.sectors_per_track;
	int sectors_per_cylinder = (heads * sectors_per_track);

	*size = transfer_size;

	if (!sector_size || !sectors_per_cylinder)
		return DE_UNEXPECTED;

	if ((offset & (sector_size - 1)) != 0)
		return DE_ALIGNMENT;

	if ((requested_size & (size_t)(sector_size - 1)) != 0)
		return DE_ALIGNMENT;

	while (transfer_size < requested_size) {
		size_t track_size = (size_t)sector_size;
		int retry_count = 5;

		int lba = offset / sector_size;
		int c = (lba / sectors_per_cylinder);
		int h = (lba % sectors_per_cylinder) / sectors_per_track;
		int s = (lba % sectors_per_track) + 1;

		if (c >= cylinders)
			break;

		track_size *= (size_t)(sectors_per_track - (s - 1));

		if (track_size > (requested_size - transfer_size))
			track_size = (requested_size - transfer_size);

		while (retry_count--) {
			if (!write_mode)
				r = fdc0_read(c, h, s, track_size, buffer);
			else
				r = fdc0_write(c, h, s, track_size, buffer);

			if (!r || r == DE_MEDIA_CHANGED)
				break;

			/*
			 * Alternatively, try to read a single sector.
			 */
			track_size = (size_t)sector_size;

			/*
			 * Try to reset the controller.
			 */
			if (retry_count == 2)
				fdc0_reset();
		}

		if (r)
			break;

		buffer += (int)track_size;
		offset += (int)track_size;

		transfer_size += track_size;
	}

	return *size = transfer_size, r;
}

static void start_motor_and_select(int dsel)
{
	void *lock_local = &drive_data_lock;
	int dsel_changed = 0;
	int motor_delay = 0;
	unsigned int value;

	spin_enter(&lock_local);
	value = fdc0_dor_value;

	if (dsel == 0) {
		drive_data[0].motor_ticks = timer_ticks;
		value = ((value | 0x10u) & 0xFCu) | 0u;
	}

	if (dsel == 1) {
		drive_data[1].motor_ticks = timer_ticks;
		value = ((value | 0x20u) & 0xFCu) | 1u;
	}

	if ((fdc0_dor_value & 0x03u) != (value & 0x03u))
		dsel_changed = 1;

	if ((fdc0_dor_value & 0xF0u) != (value & 0xF0u))
		motor_delay = 1;

	if (fdc0_dor_value != value) {
		fdc0_dor_value = (uint8_t)value;
		cpu_out8(fdc0_dor, fdc0_dor_value);
	}

	spin_leave(&lock_local);

	if (dsel_changed)
		fdc0_configure();

	if (motor_delay)
		task_sleep(400);
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;

	*node = NULL;

	if (vfs_decrement_count(n) == 0) {
		memset(n, 0, sizeof(*n));
		free(n);
	}
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	int dsel = (node->internal_data == &drive_data[0]) ? 0 : 1;

	return floppy_read(dsel, offset, size, buffer);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	int dsel = (node->internal_data == &drive_data[0]) ? 0 : 1;

	return floppy_write(dsel, offset, size, buffer);
}

static int n_sync(struct vfs_node *node)
{
	int dsel = (node->internal_data == &drive_data[0]) ? 0 : 1;

	return floppy_test(dsel);
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	int dsel = (node->internal_data == &drive_data[0]) ? 0 : 1;
	int r;

	memset(stat, 0, sizeof(*stat));

	if (mtx_lock(&floppy_mtx) != thrd_success)
		return DE_UNEXPECTED;

	if (drive_data[dsel].media_changed) {
		if ((r = prepare_transfer(dsel)) != 0) {
			mtx_unlock(&floppy_mtx);
			return r;
		}
	}

	if (drive_data[dsel].type) {
		int sector_size = drive_data[dsel].table.sector_size;
		uint32_t size = (uint32_t)sector_size;

		size *= (uint32_t)drive_data[dsel].table.sectors_per_track;
		size *= (uint32_t)drive_data[dsel].table.heads;
		size *= (uint32_t)drive_data[dsel].table.cylinders;

		stat->size = (uint64_t)size;
		stat->block_size = (size_t)sector_size;
	}

	mtx_unlock(&floppy_mtx);

	return 0;
}

static int mount_floppy(int dsel, const char *name)
{
	struct vfs_node *dev_node, *node;
	int r;

	if ((dev_node = malloc(sizeof(*dev_node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(dev_node, 0);

	dev_node->count = 1;
	dev_node->type = vfs_type_block;

	dev_node->internal_data = &drive_data[dsel];
	dev_node->n_release = n_release;

	dev_node->n_read  = n_read;
	dev_node->n_write = n_write;
	dev_node->n_sync  = n_sync;
	dev_node->n_stat  = n_stat;

	if ((r = vfs_open("/mnt/", &node, 0, vfs_mode_create)) != 0) {
		dev_node->n_release(&dev_node);
		return r;
	}

	node->n_release(&node);

	if ((r = vfs_open(name, &node, 0, vfs_mode_create)) != 0) {
		dev_node->n_release(&dev_node);
		return r;
	}

	node->n_release(&node);

	if ((r = fat_io_create(&node, dev_node)) != 0) {
		dev_node->n_release(&dev_node);
		return r;
	}

	/*
	 * The fat_io_create function has increased the reference count.
	 */
	dev_node->n_release(&dev_node);

	if ((r = vfs_mount(name, node)) != 0)
		return r;

	node->n_release(&node);

	return 0;
}

int floppy_init(void)
{
	static int run_once;
	const int irq6 = 6;
	int r, reg;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&floppy_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	drive_data[0].media_changed = 1;
	drive_data[1].media_changed = 1;

	reg = cmos(0x10);
	drive_data[0].type = (reg >> 4) & 0x0F;
	drive_data[1].type = reg & 0x0F;

	if (!drive_data[0].type && !drive_data[1].type)
		return 0;

	floppy_event = event_create(event_type_manual_reset);

	if (!floppy_event)
		return DE_MEMORY;

	if (!irq_install(irq6, NULL, irq6_func))
		return DE_UNEXPECTED;

	irq_enable(irq6);

	/*
	 * Send the VERSION command.
	 */
	fdc0_send_byte(0x10);
	if (fdc0_receive_byte() != 0x90)
		return 0;

	/*
	 * Send the (UN)LOCK command.
	 */
	fdc0_send_byte(0x14);
	if (fdc0_receive_byte() < 0)
		return 0;

	if (!task_create(floppy_task, NULL, task_detached))
		return DE_MEMORY;

	cpu_add32(&floppy_state, 1);

	if (drive_data[0].type) {
		if ((r = mount_floppy(0, "/mnt/drive_a/")) != 0)
			return r;
	}

	if (drive_data[1].type) {
		if ((r = mount_floppy(1, "/mnt/drive_b/")) != 0)
			return r;
	}

	return 0;
}

int floppy_read(int dsel, uint64_t offset, size_t *size, void *buffer)
{
	int r;

	if (offset > (uint64_t)(INT_MAX))
		return *size = 0, 0;

	if (mtx_lock(&floppy_mtx) != thrd_success)
		return *size = 0, DE_UNEXPECTED;

	if ((r = prepare_transfer(dsel)) != 0) {
		mtx_unlock(&floppy_mtx);
		return *size = 0, r;
	}

	r = do_transfer((int)offset, size, (addr_t)buffer, 0);
	mtx_unlock(&floppy_mtx);

	return r;
}

int floppy_write(int dsel, uint64_t offset, size_t *size, const void *buffer)
{
	int r;

	if (offset > (uint64_t)(INT_MAX))
		return *size = 0, 0;

	if (mtx_lock(&floppy_mtx) != thrd_success)
		return *size = 0, DE_UNEXPECTED;

	if ((r = prepare_transfer(dsel)) != 0) {
		mtx_unlock(&floppy_mtx);
		return *size = 0, r;
	}

	r = do_transfer((int)offset, size, (addr_t)buffer, 1);
	mtx_unlock(&floppy_mtx);

	return r;
}

int floppy_test(int dsel)
{
	int r;

	if (dsel < 0 || dsel > 1)
		return DE_UNSUPPORTED;

	if (mtx_lock(&floppy_mtx) != thrd_success)
		return DE_UNEXPECTED;

	r = prepare_transfer(dsel);
	mtx_unlock(&floppy_mtx);

	return r;
}
