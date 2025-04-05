/*
 * Copyright (c) 2025 Antti Tiihala
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
 * pci/ahci.c
 *      Advanced Host Controller Interface
 */

#include <dancy.h>

struct ahci_port {
	uint8_t *base;
	void *buffer_cmd;
	void *buffer_fis;
	void *buffer_ct;
	void *buffer_io;

	uint64_t disk_size;
	uint32_t signature;

	int lock;
	int sata_available;
	void *ahci;
};

struct ahci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint32_t hba_cap[2];
	uint32_t hba_pi;
	uint32_t hba_vs;

	void *hba_ghc;
	void *hba_is;

	struct ahci_port ports[32];
	event_t event;
};

static int mount_drive(struct ahci_port *port);

static void *ahci_alloc(size_t size)
{
	if (size <= 0x01000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 0);
		return pg_map_kernel(addr, 0x01000, pg_uncached);
	}

	if (size <= 0x02000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 1);
		return pg_map_kernel(addr, 0x02000, pg_uncached);
	}

	if (size <= 0x10000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 4);
		return pg_map_kernel(addr, 0x10000, pg_uncached);
	}

	return NULL;
}

static void ahci_irq_func(int irq, void *arg)
{
	struct ahci *ahci = arg;

	(void)irq;

	event_signal(ahci->event);
}

static int ahci_get_slot(struct ahci *ahci, struct ahci_port *port,
	uint32_t **command_header, uint32_t **command_table)
{
	int i, ncs = (int)((ahci->hba_cap[0] >> 8) & 0x1F) + 1;

	uint32_t ds = cpu_read32(port->base + 0x34);
	uint32_t ci = cpu_read32(port->base + 0x38);

	if (command_header)
		*command_header = NULL;

	if (command_table)
		*command_table = NULL;

	for (i = 0; i < ncs; i++) {
		unsigned int shl = (unsigned int)i;

		if ((ds & (1u << shl)) == 0 && (ci & (1u << shl)) == 0) {
			uint32_t *p;

			if (command_header) {
				p = port->buffer_cmd;
				p += (i * 8);
				*command_header = p;
			}

			if (command_table) {
				p = port->buffer_ct;
				p += (i * 64);
				*command_table = p;
			}

			return i;
		}
	}

	return -1;
}

static uint64_t ahci_get_disk_size(const void *identify_data)
{
	const uint8_t *p8 = identify_data;
	const uint16_t *p16 = (uint16_t *)((addr_t)(p8 + (83 * 2)));
	const uint32_t *p32 = (uint32_t *)((addr_t)(p8 + (60 * 2)));
	uint64_t sectors = *p32;

	if ((*p16 & 0x400) != 0) {
		uint64_t *p64 = (uint64_t *)((addr_t)(p8 + (100 * 2)));
		uint64_t s48 = *p64;

		if (s48 != 0 && s48 <= 0xFFFFFFFFFFFFull) {
			if (sectors < s48)
				sectors = s48;
		}
	}

	return (sectors * 512);
}

