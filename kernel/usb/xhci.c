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

struct xhci_port {
	uint8_t rev_major;
	uint8_t rev_minor;
	char name[6];

	uint32_t *portsc;
};

struct xhci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint32_t cap_length;
	uint32_t hci_version;
	uint32_t hcs_params[3];
	uint32_t hcc_params[2];

	uint32_t db_off;
	uint32_t rts_off;

	uint8_t *base_cap;
	uint8_t *xecp;

	int max_slots;
	int max_intrs;
	int max_ports;

	struct xhci_port *ports;

	uint32_t *usb_cmd;
	uint32_t *usb_sts;
};

static int xhci_count;
static struct xhci *xhci_array[32];

static int usb_task(void *arg)
{
	struct xhci *xhci = arg;

	task_set_cmdline(task_current(), NULL, "[xhci]");

	task_sleep(10000);

	return (cpu_read32(xhci->usb_sts) & 1) ? 0 : 1;
}

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

static void check_supported_protocols(struct xhci *xhci)
{
	const uint32_t supported_protocol = 2;
	uint8_t *p = xhci->xecp;
	int i, j;

	while (xhci->xecp) {
		uint32_t val = cpu_read32(p);
		uint32_t add = ((val >> 8) & 0xFF) << 2;
		uint32_t name;

		if ((val & 0xFF) == supported_protocol) {
			uint8_t rev_major, rev_minor;
			uint8_t port_count, port_offset;

			rev_major = (uint8_t)((val >> 24) & 0xFF);
			rev_minor = (uint8_t)((val >> 16) & 0xFF);

			name = cpu_read32(p + 0x04);
			val = cpu_read32(p + 0x08);

			port_count  = (uint8_t)((val >> 8) & 0xFF);
			port_offset = (uint8_t)((val >> 0) & 0xFF);

			for (i = 0; i < (int)port_count; i++) {
				struct xhci_port *port;

				j = ((int)port_offset + i) - 1;

				if (j < 0 || j >= xhci->max_ports)
					continue;

				port = &xhci->ports[j];
				port->rev_major = rev_major;
				port->rev_minor = rev_minor;

				port->name[0] = (char)((name >> 0x00) & 0xFF);
				port->name[1] = (char)((name >> 0x08) & 0xFF);
				port->name[2] = (char)((name >> 0x10) & 0xFF);
				port->name[3] = (char)((name >> 0x18) & 0xFF);
				port->name[4] = '\0';

				for (j = 3; j >= 0; j--) {
					if (port->name[j] == ' ')
						port->name[j] = '\0';
				}
			}
		}

		if (add == 0)
			break;
		p += add;
	}
}

static int xhci_init(struct xhci *xhci)
{
	uint8_t *base = xhci->base;
	uint32_t val;
	int i;

	val = cpu_read32(base + 0x00);
	xhci->cap_length = (val & 0xFF);
	xhci->hci_version = ((val >> 16) & 0xFFFF);

	xhci->hcs_params[0] = cpu_read32(base + 0x04);
	xhci->hcs_params[1] = cpu_read32(base + 0x08);
	xhci->hcs_params[2] = cpu_read32(base + 0x0C);

	xhci->hcc_params[0] = cpu_read32(base + 0x10);
	xhci->hcc_params[1] = cpu_read32(base + 0x1C);

	xhci->db_off  = cpu_read32(base + 0x14) & 0xFFFFFFFC;
	xhci->rts_off = cpu_read32(base + 0x18) & 0xFFFFFFE0;

	xhci->base_cap = (base + xhci->cap_length);
	xhci->xecp = base + (((xhci->hcc_params[0] >> 16) & 0xFFFF) << 2);

	if (xhci->xecp == base)
		return DE_UNSUPPORTED;

	xhci->max_slots = (int)((xhci->hcs_params[0] >>  0) & 0x00FF);
	xhci->max_intrs = (int)((xhci->hcs_params[0] >>  8) & 0x07FF);
	xhci->max_ports = (int)((xhci->hcs_params[0] >> 24) & 0x00FF);

	if (xhci->max_ports == 0)
		return DE_UNSUPPORTED;

	xhci->ports = calloc((size_t)xhci->max_ports, sizeof(*xhci->ports));
	if (xhci->ports == NULL)
		return DE_MEMORY;

	check_supported_protocols(xhci);

	for (i = 0; i < xhci->max_ports; i++) {
		addr_t a = (addr_t)(xhci->base_cap + 0x400 + (i * 16));
		xhci->ports[i].portsc = (uint32_t *)a;
	}

	xhci->usb_cmd = (uint32_t *)((void *)(xhci->base_cap + 0x0000));
	xhci->usb_sts = (uint32_t *)((void *)(xhci->base_cap + 0x0004));

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

			if ((r = xhci_add(xhci)) == 0)
				r = xhci_init(xhci);

			if (!r && !task_create(usb_task, xhci, task_detached))
				r = DE_MEMORY;
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(usb_xhci_init, -1, -1, 0x0C, 0x03, 0x30);
