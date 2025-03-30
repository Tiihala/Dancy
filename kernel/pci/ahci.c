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

	uint32_t hba_cap[2];
	uint32_t hba_pi;
	uint32_t hba_vs;

	void *hba_ghc;
	void *hba_is;
};

static int ahci_init_0(struct ahci *ahci)
{
	uint8_t *base = ahci->base;
	int i, ports = 0;
	uint32_t val;

	printk("[AHCI] Base Address %08X, Size %08X\n",
		(unsigned int)((addr_t)ahci->base), (unsigned int)ahci->size);

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

		printk("[AHCI] Port %d, Command and Status %08X, "
			"Signature %08X\n", i,
			(unsigned int)cpu_read32(port + 0x18),
			(unsigned int)cpu_read32(port + 0x24));
	}

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