static int ahci_identify(struct ahci *ahci, struct ahci_port *port)
{
	uint32_t val, *ch, *ct;
	int i, slot, r = 0;

	if ((slot = ahci_get_slot(ahci, port, &ch, &ct)) < 0)
		return DE_BUSY;

	/*
	 * Clear the port interrupt status bits (RWC).
	 */
	cpu_write32(port->base + 0x10, cpu_read32(port->base + 0x10));

	/*
	 * Clear the interrupt status bits (RWC).
	 */
	cpu_write32(ahci->hba_is, cpu_read32(ahci->hba_is));

	/*
	 * Modify the first command header.
	 */
	{
		const uint32_t cfl = 5, w = 0;

		val = ch[0] & 0xFFFFFFB0u;

		val |= (cfl << 0);
		val |= (w << 6);

		ch[0] = val;
	}

	/*
	 * Modify the "Command FIS" structure.
	 */
	{
		uint8_t *cfis = (void *)((addr_t)(&ct[0]));

		memset(cfis, 0, 64);

		cfis[0] = 0x27;
		cfis[1] = 0x80;
		cfis[2] = 0xEC;
		cfis[7] = 0x40;
	}

	/*
	 * Modify the first physical region descriptor.
	 */
	{
		uint32_t *prdt = &ct[32];
		const uint32_t dbc = 511;

		prdt[0] = (uint32_t)((phys_addr_t)port->buffer_io);
		prdt[1] = 0;

		val = prdt[3] & 0x7FC00000u;

		val |= (dbc << 0);
		val |= (1u << 31);

		prdt[3] = val;
	}

	/*
	 * Check the task file data register.
	 */
	for (i = 0; /* void */; i++) {
		const uint32_t sts_drq = (1u << 3);
		const uint32_t sts_bsy = (1u << 7);

		val = cpu_read32(port->base + 0x20);

		if ((val & sts_drq) == 0 && (val & sts_bsy) == 0)
			break;

		if (i == 250)
			return DE_BUSY;

		task_sleep(10);
	}

	/*
	 * Write the command issue register.
	 */
	{
		uint32_t shl = (uint32_t)slot;

		cpu_write32(port->base + 0x38, (1u << shl));
	}

	/*
	 * Wait for the command.
	 */
	for (i = 0; /* void */; i++) {
		uint32_t shl = (uint32_t)slot;
		uint32_t ci = cpu_read32(port->base + 0x38);
		uint32_t is = cpu_read32(port->base + 0x10);

		if ((is & (1u << 30)) != 0) {
			printk("[AHCI] SATA Identify Error (TFEE)\n");
			r = DE_UNSUPPORTED;
			break;
		}

		if ((ci & (1u << shl)) == 0)
			break;

		if (i == 250) {
			printk("[AHCI] SATA Identify Error (Timeout)\n");
			r = DE_UNSUPPORTED;
			break;
		}

		task_sleep(10);
	}

	/*
	 * Clear the port interrupt status bits (RWC).
	 */
	cpu_write32(port->base + 0x10, cpu_read32(port->base + 0x10));

	/*
	 * Clear the interrupt status bits (RWC).
	 */
	cpu_write32(ahci->hba_is, cpu_read32(ahci->hba_is));

	return r;
}

