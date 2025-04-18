/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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

#define FAT_IO_TOTAL 64

struct fat_io {
	struct vfs_node *dev_node;

	mtx_t fat_mtx;
	void *instance;
	int id;
	int media_changed;

	int node_count;
	struct vfs_node *node_array[1024];
};

static struct fat_io *fat_io_array[FAT_IO_TOTAL];
static int fat_io_lock;

struct fat_internal_data {
	char path[256];
	struct fat_io *io;
	int fd;
	int media_changed;
};

static void check_id(int id)
{
	if (id >= 0 && id < FAT_IO_TOTAL && fat_io_array[id] != NULL)
		return;

	kernel->panic("fat_io: using an uninitialized ID");
}

static int check_name(const char *name)
{
	size_t i;

	for (i = 0; name[i] != '\0'; i++) {
		char c = name[i];

		if (c >= 'A' && c <= 'Z')
			return 1;

		if (c < 0x20 || c > 0x7E)
			return 1;
	}

	return 0;
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
		r = DE_NOT_EMPTY;
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
		r = DE_ARGUMENT;
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
	case FAT_DIRECTORY_RECORD:
		r = DE_DIRECTORY;
		break;
	case FAT_FILE_RECORD:
		r = DE_FILE;
		break;
	default:
		r = DE_UNEXPECTED;
		break;
	}

	return r;
}

static dancy_time_t calculate_time(int fat_date, int fat_time)
{
	int year, mon, day, hour, min, sec;
	char buf[32];
	long long r;

	year = ((fat_date & 0xFE00) >> 9) + 1980;
	mon = (fat_date & 0x01E0) >> 5;
	day = fat_date & 0x001F;

	hour = (fat_time & 0xF800) >> 11;
	min = (fat_time & 0x07E0) >> 5;
	sec = (fat_time & 0x001F) << 1;

	snprintf(&buf[0], 32, "%04d-%02d-%02dT%02d:%02d:%02d",
		year, mon, day, hour, min, sec);

	if ((r = epoch_seconds(&buf[0])) < 0)
		r = 0;

	return (dancy_time_t)r;
}

static int enter_fat(struct vfs_node *node)
{
	struct fat_internal_data *data = node->internal_data;
	struct fat_io *io = data->io;
	int i, r;

	if (mtx_lock(&io->fat_mtx) != thrd_success)
		kernel->panic("fat_io: unexpected mutex error");

	if (io->media_changed) {
		int fd = INT_MIN;

		if (io->instance)
			fat_delete(io->instance), io->instance = NULL;

		io->media_changed = 0;

		if (fat_create(&io->instance, io->id)) {
			io->instance = NULL;
			io->media_changed = 1;
			mtx_unlock(&io->fat_mtx);
			return DE_MEDIA_CHANGED;
		}

		for (i = 0; i < data->io->node_count; i++) {
			struct vfs_node *n;
			struct fat_internal_data *d;

			if ((n = data->io->node_array[i]) != NULL) {
				d = n->internal_data;

				if (d->path[0] != '/')
					continue;
				if (d->path[1] != '.')
					continue;

				if (d->path[2] == '\0') {
					fd = d->fd;
					d->media_changed = 0;
					break;
				}
			}
		}

		if (fd < 0)
			return 0;

		if ((r = fat_open(io->instance, fd, "/.", "rb+")) != 0) {
			fat_delete(io->instance), io->instance = NULL;
			io->media_changed = 1;
			mtx_unlock(&io->fat_mtx);
			return translate_error(r);
		}
	}

	return 0;
}

