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
};

static int e1000_init(struct e1000 *e1000)
{
	printk("[NETWORK] Ethernet Controller Found (%04X %04X)\n",
		e1000->pci->vendor_id, e1000->pci->device_id);

	return 0;
}

static int network_e1000_init(struct pci_id *pci)
{
	struct e1000 e1000;
	int r = DE_UNSUPPORTED;

	switch (pci->device_id) {
		case 0x100E: r = 0; break;
		default: break;
	}

	if (r == 0) {
		memset(&e1000, 0, sizeof(e1000));
		e1000.pci = pci;

		r = e1000_init(&e1000);
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(network_e1000_init, 0x8086, -1, 0x02, -1, -1);
