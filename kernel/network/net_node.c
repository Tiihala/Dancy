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
 * network/net_node.c
 *      Devices /dev/dancy-network-*
 */

#include <dancy.h>

static int controller_monotonic;
static int controller_monotonic_lock;

int net_register_controller(struct dancy_net_controller *dnc)
{
	struct vfs_node *node;
	char buffer[24];
	int r = 0;

	spin_lock_yield(&controller_monotonic_lock);

	if (controller_monotonic >= 0 && controller_monotonic < 99)
		r = (++controller_monotonic);

	spin_unlock(&controller_monotonic_lock);

	if (r == 0)
		return DE_OVERFLOW;

	r = snprintf(&buffer[0], sizeof(buffer),
		"/dev/dancy-network-%02d", r);

	if (r != 21)
		return DE_UNEXPECTED;

	{
		int mode = vfs_mode_create | vfs_mode_exclusive;

		if ((r = vfs_open(&buffer[0], &node, 0, mode)) != 0)
			return r;
	}

	node->n_release(&node);

	if ((node = malloc(sizeof(*node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(node, 0);

	node->type = vfs_type_block;
	node->count = 1;

	node->internal_data = dnc;
	dnc->node = node;

	r = vfs_mount(&buffer[0], node);
	node->n_release(&node);

	return r;
}