static void leave_fat(struct vfs_node *node)
{
	struct fat_internal_data *data = node->internal_data;
	struct fat_io *io = data->io;
	int i;

	if (io->media_changed) {
		for (i = 0; i < data->io->node_count; i++) {
			struct vfs_node *n;
			struct fat_internal_data *d;

			if ((n = data->io->node_array[i]) != NULL) {
				d = n->internal_data;
				d->media_changed = 1;
			}
		}

		if (io->instance)
			fat_delete(io->instance), io->instance = NULL;
	}

	mtx_unlock(&io->fat_mtx);
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct fat_internal_data *data;
	struct fat_io *io;
	void *lock_local;

	int free_fat_io = 0;
	int count;

	*node = NULL;

	if (!n)
		kernel->panic("fat_io: releasing null node");

	data = n->internal_data;
	io = data->io;

	if (mtx_lock(&io->fat_mtx) != thrd_success)
		kernel->panic("fat_io: unexpected mutex error");

	if ((count = vfs_decrement_count(n)) < 0)
		kernel->panic("fat_io: decrementing null node");

	if (!count) {
		int i, fd;

		if ((fd = data->fd) >= 0) {
			if (!io->media_changed && !data->media_changed) {
				if (io->instance)
					fat_close(io->instance, fd);
			}

			if (io->node_count == fd + 1)
				io->node_count -= 1;

			io->node_array[fd] = NULL;
		}

		free_fat_io = 1;

		for (i = 0; i < io->node_count; i++) {
			if (io->node_array[i]) {
				free_fat_io = 0;
				break;
			}
		}

		memset(n, 0, sizeof(*n));
		free(n);
	}

	if (!free_fat_io) {
		mtx_unlock(&io->fat_mtx);
		return;
	}

	if (io->instance)
		fat_delete(io->instance), io->instance = NULL;

	io->dev_node->n_release(&io->dev_node);

	lock_local = &fat_io_lock;
	spin_enter(&lock_local);
	fat_io_array[io->id] = NULL;
	spin_leave(&lock_local);

	mtx_unlock(&io->fat_mtx);
	mtx_destroy(&io->fat_mtx);

	memset(io, 0, sizeof(*io));
	free(io);
}

static struct vfs_node *alloc_node(struct fat_io *io);

static int n_open(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	struct fat_internal_data *data = node->internal_data;
	struct fat_io *io = data->io;
	struct vfs_node *allocated_node;
	int write_record = 0;
	unsigned char record[32];
	char buf[256];
	int size = 0;
	int i, r;

	*new_node = NULL;

	if (type != vfs_type_unknown) {
		if (type != vfs_type_regular && type != vfs_type_directory)
			return DE_TYPE;
	}

	buf[0] = '\0';

	if (check_name(name))
		return DE_ARGUMENT;

	{
		struct vfs_node *owner = node;
		int depth = 0;
		const char *p;

		while (owner->tree[0] != NULL && owner->mount_state == 0)
			owner = owner->tree[0], depth += 1;

		while (depth >= 0) {
			owner = node;
			p = name;

			if (--depth >= 0) {
				for (i = 0; i < depth; i++)
					owner = owner->tree[0];
				p = &owner->name[0];
			}

			while ((buf[size] = (char)tolower((int)*p++)) != 0) {
				if (size > (int)(sizeof(buf) - 3))
					return DE_PATH;
				size += 1;
			}

			buf[size++] = '/', buf[size] = '\0';
		}
	}

	buf[size - 1] = '\0';

	if (buf[0] == '\0')
		return DE_PATH;

	if ((r = enter_fat(node)) != 0)
		return r;

	if ((allocated_node = alloc_node(io)) == NULL)
		return leave_fat(node), DE_MEMORY;

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
		leave_fat(node);
		allocated_node->n_release(&allocated_node);
		return DE_MEMORY;
	}

	if (type == vfs_type_directory)
		buf[size - 1] = '/';

	if ((mode & vfs_mode_exclusive) != 0) {
		if ((mode & vfs_mode_create) != 0) {
			r = fat_open(io->instance, data->fd, &buf[0], "rb");

			if (!r) {
				fat_close(io->instance, data->fd);
				leave_fat(node);
				allocated_node->n_release(&allocated_node);
				return DE_BUSY;
			}

			if (r != FAT_FILE_NOT_FOUND) {
				leave_fat(node);
				allocated_node->n_release(&allocated_node);
				return translate_error(r);
			}
		}
	}

	r = fat_open(io->instance, data->fd, &buf[0], "rb+");

	if (r == FAT_READ_ONLY_FILE)
		r = fat_open(io->instance, data->fd, &buf[0], "rb");

	if (r == FAT_FILE_NOT_FOUND && (mode & vfs_mode_create) != 0)
		r = fat_open(io->instance, data->fd, &buf[0], "wb+");

	if (!r)
		r = fat_control(io->instance, data->fd, 0, record);

	if (!r) {
		unsigned long file_size = 0;
		int fat_attributes = (int)record[11];

		if ((mode & vfs_mode_truncate) != 0) {
			W_LE32(&record[28], file_size);
			write_record = 1;
		}

		if ((mode & vfs_mode_exclusive) != 0)
			allocated_node->mode |= vfs_mode_exclusive;

		if ((fat_attributes & 0x01) != 0)
			allocated_node->mode |= vfs_mode_read_only;
		if ((fat_attributes & 0x02) != 0)
			allocated_node->mode |= vfs_mode_hidden;
		if ((fat_attributes & 0x04) != 0)
			allocated_node->mode |= vfs_mode_system;

		if ((fat_attributes & 0x10) != 0)
			allocated_node->type = vfs_type_directory;
		else
			allocated_node->type = vfs_type_regular;
	}

	if (write_record)
		r = fat_control(io->instance, data->fd, 1, record);

	leave_fat(node);

	if (r) {
		allocated_node->n_release(&allocated_node);
		return translate_error(r);
	}

	*new_node = allocated_node;

	return 0;
}

