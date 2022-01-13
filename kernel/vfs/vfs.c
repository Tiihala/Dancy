/*
 * Copyright (c) 2021, 2022 Antti Tiihala
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
 * vfs/vfs.c
 *      Virtual File System
 */

#include <dancy.h>

int vfs_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	return 0;
}

void vfs_init_node(struct vfs_node *node)
{
	size_t size = sizeof(struct vfs_node);

	memset(node, 0, size);

	node->n_release = vfs_default_n_release;
	node->n_create  = vfs_default_n_create;
	node->n_open    = vfs_default_n_open;

	node->n_read    = vfs_default_n_read;
	node->n_write   = vfs_default_n_write;

	node->n_flush   = vfs_default_n_flush;
	node->n_readdir = vfs_default_n_readdir;

	node->n_link    = vfs_default_n_link;
	node->n_unlink  = vfs_default_n_unlink;
}

int vfs_open_node(const char *name, struct vfs_node **node)
{
	char **path = vfs_build_path(name);
	struct vfs_node *ret_node = NULL;

	if (!node)
		return DE_ARGUMENT;

	*node = ret_node;

	if (path[0][0] == 'E')
		return DE_ARGUMENT;

	return DE_NAME;
}