static int ahci_read_write(struct ahci *ahci, struct ahci_port *port,
	uint64_t lba, unsigned int count, int write_mode)
{
	uint32_t val, *ch, *ct;
	int i, slot, r = 0;

	if (lba > 0x0000FFFFFFFFFFFFull || count == 0)
		return write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;

	if ((slot = ahci_get_slot(ahci, port, &ch, &ct)) < 0)
		return write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;

	/*
	 * Clear the port interrupt status bits (RWC).
	 */
	cpu_write32(port->base + 0x10, cpu_read32(port->base + 0x10));

	/*
	 * Clear the interrupt status bits (RWC).
	 */
	cpu_write32(ahci->hba_is, cpu_read32(ahci->hba_is));

	/*
	 * Modify the first command header.
	 */
	{
		const uint32_t cfl = 5;
		const uint32_t w = (write_mode ? 1 : 0);

		val = ch[0] & 0xFFFFFFB0u;

		val |= (cfl << 0);
		val |= (w << 6);

		ch[0] = val;
	}

	/*
	 * Modify the "Command FIS" structure.
	 */
	{
		uint8_t *cfis = (void *)((addr_t)(&ct[0]));

		memset(cfis, 0, 64);

		cfis[ 0] = 0x27;
		cfis[ 1] = 0x80;
		cfis[ 2] = (uint8_t)(write_mode ? 0x35 : 0x25);

		cfis[ 4] = (uint8_t)((lba  >>  0) & 0xFF);
		cfis[ 5] = (uint8_t)((lba  >>  8) & 0xFF);
		cfis[ 6] = (uint8_t)((lba  >> 16) & 0xFF);
		cfis[ 7] = 0x40;
		cfis[ 8] = (uint8_t)((lba  >> 24) & 0xFF);
		cfis[ 9] = (uint8_t)((lba  >> 32) & 0xFF);
		cfis[10] = (uint8_t)((lba  >> 40) & 0xFF);

		cfis[12] = (uint8_t)((count >> 0) & 0xFF);
		cfis[13] = (uint8_t)((count >> 8) & 0xFF);
	}

	/*
	 * Modify the first physical region descriptor.
	 */
	{
		uint32_t *prdt = &ct[32];
		const uint32_t dbc = (uint32_t)((count * 512) - 1);

		prdt[0] = (uint32_t)((phys_addr_t)port->buffer_io);
		prdt[1] = 0;

		val = prdt[3] & 0x7FC00000u;

		val |= (dbc << 0);
		val |= (1u << 31);

		prdt[3] = val;
	}

	/*
	 * Check the task file data register.
	 */
	for (i = 0; /* void */; i++) {
		const uint32_t sts_drq = (1u << 3);
		const uint32_t sts_bsy = (1u << 7);

		val = cpu_read32(port->base + 0x20);

		if ((val & sts_drq) == 0 && (val & sts_bsy) == 0)
			break;

		if (i == 250)
			return write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;

		task_sleep(10);
	}

	/*
	 * Write the command issue register.
	 */
	{
		uint32_t shl = (uint32_t)slot;

		cpu_write32(port->base + 0x38, (1u << shl));
	}

	/*
	 * Wait for the command.
	 */
	for (i = 0; /* void */; i++) {
		uint32_t shl = (uint32_t)slot;
		uint32_t ci = cpu_read32(port->base + 0x38);
		uint32_t is = cpu_read32(port->base + 0x10);

		if ((is & (1u << 30)) != 0) {
			printk("[AHCI] SATA I/O Error (TFEE)\n");
			r = write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;
			break;
		}

		if ((ci & (1u << shl)) == 0)
			break;

		if (i == 250) {
			printk("[AHCI] SATA I/O Error (Timeout)\n");
			r = write_mode ? DE_BLOCK_WRITE : DE_BLOCK_READ;
			break;
		}

		event_wait(ahci->event, 10);
	}

	/*
	 * Clear the port interrupt status bits (RWC).
	 */
	cpu_write32(port->base + 0x10, cpu_read32(port->base + 0x10));

	/*
	 * Clear the interrupt status bits (RWC).
	 */
	cpu_write32(ahci->hba_is, cpu_read32(ahci->hba_is));

	return r;
}

