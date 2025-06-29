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

	int eeprom_available;
	int mac_available;

	uint8_t mac[6];
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

static int e1000_init(struct e1000 *e1000)
{
	uint32_t val;
	int i;

	printk("[NETWORK] Ethernet Controller Found (%04X %04X)\n",
		e1000->pci->vendor_id, e1000->pci->device_id);

	if (e1000->base == NULL || e1000->size < 0x1000)
		return DE_UNSUPPORTED;

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

	val = pci_read(e1000->pci, 0x04) & 0xFFFF;
	pci_write(e1000->pci, 0x04, (val | (1u << 1) | (1u << 2)));

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

static int network_e1000_init(struct pci_id *pci)
{
	struct e1000 e1000;
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
		memset(&e1000, 0, sizeof(e1000));
		e1000.pci = pci;
		e1000.base = pg_map_kernel(addr, size, pg_uncached);
		e1000.size = size;

		r = e1000_init(&e1000);
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(network_e1000_init, 0x8086, -1, 0x02, -1, -1);