static int n_read_write_common(struct vfs_node *node,
	uint64_t offset, size_t *size, addr_t buffer_addr, int write_mode)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	size_t retval = *size;
	int fat_offset[2];
	int r;

	*size = 0;

	if (node->type == vfs_type_directory)
		return DE_DIRECTORY;

	if (offset >= 0xFFFFFFFF)
		return 0;

	if (offset <= INT_MAX) {
		fat_offset[0] = (int)offset;
		fat_offset[1] = 0;
	} else {
		fat_offset[0] = INT_MAX;
		fat_offset[1] = (int)(offset - INT_MAX);
	}

	if ((r = enter_fat(node)) != 0)
		return r;

	instance = data->io->instance;
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

	leave_fat(node);

	if (r)
		return translate_error(r);

	return *size = retval, 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	return n_read_write_common(node, offset, size, (addr_t)buffer, 0);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	return n_read_write_common(node, offset, size, (addr_t)buffer, 1);
}

static int n_append(struct vfs_node *node, size_t *size, const void *buffer)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	size_t retval = *size;
	unsigned char record[32];
	int fat_offset[2];
	int r;

	*size = 0;

	if (node->type == vfs_type_directory)
		return DE_DIRECTORY;

	if ((r = enter_fat(node)) != 0)
		return r;

	instance = data->io->instance;

	if ((r = fat_control(instance, data->fd, 0, record)) != 0)
		return leave_fat(node), translate_error(r);

	{
		unsigned long file_size = LE32(&record[28]);

		if (file_size == 0xFFFFFFFF)
			return leave_fat(node), 0;

		if (file_size <= INT_MAX) {
			fat_offset[0] = (int)file_size;
			fat_offset[1] = 0;
		} else {
			fat_offset[0] = INT_MAX;
			fat_offset[1] = (int)(file_size - INT_MAX);
		}
	}

	r = fat_seek(instance, data->fd, fat_offset[0], 0);

	if (fat_offset[1] && !r)
		r = fat_seek(instance , data->fd, fat_offset[1], 1);

	if (!r)
		r = fat_write(instance, data->fd, &retval, buffer);

	leave_fat(node);

	if (r)
		return translate_error(r);

	return *size = retval, 0;
}

static int n_sync(struct vfs_node *node)
{
	struct fat_internal_data *data = node->internal_data;
	int r;

	if ((r = enter_fat(node)) != 0)
		return r;

	r = data->io->dev_node->n_sync(data->io->dev_node);

	if (r == DE_MEDIA_CHANGED)
		data->io->media_changed = 1;

	leave_fat(node);

	return r;
}

