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
 * network/e1000.c
 *      Ethernet controller (Intel)
 */

#include <dancy.h>

struct e1000 {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint8_t *rx;
	uint8_t *tx;

	int rx_head;
	int tx_tail;

	int eeprom_available;
	int mac_available;

	uint8_t mac[6];
	uint32_t irq_count;

	event_t rx_event[2];
	size_t rx_frame_size;
	void *rx_frame;

	int lock;
};

static void *e1000_alloc(size_t size)
{
	if (size <= 0x1000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 0);
		return pg_map_kernel(addr, 0x1000, pg_uncached);
	}

	if (size <= 0x2000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 1);
		return pg_map_kernel(addr, 0x2000, pg_uncached);
	}

	return NULL;
}

static uint32_t e1000_read32(struct e1000 *e1000, int offset)
{
	return cpu_read32(e1000->base + offset);
}

static void e1000_write32(struct e1000 *e1000, int offset, uint32_t value)
{
	cpu_write32(e1000->base + offset, value);
}

static uint32_t e1000_read_eeprom(struct e1000 *e1000, int offset)
{
	uint32_t val;

	if (e1000->eeprom_available) {
		int counter = 0;

		val = (uint32_t)((offset << 8) | 1);
		e1000_write32(e1000, 0x14, val);

		do {
			val = e1000_read32(e1000, 0x14);

			if (counter > 0x1000)
				task_yield();
			if (counter > 0x2000)
				return 0;

			counter += 1;

		} while ((val & (1u << 4)) == 0);

		return ((val >> 16) & 0xFFFF);
	}

	return 0;
}

static void e1000_rx_frame(struct e1000 *e1000, uint8_t *data, size_t size)
{
	int r = DE_RETRY;

	while (r == DE_RETRY) {
		spin_lock_yield(&e1000->lock);

		if (e1000->rx_frame == NULL) {
			e1000->rx_frame_size = size;
			e1000->rx_frame = data, r = 0;
		}

		spin_unlock(&e1000->lock);

		event_signal(e1000->rx_event[1]);
		event_wait(e1000->rx_event[0], 0xFFFF);
	}
}

static void e1000_irq_func(int irq, void *arg)
{
	struct e1000 *e1000 = arg;
	const int icr_reg = 0xC0;
	uint32_t val;

	pg_enter_kernel();

	if ((val = e1000_read32(e1000, icr_reg)) != 0)
		e1000_write32(e1000, icr_reg, val);

	cpu_add32(&e1000->irq_count, 1);

	pg_leave_kernel();
	(void)irq;
}

static int e1000_task(void *arg)
{
	struct e1000 *e1000 = arg;

	task_set_cmdline(task_current(), NULL, "[e1000]");

	while (e1000) {
		const int rdh0_reg = 0x2810;
		const int rdt0_reg = 0x2818;

		int i = (int)(e1000_read32(e1000, rdh0_reg) & 0xFFFF);

		if (i == e1000->rx_head) {
			task_sleep(20);
			continue;
		}

		i = e1000->rx_head++;

		if (e1000->rx_head > 511)
			e1000->rx_head = 0;

		{
			phys_addr_t *rx = (void *)(&e1000->rx[i * 16]);

			int size_0 = (int)e1000->rx[(i * 16) + 8];
			int size_1 = (int)e1000->rx[(i * 16) + 9];
			int status = (int)e1000->rx[(i * 16) + 12];
			int errors = (int)e1000->rx[(i * 16) + 13];

			size_t size = (size_t)((size_0 << 0) | (size_1 << 8));

			if ((status & 0x01) == 0 || (errors & 0x87) != 0)
				size = 0;

			if (size >= 14 && size <= 1984)
				e1000_rx_frame(e1000, (void *)(rx[0]), size);

			memset((void *)(rx[0]), 0, 2048);
			memset(&e1000->rx[(i * 16) + 8], 0, 8);
		}

		e1000_write32(e1000, rdt0_reg, (uint32_t)i);
	}

	return 0;
}

