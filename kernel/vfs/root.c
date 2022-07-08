/*
 * Copyright (c) 2022 Antti Tiihala
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
 * vfs/root.c
 *      Root file system
 */

#include <dancy.h>

static struct vfs_node root_ramfs_node;

static size_t root_ramfs_size;
static unsigned char *root_ramfs;

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	size_t read_size;

	(void)node;

	if (offset >= (uint64_t)root_ramfs_size)
		return *size = 0, 0;

	read_size = root_ramfs_size - (size_t)offset;

	if (read_size > *size)
		read_size = *size;

	memcpy(buffer, &root_ramfs[(size_t)offset], read_size);

	return *size = read_size, 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	size_t write_size;

	(void)node;

	if (offset >= (uint64_t)root_ramfs_size)
		return *size = 0, 0;

	write_size = root_ramfs_size - (size_t)offset;

	if (write_size > *size)
		write_size = *size;

	memcpy(&root_ramfs[(size_t)offset], buffer, write_size);

	return *size = write_size, 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	(void)node;

	memset(stat, 0, sizeof(*stat));
	stat->size = (uint64_t)root_ramfs_size;

	return 0;
}

int vfs_init_root(struct vfs_node **node)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	*node = NULL;

	root_ramfs_size = kernel->arctic_root_size;
	root_ramfs = (unsigned char *)kernel->arctic_root_addr;

	vfs_init_node(&root_ramfs_node, 0);
	root_ramfs_node.type = vfs_type_block;
	root_ramfs_node.n_read = n_read;
	root_ramfs_node.n_write = n_write;
	root_ramfs_node.n_stat = n_stat;

	if ((r = fat_io_create(node, &root_ramfs_node)) != 0)
		return r;

	return 0;
}
