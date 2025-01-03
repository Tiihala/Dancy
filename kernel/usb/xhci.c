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
 * usb/xhci.c
 *      Extensible Host Controller Interface (xHCI)
 */

#include <dancy.h>

struct xhci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint32_t cap_length;
	uint32_t hci_version;
	uint32_t hcs_params[3];
	uint32_t hcc_params[2];

	uint8_t *base_cap;
	uint8_t *xecp;
};

static int xhci_count;
static struct xhci *xhci_array[32];

static int xhci_add(struct xhci *xhci)
{
	static int lock;
	int r = 0;

	spin_lock(&lock);

	if (xhci_count < (int)(sizeof(xhci_array) / sizeof(xhci_array[0])))
		xhci_array[xhci_count++] = xhci;
	else
		r = DE_OVERFLOW;

	spin_unlock(&lock);

	return r;
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

static int xhci_init(struct xhci *xhci)
{
	uint8_t *base = xhci->base;
	uint32_t val;

	val = cpu_read32(base + 0x00);
	xhci->cap_length = (val & 0xFF);
	xhci->hci_version = ((val >> 16) & 0xFFFF);

	xhci->hcs_params[0] = cpu_read32(base + 0x04);
	xhci->hcs_params[1] = cpu_read32(base + 0x08);
	xhci->hcs_params[2] = cpu_read32(base + 0x0C);

	xhci->hcc_params[0] = cpu_read32(base + 0x10);
	xhci->hcc_params[1] = cpu_read32(base + 0x1C);

	xhci->base_cap = (base + xhci->cap_length);
	xhci->xecp = base + (((xhci->hcc_params[0] >> 16) & 0xFFFF) << 2);

	if (xhci->xecp == base)
		xhci->xecp = NULL;

	return 0;
}

static int usb_xhci_init(struct pci_id *pci)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFF;
	phys_addr_t addr = get_base(pci, 0x10);
	int r = DE_UNSUPPORTED;

	if ((cmd & 2) != 0 && addr > 1 && addr < SIZE_MAX) {
		size_t size = get_size(pci, 0x10);
		void *base = pg_map_kernel(addr, size, pg_uncached);

		if (base != NULL) {
			struct xhci *xhci;

			if ((xhci = malloc(sizeof(*xhci))) == NULL)
				return DE_MEMORY;

			memset(xhci, 0, sizeof(*xhci));
			xhci->pci = pci;
			xhci->base = base;
			xhci->size = size;

			if ((r = xhci_init(xhci)) == 0)
				r = xhci_add(xhci);
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(usb_xhci_init, -1, -1, 0x0C, 0x03, 0x30);
