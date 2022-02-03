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
 * vfs/fat_io.c
 *      File system driver I/O interface
 */

#include <dancy.h>

#define FAT_IO_TOTAL 8

static struct fat_io *fat_io_array[FAT_IO_TOTAL];

static int fat_io_count;
static int fat_io_lock;

static void check_id(int id)
{
	if (id >= 0 && id < fat_io_count)
		return;

	kernel->panic("fat_io: using an uninitialized ID");
}

static struct vfs_node *alloc_node(void)
{
	struct vfs_node *node;
	size_t size = sizeof(*node);

	if ((node = malloc(size)) != NULL) {
		vfs_init_node(node, size);
	}

	return node;
}

int fat_io_add(struct fat_io *io)
{
	void *lock_local = &fat_io_lock;
	int new_id = -1;

	spin_enter(&lock_local);

	if (fat_io_count < FAT_IO_TOTAL) {
		new_id = fat_io_count++;

		io->id = new_id;
		fat_io_array[new_id] = io;
	}

	spin_leave(&lock_local);

	if (new_id < 0)
		return DE_OVERFLOW;

	if ((io->root_node = alloc_node()) == NULL)
		return DE_MEMORY;

	if (mtx_init(&io->fat_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if (fat_create(&io->instance, new_id))
		return DE_MEMORY;

	return 0;
}

int fat_get_size(int id, size_t *block_size, size_t *block_total)
{
	check_id(id);

	return fat_io_array[id]->get_size(id, block_size, block_total);
}

int fat_get_time(char iso_8601_format[19])
{
	char buf[32];

	snprintf(&buf[0], 32, "%04d-%02d-%02dT%02d:%02d:%02d",
		1980, 1, 1, 0, 0, 0);

	memcpy(&iso_8601_format[0], &buf[0], 19);

	return 0;
}

int fat_io_read(int id, size_t lba, size_t *size, void *buf)
{
	check_id(id);

	return fat_io_array[id]->io_read(id, lba, size, buf);
}

int fat_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	check_id(id);

	return fat_io_array[id]->io_write(id, lba, size, buf);
}
