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
static int fat_io_lock;

struct fat_internal_data {
	char path[256];
	struct fat_io *io;
	int fd;
};

static void check_id(int id)
{
	if (id >= 0 && id < FAT_IO_TOTAL && fat_io_array[id] != NULL)
		return;

	kernel->panic("fat_io: using an uninitialized ID");
}

static struct vfs_node *find_node(struct vfs_node *node, const char *name)
{
	struct fat_internal_data *data = node->internal_data;
	struct vfs_node *existing_node = NULL;
	struct vfs_node *n;
	int i;

	for (i = 0; i < data->io->node_count; i++) {
		if ((n = data->io->node_array[i]) != NULL) {
			struct fat_internal_data *d = n->internal_data;

			if (!strcmp(&d->path[0], name)) {
				existing_node = n;
				break;
			}
		}
	}

	return existing_node;
}

static int translate_error(int fat_error)
{
	int r;

	switch (fat_error) {
	case FAT_BLOCK_READ_ERROR:
		r = DE_BLOCK_READ;
		break;
	case FAT_BLOCK_WRITE_ERROR:
		r = DE_BLOCK_WRITE;
		break;
	case FAT_DIRECTORY_NOT_EMPTY:
		r = DE_BUSY;
		break;
	case FAT_FILE_ALREADY_OPEN:
		r = DE_BUSY;
		break;
	case FAT_FILE_NOT_FOUND:
		r = DE_NAME;
		break;
	case FAT_INCONSISTENT_STATE:
		r = DE_UNEXPECTED;
		break;
	case FAT_INVALID_FILE_NAME:
		r = DE_PATH;
		break;
	case FAT_INVALID_PARAMETERS:
		r = DE_ARGUMENT;
		break;
	case FAT_NOT_ENOUGH_SPACE:
		r = DE_FULL;
		break;
	case FAT_NOT_READY:
		r = DE_UNINITIALIZED;
		break;
	case FAT_READ_ONLY_FILE:
		r = DE_READ_ONLY;
		break;
	case FAT_READ_ONLY_RECORD:
		r = DE_READ_ONLY;
		break;
	case FAT_SEEK_ERROR:
		r = DE_SEEK;
		break;
	default:
		r = DE_UNEXPECTED;
		break;
	}

	return r;
}

static void lock_fat(struct vfs_node *node)
{
	struct fat_internal_data *data = node->internal_data;

	if (mtx_lock(&data->io->fat_mtx) != thrd_success)
		kernel->panic("fat_io: unexpected mutex error");
}

static void unlock_fat(struct vfs_node *node)
{
	struct fat_internal_data *data = node->internal_data;

	mtx_unlock(&data->io->fat_mtx);
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct fat_internal_data *data = n->internal_data;

	if (n && !vfs_decrement_count(n)) {
		int fd = data->fd;

		lock_fat(n);

		if (fd >= 0) {
			fat_close(data->io->instance, fd);

			if (data->io->node_count == fd + 1)
				data->io->node_count -= 1;

			data->io->node_array[fd] = NULL;
		}

		unlock_fat(n);

		n->internal_data = NULL;
		free(n);
	}

	*node = NULL;
}

static struct vfs_node *alloc_node(struct fat_io *io);

static int n_create(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname)
{
	struct fat_internal_data *data = node->internal_data;
	struct fat_io *io = data->io;
	struct vfs_node *allocated_node, *existing_node;
	int write_record = 0;
	unsigned char record[32];
	char buf[256];
	int size = 0;
	int i, r;

	buf[0] = '\0';
	*new_node = NULL;

	for (i = vname->offset; vname->components[i] != NULL; i++) {
		char *p = vname->components[i];

		while ((buf[size] = (char)tolower((int)*p++)) != '\0') {
			if (size > (int)(sizeof(buf) - 3))
				return DE_PATH;
			size += 1;
		}

		buf[size++] = '/', buf[size] = '\0';
	}

	if (buf[0] == '\0') {
		vfs_increment_count(io->root_node);
		*new_node = io->root_node;
		return 0;
	}

	buf[size - 1] = '\0';

	lock_fat(node);

	if ((existing_node = find_node(node, &buf[0])) != NULL) {
		data = existing_node->internal_data;
		r = fat_control(io->instance, data->fd, 0, record);

		if ((mode & vfs_mode_create) != 0)
			write_record = 1;

		if ((mode & vfs_mode_truncate) != 0)
			write_record = 1;

		if (!r && write_record) {
			unsigned long file_size = 0;

			W_LE32(&record[28], file_size);
			r = fat_control(io->instance, data->fd, 1, record);
		}

		if (r)
			return unlock_fat(node), translate_error(r);

		vfs_increment_count(existing_node);
		*new_node = existing_node;

		return unlock_fat(node), 0;
	}

	if ((allocated_node = alloc_node(io)) == NULL)
		return unlock_fat(node), DE_MEMORY;

	data = allocated_node->internal_data;
	strcpy(&data->path[0], &buf[0]);
	i = 0;

	while (i < (int)(sizeof(io->node_array) / sizeof(*io->node_array))) {
		if (io->node_array[i] == NULL) {
			if (io->node_count < i + 1)
				io->node_count = i + 1;
			io->node_array[i] = allocated_node;
			data->fd = i;
			break;
		}
		i += 1;
	}

	if (data->fd < 0) {
		unlock_fat(node);
		allocated_node->n_release(&allocated_node);
		return DE_MEMORY;
	}

	allocated_node->type = vfs_type_regular;

	if (type == vfs_type_directory || vname->type == vfs_type_directory) {
		allocated_node->type = vfs_type_directory;
		buf[size - 1] = '/';
	}

	if ((mode & vfs_mode_create) != 0)
		r = fat_open(io->instance, data->fd, &buf[0], "wb+");
	else
		r = fat_open(io->instance, data->fd, &buf[0], "rb+");

	if (!r)
		r = fat_control(io->instance, data->fd, 0, record);

	if (!r) {
		unsigned long file_size = 0;

		if ((mode & vfs_mode_truncate) != 0) {
			W_LE32(&record[28], file_size);
			write_record = 1;
		}

		file_size = LE32(&record[28]);
	}

	if (write_record)
		r = fat_control(io->instance, data->fd, 1, record);

	unlock_fat(node);

	if (r) {
		allocated_node->n_release(&allocated_node);
		return translate_error(r);
	}

	*new_node = allocated_node;

	return 0;
}