static int n_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	int read_offset;
	int i, r;

	memset(dent, 0, sizeof(*dent));

	if (node->type != vfs_type_directory)
		return DE_TYPE;

	if (offset == 0) {
		strcpy(&dent->name[0], ".");
		return 0;
	}

	if (offset == 1) {
		strcpy(&dent->name[0], "..");
		return 0;
	}

	if (offset > 0x10000)
		return 0;

	if ((r = enter_fat(node)) != 0) {
		if (r != DE_MEDIA_CHANGED)
			return r;
		if ((r = enter_fat(node)) != 0)
			return r;
	}

	r = data->io->dev_node->n_sync(data->io->dev_node);

	if (r == DE_MEDIA_CHANGED) {
		data->io->media_changed = 1;
		return leave_fat(node), DE_MEDIA_CHANGED;
	}

	if (!strcmp(&data->path[0], "/."))
		read_offset = (int)(offset - 2) * 32;
	else
		read_offset = (int)offset * 32;

	instance = data->io->instance;
	r = fat_seek(instance, data->fd, read_offset, 0);

	if (!r) {
		unsigned char fat_record[32];
		size_t read_size = 32;
		int fat_attributes;

		r = fat_read(instance, data->fd, &read_size, &fat_record[0]);

		if (!r) {
			char *name = &dent->name[0];
			int base_size = 8;
			int ext_size = 3;

			if (read_size != 32 || fat_record[0] == 0)
				return leave_fat(node), 0;

			fat_attributes = (int)fat_record[11];

			if (fat_record[0] == 0xE5)
				return leave_fat(node), DE_PLACEHOLDER;

			if ((fat_attributes & 0x08) != 0)
				return leave_fat(node), DE_PLACEHOLDER;

			for (i = 0; i < 11; i++) {
				int c = (int)fat_record[i];

				if (c == '\0') {
					leave_fat(node);
					return DE_UNEXPECTED;
				}

				if (c >= 'A' && c <= 'Z') {
					int lower = c + 32;
					fat_record[i] = (unsigned char)lower;
				}
			}

			for (i = 7; i >= 0; i--) {
				if (fat_record[i] != 0x20)
					break;
				base_size -= 1;
			}

			if (base_size == 0)
				return leave_fat(node), DE_PLACEHOLDER;

			for (i = 10; i >= 8; i--) {
				if (fat_record[i] != 0x20)
					break;
				ext_size -= 1;
			}

			for (i = 0; i < base_size; i++)
				*name++ = (char)fat_record[i];

			if (ext_size != 0)
				*name++ = '.';

			for (i = 8; i < ext_size + 8; i++)
				*name++ = (char)fat_record[i];
		}
	}

	leave_fat(node);

	return (r != 0) ? translate_error(r) : 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	unsigned char record[32];
	int r, fat_date, fat_time;

	memset(stat, 0, sizeof(*stat));

	if ((r = enter_fat(node)) != 0)
		return r;

	instance = data->io->instance;
	r = fat_control(instance, data->fd, 0, record);

	leave_fat(node);

	if (r) {
		if (!strcmp(&data->path[0], "/."))
			return 0;
		return translate_error(r);
	}

	stat->size = (uint64_t)LE32(&record[28]);

	fat_date = (int)LE16(&record[18]), fat_time = 0;
	stat->access_time.tv_sec = calculate_time(fat_date, fat_time);

	fat_date = (int)LE16(&record[16]), fat_time = (int)LE16(&record[14]);
	stat->creation_time.tv_sec = calculate_time(fat_date, fat_time);

	fat_date = (int)LE16(&record[24]), fat_time = (int)LE16(&record[22]);
	stat->write_time.tv_sec = calculate_time(fat_date, fat_time);

	return 0;
}

static int n_truncate(struct vfs_node *node, uint64_t size)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	unsigned char record[32];
	int extend_file = 0;
	int r;

	if (size > 0xFFFFFFFF)
		return DE_OVERFLOW;

	if ((r = enter_fat(node)) != 0)
		return r;

	instance = data->io->instance;

	if ((r = fat_control(instance, data->fd, 0, record)) == 0) {
		int fat_attributes = (int)record[11];
		unsigned long fat_size = LE32(&record[28]);

		if ((fat_attributes & 0x1D) != 0)
			return leave_fat(node), DE_READ_ONLY;

		if (fat_size < (unsigned long)size)
			extend_file = 1;

		if (fat_size > (unsigned long)size) {
			fat_size = (unsigned long)size;
			W_LE32(&record[28], fat_size);
			r = fat_control(instance, data->fd, 1, record);
		}
	}

	leave_fat(node);

	if (extend_file) {
		uint64_t offset = size - 1;
		unsigned char buf[1] = { 0 };
		addr_t addr = (addr_t)&buf[0];
		size_t one_byte = 1;

		r = n_read_write_common(node, offset, &one_byte, addr, 1);
	}

	return (r != 0) ? translate_error(r) : 0;
}

static int n_remove(struct vfs_node *node, const char *name, int dir)
{
	void *instance;
	struct fat_internal_data *data = node->internal_data;
	char buf[256];
	int size = 0;
	int i, r;

	buf[0] = '\0';

	if (check_name(name))
		return DE_ARGUMENT;

	{
		struct vfs_node *owner = node;
		int depth = 0;
		const char *p;

		while (owner->tree[0] != NULL && owner->mount_state == 0)
			owner = owner->tree[0], depth += 1;

		while (depth >= 0) {
			owner = node;
			p = name;

			if (--depth >= 0) {
				for (i = 0; i < depth; i++)
					owner = owner->tree[0];
				p = &owner->name[0];
			}

			while ((buf[size] = (char)tolower((int)*p++)) != 0) {
				if (size > (int)(sizeof(buf) - 3))
					return DE_PATH;
				size += 1;
			}

			buf[size++] = '/', buf[size] = '\0';
		}
	}

	buf[size - 1] = '\0';

	if (buf[0] == '\0')
		return 0;

	if ((r = enter_fat(node)) != 0)
		return r;

	instance = data->io->instance;

	if (dir)
		buf[size - 1] = '/';

	r = fat_remove(instance, &buf[0]);

	leave_fat(node);

	return (r != 0) ? translate_error(r) : 0;
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

		node->n_release  = n_release;
		node->n_open     = n_open;
		node->n_read     = n_read;
		node->n_write    = n_write;
		node->n_append   = n_append;
		node->n_sync     = n_sync;
		node->n_readdir  = n_readdir;
		node->n_stat     = n_stat;
		node->n_truncate = n_truncate;
		node->n_remove   = n_remove;

		data = node->internal_data;
		data->io = io;
		data->fd = -1;
	}

	return node;
}