static int e1000_init(struct e1000 *e1000)
{
	uint32_t val;
	int i;

	printk("[NETWORK] Ethernet Controller Found (%04X %04X)\n",
		e1000->pci->vendor_id, e1000->pci->device_id);

	if (e1000->base == NULL || e1000->size < 0x1000)
		return DE_UNSUPPORTED;

	val = pci_read(e1000->pci, 0x04) & 0xFFFF;
	pci_write(e1000->pci, 0x04, (val | (1u << 1) | (1u << 2)));

	/*
	 * Install the IRQ handler.
	 */
	if (pci_install_handler(e1000->pci, e1000, e1000_irq_func) == NULL) {
		kernel->print("\033[91m[ERROR]\033[m E1000 IRQ Handling\n");
		return DE_UNSUPPORTED;
	}

	/*
	 * Reset the device.
	 */
	{
		const int ctrl_reg   = 0x00;
		const int status_reg = 0x04;

		const int icr_reg  = 0x00C0;
		const int imc_reg  = 0x00D8;

		const int rctl_reg = 0x0100;
		const int tctl_reg = 0x0400;

		const uint32_t rst_bit = (1u << 26);

		e1000_write32(e1000, rctl_reg, 0);
		e1000_write32(e1000, tctl_reg, 0);
		(void)e1000_read32(e1000, status_reg);

		e1000_write32(e1000, imc_reg, 0xFFFFFFFFu);
		e1000_write32(e1000, icr_reg, 0xFFFFFFFFu);
		(void)e1000_read32(e1000, status_reg);

		val = e1000_read32(e1000, ctrl_reg);
		e1000_write32(e1000, ctrl_reg, val | rst_bit);
		(void)e1000_read32(e1000, status_reg);

		e1000_write32(e1000, imc_reg, 0xFFFFFFFFu);
		e1000_write32(e1000, icr_reg, 0xFFFFFFFFu);
		(void)e1000_read32(e1000, status_reg);
	}

	/*
	 * Allocate the buffers.
	 */
	{
		if ((e1000->rx = e1000_alloc(0x2000)) == NULL)
			return DE_MEMORY;

		if ((e1000->tx = e1000_alloc(0x2000)) == NULL)
			return DE_MEMORY;

		memset(e1000->rx, 0, 0x2000);
		memset(e1000->tx, 0, 0x2000);

		for (i = 0; i < 512; i++) {
			phys_addr_t *rx = (void *)(&e1000->rx[i * 16]);
			phys_addr_t *tx = (void *)(&e1000->tx[i * 16]);

			if ((*rx = (phys_addr_t)e1000_alloc(0x1000)) == 0)
				return DE_MEMORY;

			if ((*tx = (phys_addr_t)e1000_alloc(0x1000)) == 0)
				return DE_MEMORY;

			memset((void *)(*rx), 0, 0x1000);
			memset((void *)(*tx), 0, 0x1000);
		}
	}

	printk("[NETWORK] E1000 Base %08zX (Size %08zX)\n",
		(size_t)((phys_addr_t)e1000->base), e1000->size);

	/*
	 * Detect the EEPROM.
	 */
	{
		e1000_write32(e1000, 0x14, 0x01);

		for (i = 0; i < 0x8000; i++) {
			val = e1000_read32(e1000, 0x14);

			if ((val & 0x10) != 0) {
				e1000->eeprom_available = i | 1;
				break;
			}
		}

		printk("[NETWORK] E1000 EEPROM %s (%d)\n",
			e1000->eeprom_available ? "Available" : "Not Found",
			e1000->eeprom_available);
	}

	/*
	 * Read the MAC address.
	 */
	{
		uint8_t mac[6] = { 0 };

		if (e1000->eeprom_available) {
			val = e1000_read_eeprom(e1000, 0);
			mac[0] = (uint8_t)((val >> 0) & 0xFF);
			mac[1] = (uint8_t)((val >> 8) & 0xFF);

			val = e1000_read_eeprom(e1000, 1);
			mac[2] = (uint8_t)((val >> 0) & 0xFF);
			mac[3] = (uint8_t)((val >> 8) & 0xFF);

			val = e1000_read_eeprom(e1000, 2);
			mac[4] = (uint8_t)((val >> 0) & 0xFF);
			mac[5] = (uint8_t)((val >> 8) & 0xFF);

			for (i = 0; i < 6; i++) {
				if ((e1000->mac[i] = mac[i]) != 0)
					e1000->mac_available = 1;
			}
		}

		if (!e1000->mac_available) {
			val = e1000_read32(e1000, 0x5400);
			mac[0] = (uint8_t)((val >>  0) & 0xFF);
			mac[1] = (uint8_t)((val >>  8) & 0xFF);
			mac[2] = (uint8_t)((val >> 16) & 0xFF);
			mac[3] = (uint8_t)((val >> 24) & 0xFF);

			val = e1000_read32(e1000, 0x5404);
			mac[4] = (uint8_t)((val >>  0) & 0xFF);
			mac[5] = (uint8_t)((val >>  8) & 0xFF);

			for (i = 0; i < 6; i++) {
				if ((e1000->mac[i] = mac[i]) != 0)
					e1000->mac_available = 1;
			}
		}

		if (!e1000->mac_available) {
			printk("[NETWORK] E1000 MAC Not Found\n");
			return DE_UNSUPPORTED;
		}

		printk("[NETWORK] E1000 MAC %02X-%02X-%02X-%02X-%02X-%02X\n",
			(unsigned int)mac[0], (unsigned int)mac[1],
			(unsigned int)mac[2], (unsigned int)mac[3],
			(unsigned int)mac[4], (unsigned int)mac[5]);
	}

	/*
	 * Set the FCAL, FCAH and FCT registers using recommended values.
	 */
	{
		const int fcal_reg = 0x28;
		const int fcah_reg = 0x2C;
		const int fct_reg  = 0x30;

		e1000_write32(e1000, fcal_reg, 0x00C28001);
		e1000_write32(e1000, fcah_reg, 0x00000100);
		e1000_write32(e1000, fct_reg,  0x00008808);
	}

	/*
	 * Set the FCTTV register.
	 */
	{
		const int fcttv_reg = 0x0170;

		e1000_write32(e1000, fcttv_reg, 0x0000FFFF);
	}

	/*
	 * Set the CTRL register.
	 */
	{
		const int ctrl_reg   = 0x00;
		const int status_reg = 0x04;

		const uint32_t slu_bit   = (1u << 6);
		const uint32_t speed_val = 0;

		val = e1000_read32(e1000, ctrl_reg);

		val |= slu_bit;
		val |= (speed_val << 8);
		val &= (0x7FFFFFF7u);

		e1000_write32(e1000, ctrl_reg, val);
		(void)e1000_read32(e1000, status_reg);
	}

	/*
	 * Clear the multicast table array.
	 */
	{
		const int mta_beg = 0x5200;
		const int mta_end = 0x53FC;

		for (i = mta_beg; i <= mta_end; i += 4)
			e1000_write32(e1000, i, 0);
	}

	/*
	 * Read the statistical registers to clear them.
	 */
	{
		const int stat_beg = 0x4000;
		const int stat_end = 0x4124;

		for (i = stat_beg; i <= stat_end; i += 4)
			(void)e1000_read32(e1000, i);
	}

	/*
	 * Set the receive registers.
	 */
	{
		const int rctl_reg   = 0x0100;
		const int rdbal0_reg = 0x2800;
		const int rdbah0_reg = 0x2804;
		const int rdlen0_reg = 0x2808;

		const int rdh0_reg = 0x2810;
		const int rdt0_reg = 0x2818;

		val = (uint32_t)((addr_t)((void *)e1000->rx));

		e1000_write32(e1000, rdbal0_reg, val);
		e1000_write32(e1000, rdbah0_reg, 0);
		e1000_write32(e1000, rdlen0_reg, 0x2000);

		e1000_write32(e1000, rdh0_reg, 0);
		e1000_write32(e1000, rdt0_reg, 511);

		val = 0;

		{
			const uint32_t en_bit  = (1u << 1);
			const uint32_t sbp_bit = (1u << 2);
			const uint32_t mpe_bit = (1u << 4);
			const uint32_t bam_bit = (1u << 15);

			const uint32_t bsex_bit  = (1u << 25);
			const uint32_t secrc_bit = (1u << 26);

			const uint32_t bsize = 3;

			val |= en_bit;
			val |= sbp_bit;
			val |= mpe_bit;
			val |= bam_bit;
			val |= (bsize << 16);
			val |= bsex_bit;
			val |= secrc_bit;
		}

		e1000_write32(e1000, rctl_reg, val);
	}

	/*
	 * Set the transmit registers.
	 */
	{
		const int tctl_reg  = 0x0400;
		const int tdbal_reg = 0x3800;
		const int tdbah_reg = 0x3804;
		const int tdlen_reg = 0x3808;

		const int tdh_reg = 0x3810;
		const int tdt_reg = 0x3818;

		val = (uint32_t)((addr_t)((void *)e1000->tx));

		e1000_write32(e1000, tdbal_reg, val);
		e1000_write32(e1000, tdbah_reg, 0);
		e1000_write32(e1000, tdlen_reg, 0x2000);

		e1000_write32(e1000, tdh_reg, 0);
		e1000_write32(e1000, tdt_reg, 0);

		val = e1000_read32(e1000, tctl_reg);

		{
			const uint32_t en_bit   = (1u << 1);
			const uint32_t psp_bit  = (1u << 3);
			const uint32_t rtlc_bit = (1u << 24);

			const uint32_t ct_mask = (0xFFu << 4);
			const uint32_t ct_val  = (0x0Fu << 4);

			val |= en_bit;
			val |= psp_bit;
			val |= rtlc_bit;

			val &= (~ct_mask);
			val |= (ct_val);
		}

		e1000_write32(e1000, tctl_reg, val);
	}

	/*
	 * Create a kernel task for this device.
	 */
	if (!task_create(e1000_task, e1000, task_detached))
		return DE_MEMORY;

	/*
	 * Set the interrupt mask set/read register.
	 */
	{
		const int status_reg = 0x04;
		const int ims_reg = 0xD0;

		val = 0;

		{
			const uint32_t txdw_bit = (1u << 0);
			const uint32_t txqe_bit = (1u << 1);
			const uint32_t lsc_bit  = (1u << 2);
			const uint32_t rxt0_bit = (1u << 7);

			val |= txdw_bit;
			val |= txqe_bit;
			val |= lsc_bit;
			val |= rxt0_bit;
		}

		e1000_write32(e1000, ims_reg, val);
		(void)e1000_read32(e1000, status_reg);
	}

	printk("[NETWORK] E1000 Initialization Completed\n");

	return 0;
}

