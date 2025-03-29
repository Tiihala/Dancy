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

struct ahci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;
};

static int ahci_init_0(struct ahci *ahci)
{
	printk("[AHCI] Base Address %08X, Size %08X\n",
		(unsigned int)((addr_t)ahci->base), (unsigned int)ahci->size);

	return 0;
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
