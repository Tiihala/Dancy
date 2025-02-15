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
 * usb/usbfs.c
 *      Devices /dev/dancy-usb-*
 */

#include <dancy.h>

static struct vfs_node *alloc_node(size_t data_size)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += data_size;

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;

		vfs_init_node(node, size);
		node->internal_data = (void *)a;
	}

	return node;
}

int usbfs_create(struct dancy_usb_controller *hci)
{
	struct pci_id *pci = hci->pci;
	struct vfs_node *node;
	char buffer[32];
	int r;

	r = snprintf(&buffer[0], sizeof(buffer),
		"/dev/dancy-usb-%04X-%02X-%02X-%X/",
		pci->group, pci->bus, pci->device, pci->func);

	if (r != 28)
		return DE_UNEXPECTED;

	{
		int mode = vfs_mode_create | vfs_mode_exclusive;

		if ((r = vfs_open(&buffer[0], &node, 0, mode)) != 0)
			return r;
	}

	node->n_release(&node);

	if ((node = alloc_node(sizeof(*hci))) == NULL)
		return DE_MEMORY;

	node->count = 1;
	node->type = vfs_type_directory;

	memcpy(node->internal_data, hci, sizeof(*hci));

	r = vfs_mount(&buffer[0], node);
	node->n_release(&node);

	return r;
}