static phys_addr_t get_base(struct pci_id *pci, int offset)
{
	phys_addr_t base = (phys_addr_t)pci_read(pci, offset), base_high = 0;
	int io_space = (int)(base & 1);
	int type = (int)((base >> 1) & 3);

	if (io_space)
		return (phys_addr_t)(1);

	base &= 0xFFFFFFF0u;

	if (type == 2) {
		base_high = (phys_addr_t)pci_read(pci, offset + 4);
		base |= ((base_high << 16) << 16);
#ifdef DANCY_32
		if (base_high)
			base = 0;
#endif
	}

	return base;
}

static size_t get_size(struct pci_id *pci, int offset)
{
	uint32_t val, saved;

	saved = pci_read(pci, offset);

	pci_write(pci, offset, 0xFFFFFFFFu);
	val = pci_read(pci, offset);

	pci_write(pci, offset, saved);

	return (size_t)((~(val & 0xFFFFFFF0u)) + 1u);
}

static struct e1000 *get_e1000(struct vfs_node *node)
{
	struct dancy_net_controller *dnc = node->internal_data;
	return dnc->controller;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct e1000 *e1000 = get_e1000(node);
	size_t buffer_size = *size;
	int r = DE_RETRY;

	(void)offset;
	*size = 0;

	if (buffer_size == 0 || (buffer_size % 2048) != 0)
		return DE_ALIGNMENT;

