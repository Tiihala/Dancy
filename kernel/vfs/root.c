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

static struct vfs_node *root_node;

static int root_id;
static struct fat_io root_io;

static int root_get_size(int id, size_t *block_size, size_t *block_total);
static int root_io_read(int id, size_t lba, size_t *size, void *buf);
static int root_io_write(int id, size_t lba, size_t *size, const void *buf);

int vfs_init_root(struct vfs_node **node)
{
	int r;

	if ((root_node = vfs_alloc_node()) == NULL)
		return DE_MEMORY;

	root_io.get_size = root_get_size;
	root_io.io_read  = root_io_read;
	root_io.io_write = root_io_write;

	if ((r = fat_io_add(&root_io, &root_id)) != 0)
		return r;

	*node = root_node;

	return 0;
}

static int root_get_size(int id, size_t *block_size, size_t *block_total)
{
	(void)id;
	(void)block_size;
	(void)block_total;

	return 1;
}

static int root_io_read(int id, size_t lba, size_t *size, void *buf)
{
	(void)id;
	(void)lba;
	(void)buf;

	*size = 0;

	return 1;
}

static int root_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	(void)id;
	(void)lba;
	(void)buf;

	*size = 0;

	return 1;
}