static long long n_read_write_common(struct vfs_node *node,
	uint64_t offset, size_t size, addr_t buffer_addr, int write_mode)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	size_t retval = size;
	int fat_offset[2];
	int r;

	if (offset >= 0xFFFFFFFF)
		return 0;

	if (offset <= INT_MAX) {
		fat_offset[0] = (int)offset;
		fat_offset[1] = 0;
	} else {
		fat_offset[0] = INT_MAX;
		fat_offset[1] = (int)(offset - INT_MAX);
	}

	lock_fat(node);

	r = fat_seek(instance, data->fd, fat_offset[0], 0);

	if (fat_offset[1] && !r)
		r = fat_seek(instance , data->fd, fat_offset[1], 1);

	if (!r) {
		if (!write_mode) {
			void *buffer = (void *)buffer_addr;
			r = fat_read(instance, data->fd, &retval, buffer);
		} else {
			const void *buffer = (const void *)buffer_addr;
			r = fat_write(instance, data->fd, &retval, buffer);
		}
	}

	unlock_fat(node);

	if (r)
		return -1LL;

	return (long long)retval;
}

static long long n_read(struct vfs_node *node,
	uint64_t offset, size_t size, void *buffer)
{
	return n_read_write_common(node, offset, size, (addr_t)buffer, 0);
}

static long long n_write(struct vfs_node *node,
	uint64_t offset, size_t size, const void *buffer)
{
	return n_read_write_common(node, offset, size, (addr_t)buffer, 1);
}

static struct vfs_node *alloc_node(struct fat_io *io)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += sizeof(struct fat_internal_data);

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;
		struct fat_internal_data *data;

		vfs_init_node(node, size);

		node->count = 1;
		node->internal_data = (void *)a;

		node->n_release = n_release;
		node->n_create  = n_create;
		node->n_read    = n_read;
		node->n_write   = n_write;

		data = node->internal_data;
		data->io = io;
		data->fd = -1;
	}

	return node;
}

int fat_io_add(struct fat_io *io)
{
	void *lock_local = &fat_io_lock;
	struct fat_internal_data *data;
	int new_id = -1;
	int i, r;

	spin_enter(&lock_local);

	for (i = 0; i < FAT_IO_TOTAL; i++) {
		if (!fat_io_array[i]) {
			io->id = new_id = i;
			fat_io_array[i] = io;
		}
	}

	spin_leave(&lock_local);

	if (new_id < 0)
		return DE_OVERFLOW;

	if (fat_create(&io->instance, new_id)) {
		fat_io_array[new_id] = NULL;
		return DE_UNEXPECTED;
	}

	if (mtx_init(&io->fat_mtx, mtx_plain) != thrd_success) {
		fat_delete(io->instance), io->instance = NULL;

		fat_io_array[new_id] = NULL;
		return DE_UNEXPECTED;
	}

	if ((io->root_node = alloc_node(io)) == NULL) {
		mtx_destroy(&io->fat_mtx);
		fat_delete(io->instance), io->instance = NULL;

		fat_io_array[new_id] = NULL;
		return DE_MEMORY;
	}

	io->root_node->count = -1;
	io->root_node->type = vfs_type_directory;

	lock_fat(io->root_node);

	data = io->root_node->internal_data;
	data->fd = 0;

	io->node_count = 1;
	io->node_array[0] = io->root_node;

	r = fat_open(io->instance, data->fd, "/.", "rb+");

	unlock_fat(io->root_node);

	if (r != 0) {
		io->root_node->n_release(&io->root_node);
		mtx_destroy(&io->fat_mtx);
		fat_delete(io->instance), io->instance = NULL;

		fat_io_array[new_id] = NULL;
		return translate_error(r);
	}

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
