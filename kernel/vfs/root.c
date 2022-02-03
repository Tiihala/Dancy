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

static struct fat_io root_io;

static size_t root_ramfs_size;
static unsigned char *root_ramfs;

static size_t root_block_size;
static size_t root_block_total;

static int create_ramfs(void)
{
	static const unsigned char parameter_block[] = {
		/*
		 * 1 MiB File System (FAT 12).
		 */
	DANCY_BLOB(0xEB3C,0x906D,0x6B66,0x732E,0x6661,0x7400,0x1004,0x0100),
	DANCY_BLOB(0x0180,0x0000,0x01F8,0x0100,0x1000,0x0200,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x8000,0x29C7,0xC6EB,0x2F4E,0x4F20,0x4E41),
	DANCY_BLOB(0x4D45,0x2020,0x2020,0x4641,0x5431,0x3220,0x2020,0xEBFE)};

	root_ramfs_size = 0x100000;
	root_ramfs = (unsigned char *)mm_alloc_pages(mm_kernel, 8);

	if (!root_ramfs)
		return DE_MEMORY;

	memset(root_ramfs, 0, root_ramfs_size);
	memcpy(&root_ramfs[0], &parameter_block[0], sizeof(parameter_block));

	root_ramfs[0x01FE] = 0x55;
	root_ramfs[0x01FF] = 0xAA;

	root_ramfs[0x1000] = 0xF8;
	root_ramfs[0x1001] = 0xFF;
	root_ramfs[0x1002] = 0xFF;

	root_block_size = 0x1000;
	root_block_total = 0x100;

	return 0;
}

static int root_get_size(int id, size_t *block_size, size_t *block_total)
{
	if (id != root_io.id)
		kernel->panic("root_get_size: unexpected behavior");

	*block_size = root_block_size;
	*block_total = root_block_total;

	return 0;
}

static int root_io_read(int id, size_t lba, size_t *size, void *buf)
{
	size_t offset = lba * root_block_size;

	if (id != root_io.id)
		kernel->panic("root_io_read: unexpected behavior");

	memcpy(buf, &root_ramfs[offset], *size);

	return 0;
}

static int root_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	size_t offset = lba * root_block_size;

	if (id != root_io.id)
		kernel->panic("root_io_write: unexpected behavior");

	memcpy(&root_ramfs[offset], buf, *size);

	return 0;
}

int vfs_init_root(struct vfs_node **node)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	*node = NULL;

	if ((r = create_ramfs()) != 0)
		return r;

	root_io.get_size = root_get_size;
	root_io.io_read  = root_io_read;
	root_io.io_write = root_io_write;

	if ((r = fat_io_add(&root_io)) != 0)
		return r;

	*node = root_io.root_node;

	return 0;
}
