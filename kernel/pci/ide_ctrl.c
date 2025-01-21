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
 * pci/ide_ctrl.c
 *      IDE Controller
 */

#include <dancy.h>

struct ide_channel {
	event_t event;
	mtx_t mtx;

	phys_addr_t buffer;
	int init_lock;
	int command_regs;
	int control_reg;
	int bus_master;
	int irq;
	int dsel;
	int nr;
};

struct ide_device {
	struct ide_channel *channel;
	void *identify_data;
	unsigned int type;
	int nr;
};

static struct ide_channel ide_channels[2];
static struct ide_device ide_devices[4];

static uint64_t get_sector_count(struct ide_device *dev)
{
	uint8_t *p8 = (uint8_t *)dev->identify_data;
	uint16_t *p16 = (uint16_t *)((addr_t)(p8 + (83 * 2)));
	uint32_t *p32 = (uint32_t *)((addr_t)(p8 + (60 * 2)));
	uint64_t sectors = *p32;

	if ((*p16 & 0x400) != 0) {
		uint64_t *p64 = (uint64_t *)((addr_t)(p8 + (100 * 2)));
		uint64_t s48 = *p64;

		if (s48 != 0 && s48 <= 0xFFFFFFFFFFFFull) {
			if (sectors < s48)
				sectors = s48;
		}
	}

	return sectors;
}

static int wait_status(struct ide_channel *channel, unsigned int milliseconds)
{
	uint64_t current = timer_read();
	int status;
	int i, r = 0;

	for (i = 0; r == 0; i++) {
		if ((timer_read() - current) >= milliseconds)
			r = (i >= 1024) ? DE_BUSY : 0;

		status = (int)cpu_in8((uint16_t)(channel->control_reg + 0));

		/*
		 * Check the BSY and DRQ bits, and stop if both are unset.
		 */
		if ((status & 0x88) == 0)
			return 0;

		/*
		 * Check the DRQ bit, and read the data port if it was set.
		 */
		if ((status & 0x08) != 0)
			(void)cpu_in16((uint16_t)(channel->command_regs + 0));
	}

	return r;
}