static int ahci_init_0(struct ahci *ahci)
{
	uint8_t *base = ahci->base;
	int i, j, ports = 0;
	uint32_t val;

	const uint32_t pxcmd_st  = (1u <<  0);
	const uint32_t pxcmd_fre = (1u <<  4);
	const uint32_t pxcmd_fr  = (1u << 14);
	const uint32_t pxcmd_cr  = (1u << 15);

	printk("[AHCI] Base Address %08X, Size %08X\n",
		(unsigned int)((addr_t)ahci->base), (unsigned int)ahci->size);

	if (!(ahci->event = event_create(0)))
		return DE_MEMORY;

	if (!pci_install_handler(ahci->pci, ahci, ahci_irq_func)) {
		printk("[AHCI] IRQ Install Error\n");
		return DE_UNSUPPORTED;
	}

	if (ahci->size < 0x180)
		return DE_UNSUPPORTED;

	ahci->hba_cap[0] = cpu_read32(base + 0x00);
	ahci->hba_cap[1] = cpu_read32(base + 0x24);

	/*
	 * Handle the BIOS/OS handoff control and status register.
	 */
	for (i = 0; /* void */; i++) {
		if ((ahci->hba_cap[1] & 1) == 0)
			break;

		val = cpu_read32(base + 0x28);

		/*
		 * If OOS == 1 && BOS == 0, ownership is obtained.
		 */
		if ((val & 3) == 2)
			break;

		if (i == 0) {
			printk("[AHCI] Request Ownership\n");
			val |= ((1u << 1) | (1u << 3));
			cpu_write32(base + 0x28, val);
		}

		if (i == 250) {
			const char *e = "[AHCI] Semaphore Error";
			kernel->print("\033[91m[WARNING]\033[m %s\n", e);
			printk("%s\n", e);
			return DE_UNSUPPORTED;
		}

		task_sleep(10);
	}

	ahci->hba_ghc  = (void *)(base + 0x04);
	ahci->hba_is   = (void *)(base + 0x08);

	/*
	 * Set the AHCI enable (AE) bit.
	 */
	{
		uint32_t ae_bit = (1u << 31);

		val = cpu_read32(ahci->hba_ghc);
		cpu_write32(ahci->hba_ghc, val | ae_bit);
	}

	ahci->hba_pi = cpu_read32(base + 0x0C);
	ahci->hba_vs = cpu_read32(base + 0x10);

	/*
	 * Get the number of available/implemented ports.
	 */
	for (i = 0; i < 32; i++) {
		unsigned int shl = (unsigned int)i;

		if ((ahci->hba_pi & (1u << shl)) == 0)
			continue;

		if ((size_t)(0x180 + (i * 0x80)) > ahci->size) {
			printk("[AHCI] Port %d, Register Overflow\n", i);
			return DE_UNSUPPORTED;
		}

		ports += 1;
	}

	printk("[AHCI] Version %d.%d%d, Number of Ports %d, Available %d\n",
		(int)((ahci->hba_vs >> 16) & 0xFFFF),
		(int)((ahci->hba_vs >>  8) & 0xFF),
		(int)((ahci->hba_vs >>  0) & 0xFF),
		(int)(ahci->hba_cap[0] & 0x1F) + 1, (int)ports);

	if (ports == 0 || ports > (int)(ahci->hba_cap[0] & 0x1F) + 1)
		return DE_UNSUPPORTED;

	/*
	 * Check the port registers.
	 */
	for (i = 0; i < 32; i++) {
		unsigned int shl = (unsigned int)i;
		uint8_t *port = base + 0x100;

		if ((ahci->hba_pi & (1u << shl)) == 0)
			continue;

		port += (i * 0x80);

		ahci->ports[i].base = port;
		ahci->ports[i].signature = cpu_read32(port + 0x24);

		/*
		 * The "Command List" must not be running.
		 */
		for (j = 0; /* void */; j++) {
			val = cpu_read32(port + 0x18);

			if ((val & (pxcmd_st | pxcmd_cr)) == 0)
				break;

			if (j == 0) {
				val &= (~pxcmd_st);
				cpu_write32(port + 0x18, val);
			}

			if (j == 250) {
				printk("[AHCI] Port %d, CR Error\n", i);
				return DE_UNSUPPORTED;
			}

			task_sleep(10);
		}

		/*
		 * The "FIS Receive" must not be running.
		 */
		for (j = 0; /* void */; j++) {
			val = cpu_read32(port + 0x18);

			if ((val & (pxcmd_fre | pxcmd_fr)) == 0)
				break;

			if (j == 0) {
				val &= (~pxcmd_fre);
				cpu_write32(port + 0x18, val);
			}

			if (j == 250) {
				printk("[AHCI] Port %d, FR Error\n", i);
				return DE_UNSUPPORTED;
			}

			task_sleep(10);
		}
	}

	/*
	 * Set the global interrupt enable (IE) bit.
	 */
	{
		uint32_t ie_bit = (1u << 1);

		val = cpu_read32(ahci->hba_ghc);
		cpu_write32(ahci->hba_ghc, val | ie_bit);

		cpu_write32(ahci->hba_is, 0xFFFFFFFFu);
	}

	/*
	 * Initialize the Serial ATA ports.
	 */
	for (i = 0; i < 32; i++) {
		struct ahci_port *port = &ahci->ports[i];

		if (port->signature != 0x00000101)
			continue;

		/*
		 * Read the "Serial ATA Status" register.
		 */
		{
			val = cpu_read32(port->base + 0x28);

			/*
			 * Check the "DET" state.
			 */
			if (((val >> 0) & 0x0F) != 3)
				continue;

			/*
			 * Check the "IPM" state.
			 */
			if (((val >> 8) & 0x0F) != 1)
				continue;
		}

		/*
		 * Allocate the buffers.
		 */
		{
			if ((port->buffer_cmd = ahci_alloc(0x1000)) == NULL)
				return DE_MEMORY;

			memset(port->buffer_cmd, 0, 0x1000);

			if ((port->buffer_fis = ahci_alloc(0x1000)) == NULL)
				return DE_MEMORY;

			memset(port->buffer_fis, 0, 0x1000);

			if ((port->buffer_ct = ahci_alloc(0x2000)) == NULL)
				return DE_MEMORY;

			memset(port->buffer_ct, 0, 0x2000);

			if ((port->buffer_io = ahci_alloc(0x10000)) == NULL)
				return DE_MEMORY;

			memset(port->buffer_io, 0, 0x10000);
		}

		/*
		 * Set the command table base addresses (CTBA).
		 */
		for (j = 0; j < 32; j++) {
			uint32_t *ch = port->buffer_cmd;
			uint32_t *ct = port->buffer_ct;

			ch += (j * 8);
			ct += (j * 64);

			ch[0] = (1u << 16);
			ch[1] = 0;
			ch[2] = (uint32_t)((phys_addr_t)ct);
			ch[3] = 0;
		}

		/*
		 * Just to be sure, verify again that nothing is running.
		 */
		val = cpu_read32(port->base + 0x18);

		if ((val & (pxcmd_st | pxcmd_cr)) != 0) {
			printk("[AHCI] Port %d, Unexpected CR Error!\n", i);
			return DE_UNSUPPORTED;
		}

		if ((val & (pxcmd_fre | pxcmd_fr)) != 0) {
			printk("[AHCI] Port %d, Unexpected FR Error!\n", i);
			return DE_UNSUPPORTED;
		}

		/*
		 * Set the command list base address.
		 */
		val = (uint32_t)((phys_addr_t)port->buffer_cmd);
		cpu_write32(port->base + 0x00, val);
		cpu_write32(port->base + 0x04, 0);

		/*
		 * Set the FIS base address.
		 */
		val = (uint32_t)((phys_addr_t)port->buffer_fis);
		cpu_write32(port->base + 0x08, val);
		cpu_write32(port->base + 0x0C, 0);

		/*
		 * Set interrupt enable bits.
		 */
		cpu_write32(port->base + 0x14, 0xFFFFFFFFu);

		/*
		 * Set the "FIS Receive Enable" and "Start" bits.
		 */
		{
			val = cpu_read32(port->base + 0x18);
			val |= pxcmd_fre;

			cpu_write32(port->base + 0x18, val);
			task_sleep(5);

			val = cpu_read32(port->base + 0x18);
			val |= pxcmd_st;
			val |= pxcmd_fre;

			cpu_write32(port->base + 0x18, val);
			task_sleep(5);
		}

		if (ahci_identify(ahci, port))
			continue;

		port->disk_size = ahci_get_disk_size(port->buffer_io);

		if (port->disk_size < 0x100000)
			continue;

		printk("[AHCI] Port %d, Serial ATA Available, %lld MiB\n", i,
			((unsigned long long)port->disk_size / 1024) / 1024);

		port->sata_available = 1;
		port->ahci = ahci;

		mount_drive(port);
	}

	return 0;
}