static void set_fat_io_array_null(int id)
{
	void *lock_local = &fat_io_lock;

	spin_enter(&lock_local);
	fat_io_array[id] = NULL;
	spin_leave(&lock_local);
}

static int fat_io_add(struct fat_io *io)
{
	void *lock_local = &fat_io_lock;
	struct fat_internal_data *data;
	struct vfs_node *root_node;
	int new_id = -1;
	int i;

	spin_enter(&lock_local);

	for (i = 0; i < FAT_IO_TOTAL; i++) {
		if (!fat_io_array[i]) {
			io->id = new_id = i;
			fat_io_array[i] = io;
			break;
		}
	}

	spin_leave(&lock_local);

	if (new_id < 0)
		return DE_OVERFLOW;

	if (mtx_init(&io->fat_mtx, mtx_plain) != thrd_success) {
		set_fat_io_array_null(new_id);
		return DE_UNEXPECTED;
	}

	if ((root_node = alloc_node(io)) == NULL) {
		mtx_destroy(&io->fat_mtx);
		set_fat_io_array_null(new_id);
		return DE_MEMORY;
	}

	root_node->count = 1;
	root_node->type = vfs_type_directory;

	data = root_node->internal_data;
	data->path[0] = '/', data->path[1] = '.', data->path[2] = '\0';
	data->fd = 0;
	data->media_changed = 1;

	io->instance = NULL;
	io->media_changed = 1;
	io->node_count = 1;
	io->node_array[0] = root_node;

	return 0;
}

int fat_io_create(struct vfs_node **new_node, struct vfs_node *dev_node)
{
	struct fat_io *io;
	int r;

	*new_node = NULL;

	if ((io = malloc(sizeof(*io))) == NULL)
		return DE_MEMORY;

	memset(io, 0, sizeof(*io));

	vfs_increment_count(dev_node);
	io->dev_node = dev_node;

	if ((r = fat_io_add(io)) != 0) {
		io->dev_node->n_release(&io->dev_node);
		return free(io), r;
	}

	return *new_node = io->node_array[0], 0;
}

int fat_get_size(int id, size_t *block_size, size_t *block_total)
{
	struct vfs_node *node;
	struct vfs_stat stat;

	check_id(id);
	node = fat_io_array[id]->dev_node;

	if (node->n_stat(node, &stat)) {
		*block_size = 0;
		*block_total = 0;
		return 1;
	}

	if (stat.block_size)
		*block_size = stat.block_size;
	else
		*block_size = 512;

	*block_total = (size_t)(stat.size / (*block_size));

	return 0;
}

int fat_get_time(char iso_8601_format[19])
{
	epoch_convert(kernel->epoch_read(), &iso_8601_format[0]);

	return 0;
}

int fat_io_read(int id, size_t lba, size_t *size, void *buf)
{
	size_t requested_size = *size;
	struct fat_io *io;
	struct vfs_node *node;
	struct vfs_stat stat;
	uint64_t offset;
	int r;

	check_id(id);
	io = fat_io_array[id];

	if (io->media_changed)
		return 1;

	node = io->dev_node;

	if (node->n_stat(node, &stat))
		return 1;

	if (stat.block_size == 0)
		stat.block_size = 512;

	offset = (uint64_t)lba * (uint64_t)stat.block_size;
	r = node->n_read(node, offset, size, buf);

	if (r == DE_MEDIA_CHANGED)
		io->media_changed = 1;

	if (*size != requested_size)
		return 1;

	return (r != 0);
}

int fat_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	size_t requested_size = *size;
	struct fat_io *io;
	struct vfs_node *node;
	struct vfs_stat stat;
	uint64_t offset;
	int r;

	check_id(id);
	io = fat_io_array[id];

	if (io->media_changed)
		return 1;

	node = io->dev_node;

	if (node->n_stat(node, &stat))
		return 1;

	if (stat.block_size == 0)
		stat.block_size = 512;

	offset = (uint64_t)lba * (uint64_t)stat.block_size;
	r = node->n_write(node, offset, size, buf);

	if (r == DE_MEDIA_CHANGED)
		io->media_changed = 1;

	if (*size != requested_size)
		return 1;

	return (r != 0);
}