static int ata_read_write(struct ide_device *dev,
	uint64_t lba, unsigned int count, int write_mode)
{
	struct ide_channel *channel = dev->channel;
	uint16_t bus_master_command = (uint16_t)(channel->bus_master + 0);
	uint16_t bus_master_status  = (uint16_t)(channel->bus_master + 2);
	uint16_t bus_master_pointer = (uint16_t)(channel->bus_master + 4);
	uint32_t *prd_table;
	uint8_t val;

	uint16_t ata_r2 = (uint16_t)(channel->command_regs + 2);
	uint16_t ata_r3 = (uint16_t)(channel->command_regs + 3);
	uint16_t ata_r4 = (uint16_t)(channel->command_regs + 4);
	uint16_t ata_r5 = (uint16_t)(channel->command_regs + 5);
	uint16_t ata_r6 = (uint16_t)(channel->command_regs + 6);
	uint16_t ata_r7 = (uint16_t)(channel->command_regs + 7);

	if (count == 0 || count > 127)
		return DE_OVERFLOW;

	/*
	 * Set the default command register value.
	 */
	cpu_out8(bus_master_command, 0x00);

	/*
	 * Clear "interrupt" and "error" bits by writing to them.
	 */
	val = cpu_in8(bus_master_status);
	cpu_out8(bus_master_status, (uint8_t)(val | 0x06u));

	/*
	 * Set the "read or write" bit.
	 */
	cpu_out8(bus_master_command, (uint8_t)(write_mode ? 0x00 : 0x08));

	/*
	 * Build the Physical Region Decriptor Table.
	 */
	prd_table = (uint32_t *)((void *)(dev->channel->buffer + 0xFFC0));
	prd_table[0] = (uint32_t)dev->channel->buffer;
	prd_table[1] = (uint32_t)(((count * 512) & 0xFFFF) | 0x80000000);

	/*
	 * Load the Physical Region Decriptor Table Pointer.
	 */
	cpu_out32(bus_master_pointer, (uint32_t)((phys_addr_t)prd_table));

	if (wait_status(channel, 1000))
		return DE_BUSY;

	if (!write_mode)
		memset((void *)channel->buffer, 0, (size_t)(count * 512));

	/*
	 * Prefer the 28-bit LBA mode.
	 */
	if (lba <= 0x0FFFFF00 || get_sector_count(dev) <= 0x0FFFFFFF) {
		int dsel = (dev->nr & 1);
		int device_head = 0xE0 | (dsel << 4);
		uint32_t lba28 = (uint32_t)lba;

		device_head |= (int)((lba28 >> 24) & 0x0F);
		cpu_out8(ata_r6, (uint8_t)device_head);

		if (channel->dsel != dsel) {
			channel->dsel = dsel;
			delay(1000);
		}

		if (wait_status(channel, 1000))
			return DE_BUSY;

		cpu_out8(ata_r2, (uint8_t)count);
		cpu_out8(ata_r3, (uint8_t)((lba28 >>  0) & 0xFF));
		cpu_out8(ata_r4, (uint8_t)((lba28 >>  8) & 0xFF));
		cpu_out8(ata_r5, (uint8_t)((lba28 >> 16) & 0xFF));

		if (wait_status(channel, 1000))
			return DE_BUSY;

		event_reset(channel->event);
		cpu_out8(ata_r7, (uint8_t)(write_mode ? 0xCA : 0xC8));

	} else {
		int dsel = (dev->nr & 1);
		int device_byte = 0x40 | (dsel << 4);

		cpu_out8(ata_r6, (uint8_t)device_byte);

		if (channel->dsel != dsel) {
			channel->dsel = dsel;
			delay(1000);
		}

		if (wait_status(channel, 1000))
			return DE_BUSY;

		cpu_out8(ata_r2, (uint8_t)(0x00));
		cpu_out8(ata_r3, (uint8_t)((lba >> 24) & 0xFF));
		cpu_out8(ata_r4, (uint8_t)((lba >> 32) & 0xFF));
		cpu_out8(ata_r5, (uint8_t)((lba >> 40) & 0xFF));

		cpu_out8(ata_r2, (uint8_t)count);
		cpu_out8(ata_r3, (uint8_t)((lba >>  0) & 0xFF));
		cpu_out8(ata_r4, (uint8_t)((lba >>  8) & 0xFF));
		cpu_out8(ata_r5, (uint8_t)((lba >> 16) & 0xFF));

		if (wait_status(channel, 1000))
			return DE_BUSY;

		event_reset(channel->event);
		cpu_out8(ata_r7, (uint8_t)(write_mode ? 0x35 : 0x25));
	}

	/*
	 * Set the start/stop bit.
	 */
	cpu_out8(bus_master_command, (uint8_t)(write_mode ? 0x01 : 0x09));

	/*
	 * Wait for an interrupt.
	 */
	event_wait(channel->event, 5000);

	/*
	 * Clear the start/stop bit.
	 */
	cpu_out8(bus_master_command, (uint8_t)(write_mode ? 0x00 : 0x08));

	/*
	 * Read the bus master status register.
	 */
	val = cpu_in8(bus_master_status);

	(void)cpu_in8(ata_r7);
	(void)wait_status(channel, 1000);

	/*
	 * Send the flush cache command if using the write mode.
	 */
	if (write_mode) {
		cpu_out8(ata_r7, 0xE7);
		(void)wait_status(channel, 1000);
	}

	/*
	 * Check the error and interrupt bits.
	 */
	if ((val & 2) != 0 || (val & 4) == 0)
		return write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;

	return 0;
}

static int read_write_common(struct vfs_node *node,
	uint64_t offset, size_t *size, addr_t buffer, int write_mode)
{
	struct ide_device *dev = node->internal_data;
	struct ide_channel *channel = dev->channel;
	size_t requested_size = *size;
	size_t transfer_size = 0;

	uint64_t sector_count = get_sector_count(dev);
	uint64_t lba = (offset / 512);
	int r = 0;

	*size = 0;

	if ((offset & 0x1FF) != 0 || (requested_size & 0x1FF) != 0)
		return DE_ALIGNMENT;

	if (mtx_lock(&channel->mtx) != thrd_success)
		return DE_UNEXPECTED;

	if (!channel->buffer) {
		if (!(channel->buffer = mm_alloc_pages(mm_addr24, 4)))
			return mtx_unlock(&channel->mtx), DE_MEMORY;

		pg_map_kernel(channel->buffer, 0x10000, pg_uncached);
		memset((void *)channel->buffer, 0, 0x10000);
	}

	while (transfer_size < requested_size) {
		unsigned int unit_size = 0xFE00;
		uint64_t size_diff = requested_size - transfer_size;
		void *dst, *src;

		if (unit_size > size_diff)
			unit_size = (unsigned int)size_diff;

		if (lba + (unit_size / 512) > sector_count) {
			if (lba >= sector_count)
				break;
			unit_size = 512;
			unit_size *= (unsigned int)(sector_count - lba);
		}

		if (write_mode) {
			dst = (void *)channel->buffer;
			src = (void *)((addr_t)(buffer + transfer_size));
			memcpy(dst, src, (size_t)unit_size);
		}

		r = ata_read_write(dev, lba, (unit_size / 512), write_mode);

		if (r) {
			unit_size = 512;
			r = ata_read_write(dev, lba, 1, write_mode);
			if (r)
				break;
		}

		if (!write_mode) {
			dst = (void *)((addr_t)(buffer + transfer_size));
			src = (void *)channel->buffer;
			memcpy(dst, src, (size_t)unit_size);
		}

		lba += (unit_size / 512);
		transfer_size += unit_size;
	}

	mtx_unlock(&channel->mtx);

	return *size = transfer_size, r;
}