	if (((size_t)((addr_t)buffer) % 2048) != 0)
		return DE_ALIGNMENT;

	spin_lock_yield(&e1000->lock);

	if (e1000->rx_frame != NULL) {
		memcpy(buffer, e1000->rx_frame, e1000->rx_frame_size);

		*size = e1000->rx_frame_size;
		e1000->rx_frame_size = 0;
		e1000->rx_frame = NULL;

		event_signal(e1000->rx_event[0]);
		r = 0;
	}

	spin_unlock(&e1000->lock);

	return r;
}

static int n_ioctl(struct vfs_node *node,
	int request, long long arg)
{
	struct e1000 *e1000 = get_e1000(node);

	spin_lock_yield(&e1000->lock);

	(void)request;
	(void)arg;

	spin_unlock(&e1000->lock);

	return DE_UNSUPPORTED;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct e1000 *e1000 = get_e1000(node);

	spin_lock_yield(&e1000->lock);

	memset(stat, 0, sizeof(*stat));

	spin_unlock(&e1000->lock);

	return 0;
}

static int network_e1000_init(struct pci_id *pci)
{
	struct e1000 *e1000;
	struct dancy_net_controller *dnc;

	int r = DE_UNSUPPORTED;

	phys_addr_t addr = 0;
	size_t size = 0;

	switch (pci->device_id) {
		case 0x100E: r = 0; break;
		default: break;
	}

	if (r == 0) {
		addr = get_base(pci, 0x10);
		size = get_size(pci, 0x10);

		if (addr <= 1)
			r = DE_UNSUPPORTED;
	}

	if (r == 0) {
		if ((e1000 = malloc(sizeof(*e1000))) == NULL)
			return DE_MEMORY;

		memset(e1000, 0, sizeof(*e1000));

		e1000->pci = pci;
		e1000->base = pg_map_kernel(addr, size, pg_uncached);
		e1000->size = size;

		if (!(e1000->rx_event[0] = event_create(0)))
			return DE_MEMORY;

		if (!(e1000->rx_event[1] = event_create(0)))
			return DE_MEMORY;

		r = e1000_init(e1000);
	}

	if (r == 0) {
		if ((dnc = malloc(sizeof(*dnc))) == NULL)
			return DE_MEMORY;

		memset(dnc, 0, sizeof(*dnc));

		dnc->node = NULL;
		dnc->controller = e1000;

		if ((r = net_register_controller(dnc)) == 0) {
			void *internal_event = &e1000->rx_event[1];
			dnc->node->internal_event = internal_event;

			dnc->node->n_read = n_read;
			dnc->node->n_ioctl = n_ioctl;
			dnc->node->n_stat = n_stat;
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(network_e1000_init, 0x8086, -1, 0x02, -1, -1);