static int read_write_locked(struct ahci *ahci, struct ahci_port *port,
	uint64_t offset, size_t *size, addr_t buffer, int write_mode)
{
	size_t requested_size = *size;
	size_t transfer_size = 0;

	uint64_t sector_count = (port->disk_size / 512);
	uint64_t lba = (offset / 512);
	int r = 0;

	*size = 0;

	if ((offset & 0x1FF) != 0 || (requested_size & 0x1FF) != 0)
		return DE_ALIGNMENT;

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
			dst = (void *)port->buffer_io;
			src = (void *)((addr_t)(buffer + transfer_size));
			memcpy(dst, src, (size_t)unit_size);
		}

		pg_enter_kernel();

		r = ahci_read_write(ahci, port,
			lba, (unit_size / 512), write_mode);

		if (r) {
			unit_size = 512;
			r = ahci_read_write(ahci, port, lba, 1, write_mode);
		}

		pg_leave_kernel();

		if (r)
			break;

		if (!write_mode) {
			dst = (void *)((addr_t)(buffer + transfer_size));
			src = (void *)port->buffer_io;
			memcpy(dst, src, (size_t)unit_size);
		}

		lba += (unit_size / 512);
		transfer_size += unit_size;
	}

	return *size = transfer_size, r;
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
	struct ahci_port *port = node->internal_data;
	struct ahci *ahci = port->ahci;
	int r;

	spin_lock_yield(&port->lock);

	r = read_write_locked(ahci, port, offset, size, (addr_t)buffer, 0);

	spin_unlock(&port->lock);

	return r;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	struct ahci_port *port = node->internal_data;
	struct ahci *ahci = port->ahci;
	int r;

	spin_lock_yield(&port->lock);

	r = read_write_locked(ahci, port, offset, size, (addr_t)buffer, 1);

	spin_unlock(&port->lock);

	return r;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct ahci_port *port = node->internal_data;

	memset(stat, 0, sizeof(*stat));

	spin_lock_yield(&port->lock);

	stat->size = port->disk_size;
	stat->block_size = 512;

	spin_unlock(&port->lock);

	return 0;
}