static void ide_irq_func(int irq, void *arg)
{
	struct ide_channel *channel = arg;
	uint16_t bus_master_status = (uint16_t)(channel->bus_master + 2);

	(void)irq;

	/*
	 * Check the interrupt bit and signal the event if it was set.
	 */
	if ((cpu_in8(bus_master_status) & 4) != 0)
		event_signal(channel->event);
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
	addr_t addr = (addr_t)buffer;
	return read_write_common(node, offset, size, addr, 0);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	addr_t addr = (addr_t)buffer;
	return read_write_common(node, offset, size, addr, 1);
}

static int n_sync(struct vfs_node *node)
{
	(void)node;
	return 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct ide_device *dev = node->internal_data;
	struct ide_channel *channel = dev->channel;

	memset(stat, 0, sizeof(*stat));

	if (mtx_lock(&channel->mtx) != thrd_success)
		return DE_UNEXPECTED;

	stat->size = get_sector_count(dev) * 512;
	stat->block_size = 512;

	mtx_unlock(&channel->mtx);

	return 0;
}

static int mount_drive(int dsel, const char *name)
{
	struct vfs_node *node;
	int r;

	if ((r = vfs_open(name, &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	if ((node = malloc(sizeof(*node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(node, 0);

	node->count = 1;
	node->type = vfs_type_block;

	node->internal_data = &ide_devices[dsel];
	node->n_release = n_release;

	node->n_read  = n_read;
	node->n_write = n_write;
	node->n_sync  = n_sync;
	node->n_stat  = n_stat;

	r = vfs_mount(name, node);
	node->n_release(&node);

	return r;
}

static int deploy_channel(struct ide_channel *channel)
{
	int state = 0;
	int i, j, k;

	wait_status(channel, 10);
	cpu_out8((uint16_t)(channel->command_regs + 4), 0);
	cpu_out8((uint16_t)(channel->command_regs + 5), 0);

	/*
	 * Use the SRST bit in the device control register.
	 */
	cpu_out8((uint16_t)channel->control_reg, 0x04), task_sleep(4);
	cpu_out8((uint16_t)channel->control_reg, 0x00), task_sleep(4);

	for (i = 0; i < 2; i++) {
		uint8_t val = (uint8_t)((i == 0) ? 0xA0 : 0xB0);
		unsigned int type;
		uint8_t id[2];

		if (wait_status(channel, 50))
			continue;

		cpu_out8((uint16_t)(channel->command_regs + 6), val);
		delay(1000);

		id[0] = cpu_in8((uint16_t)(channel->command_regs + 4));
		id[1] = cpu_in8((uint16_t)(channel->command_regs + 5));

		type = ((unsigned int)id[1] << 8) | (unsigned int)id[0];
		ide_devices[(channel->nr * 2) + i].type = type;
	}

	for (i = 0; i < 2; i++) {
		uint8_t val = (uint8_t)((i == 0) ? 0xA0 : 0xB0);
		unsigned int type = ide_devices[(channel->nr * 2) + i].type;
		uint16_t *identify = NULL;

		if (type == UINT_MAX)
			continue;

		ide_devices[(channel->nr * 2) + i].type = UINT_MAX;

		if (wait_status(channel, 50))
			continue;

		cpu_out8((uint16_t)(channel->command_regs + 6), val);
		delay(1000);

		cpu_out8((uint16_t)(channel->command_regs + 2), 0);
		cpu_out8((uint16_t)(channel->command_regs + 3), 0);
		cpu_out8((uint16_t)(channel->command_regs + 4), 0);
		cpu_out8((uint16_t)(channel->command_regs + 5), 0);

		if (wait_status(channel, 50))
			continue;

		/*
		 * Send the IDENTIFY or IDENTIFY PACKET DEVICE command.
		 */
		val = (uint8_t)((type != 0xEB14) ? 0xEC : 0xA1);
		cpu_out8((uint16_t)(channel->command_regs + 7), val);
		delay(1000);

		for (j = 0; /* void */; j++) {
			uint8_t s;

			s = cpu_in8((uint16_t)(channel->command_regs + 7));

			if (j > 50 || s == 0x00 || (s & 0x01) != 0x00) {
				type = UINT_MAX;
				break;
			}

			if ((s & 0x88) == 0x08) {
				int r = channel->command_regs + 0;

				identify = calloc(256, sizeof(uint16_t));

				if (!identify)
					return DE_MEMORY;

				for (k = 0; k < 256; k++)
					identify[k] = cpu_in16((uint16_t)r);

				(void)wait_status(channel, 50);
				break;
			}

			cpu_halt(1);
		}

		ide_devices[(channel->nr * 2) + i].identify_data = identify;
		ide_devices[(channel->nr * 2) + i].type = type;

		if (type != UINT_MAX)
			state += 1;
	}

	if (state) {
		channel->event = event_create(event_type_manual_reset);

		if (!channel->event)
			return DE_MEMORY;

		if (mtx_init(&channel->mtx, mtx_plain) != thrd_success)
			return DE_UNEXPECTED;

		if (!irq_install(channel->irq, channel, ide_irq_func))
			return DE_UNEXPECTED;

		irq_enable(channel->irq);
	}

	return 0;
}

static int init_channels(struct pci_id *pci)
{
	struct ide_channel *primary = &ide_channels[0];
	struct ide_channel *secondary = &ide_channels[1];
	uint32_t cmd = pci_read(pci, 0x04);
	int bmiba = (int)(pci_read(pci, 0x20) & 0xFFFF);
	int i, r;

	if ((cmd & 1) == 0)
		return DE_UNSUPPORTED;

	if ((bmiba & 1) == 0 || (bmiba & 0xFFFC) == 0)
		return DE_UNSUPPORTED;

	if (!spin_trylock(&primary->init_lock))
		return DE_UNSUPPORTED;

	if (!spin_trylock(&secondary->init_lock))
		return DE_UNSUPPORTED;

	pci_write(pci, 0x04, (cmd | 4));

	primary->command_regs = 0x1F0;
	primary->control_reg = 0x3F6;
	primary->bus_master = (bmiba & 0xFFFC) + 0;
	primary->irq = 14;
	primary->dsel = 0;
	primary->nr = 0;

	secondary->command_regs = 0x170;
	secondary->control_reg = 0x376;
	secondary->bus_master = (bmiba & 0xFFFC) + 8;
	secondary->irq = 15;
	secondary->dsel = 0;
	secondary->nr = 1;

	for (i = 0; i < 4; i++) {
		ide_devices[i].channel = (i < 2) ? primary : secondary;
		ide_devices[i].type = UINT_MAX;
		ide_devices[i].nr = i;
	}

	if ((r = deploy_channel(primary)) != 0)
		return r;

	if ((r = deploy_channel(secondary)) != 0)
		return r;

	for (i = 0; i < 4; i++) {
		char buf[16];

		if (ide_devices[i].type == 0x0000) {
			snprintf(&buf[0], 16, "/dev/hd%c", ('a' + i));
			if ((r = mount_drive(i, &buf[0])) != 0)
				return r;
		}
	}

	return 0;
}

static int ide_ctrl_init(struct pci_id *pci)
{
	int interface = (int)(pci->class_code & 0xFF);
	int r = 0;

	if ((interface & 0x85) != 0x80 || (r = init_channels(pci)) != 0) {
		/*
		 * Disable the PCI device if it was not compatible or
		 * some other device already initialized the channels.
		 */
		uint32_t cmd = pci_read(pci, 0x04);
		pci_write(pci, 0x04, (cmd & 0xFFFF0000));
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(ide_ctrl_init, -1, -1, 0x01, 0x01, -1);
