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
 * misc/bin.c
 *      Mount point for native binaries
 */

#include <dancy.h>

static struct vfs_node dev_node;

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	unsigned char *ramfs = (void *)kernel->arctic_bin_addr;
	size_t ramfs_size = kernel->arctic_bin_size;
	size_t read_size;

	(void)node;

	if (offset >= (uint64_t)ramfs_size)
		return *size = 0, 0;

	read_size = ramfs_size - (size_t)offset;

	if (read_size > *size)
		read_size = *size;

	memcpy(buffer, &ramfs[(size_t)offset], read_size);

	return *size = read_size, 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	unsigned char *ramfs = (void *)kernel->arctic_bin_addr;
	size_t ramfs_size = kernel->arctic_bin_size;
	size_t write_size;

	(void)node;

	if (offset >= (uint64_t)ramfs_size)
		return *size = 0, 0;

	write_size = ramfs_size - (size_t)offset;

	if (write_size > *size)
		write_size = *size;

	memcpy(&ramfs[(size_t)offset], buffer, write_size);

	return *size = write_size, 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	(void)node;

	memset(stat, 0, sizeof(*stat));
	stat->size = (uint64_t)kernel->arctic_bin_size;

	return 0;
}

int bin_init(void)
{
	static int run_once;
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (!kernel->arctic_bin_addr || !kernel->arctic_bin_size)
		return DE_UNINITIALIZED;

	if ((r = vfs_open("/bin/", &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	vfs_init_node(&dev_node, 0);
	dev_node.type = vfs_type_block;

	dev_node.n_read = n_read;
	dev_node.n_write = n_write;
	dev_node.n_stat = n_stat;

	if ((r = fat_io_create(&node, &dev_node)) != 0)
		return r;

	if ((r = vfs_mount("/bin/", node)) != 0)
		return r;

	node->n_release(&node);

	return 0;
}