static int mount_drive(struct ahci_port *port)
{
	static int drive = 'a';

	char name[12];
	struct vfs_node *node;
	int r;

	if (drive < 'a' || drive > 'z')
		return DE_OVERFLOW;

	if (snprintf(&name[0], sizeof(name), "/dev/sd%c", drive) != 8)
		return DE_UNEXPECTED;

	drive += 1;

	if ((r = vfs_open(&name[0], &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	if ((node = malloc(sizeof(*node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(node, 0);

	node->count = 1;
	node->type = vfs_type_block;

	node->internal_data = port;
	node->n_release = n_release;

	node->n_read  = n_read;
	node->n_write = n_write;
	node->n_stat  = n_stat;

	r = vfs_mount(name, node);
	node->n_release(&node);

	return r;
}

static phys_addr_t get_base(struct pci_id *pci)
{
	const int offset = 0x24;
	phys_addr_t base = (phys_addr_t)pci_read(pci, offset);
	int io_space = (int)(base & 1);

	if (io_space)
		return (phys_addr_t)(1);

	base &= 0xFFFFFFF0u;

	return base;
}

static size_t get_size(struct pci_id *pci)
{
	const int offset = 0x24;
	uint32_t val, saved;

	saved = pci_read(pci, offset);

	pci_write(pci, offset, 0xFFFFFFFFu);
	val = pci_read(pci, offset);

	pci_write(pci, offset, saved);

	return (size_t)((~(val & 0xFFFFFFF0u)) + 1u);
}

static int ahci_init(struct pci_id *pci)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFF;
	phys_addr_t addr = get_base(pci);
	int r = DE_UNSUPPORTED;

	if ((cmd & 2) != 0 && addr > 1 && addr < SIZE_MAX) {
		size_t size = get_size(pci);
		void *base = pg_map_kernel(addr, size, pg_uncached);

		if (base != NULL) {
			struct ahci *ahci;

			if ((ahci = malloc(sizeof(*ahci))) == NULL)
				return DE_MEMORY;

			memset(ahci, 0, sizeof(*ahci));
			ahci->pci = pci;
			ahci->base = base;
			ahci->size = size;

			pci_write(pci, 0x04, cmd | 4);

			r = ahci_init_0(ahci);
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(ahci_init, -1, -1, 0x01, 0x06, 0x01);
