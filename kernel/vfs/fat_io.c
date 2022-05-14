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

struct fat_io {
	struct vfs_node *dev_node;
	struct vfs_node *root_node;

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

static int n_open_internal(struct vfs_node *node, struct vfs_node **new_node,
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

	if (type != vfs_type_unknown) {
		if (type != vfs_type_regular && type != vfs_type_directory)
			return DE_TYPE;
	}

	for (i = 0; vname->components[i] != NULL; i++) {
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

		if ((mode & vfs_mode_exclusive) != 0)
			return unlock_fat(node), DE_BUSY;

		if ((existing_node->mode & vfs_mode_exclusive) != 0)
			return unlock_fat(node), DE_BUSY;

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

	if (type == vfs_type_directory)
		buf[size - 1] = '/';

	if ((mode & vfs_mode_exclusive) != 0) {
		if ((mode & vfs_mode_create) != 0) {
			r = fat_open(io->instance, data->fd, &buf[0], "rb");

			if (!r) {
				fat_close(io->instance, data->fd);
				unlock_fat(node);
				allocated_node->n_release(&allocated_node);
				return DE_BUSY;
			}

			if (r != FAT_FILE_NOT_FOUND) {
				unlock_fat(node);
				allocated_node->n_release(&allocated_node);
				return translate_error(r);
			}
		}
	}

	if ((mode & vfs_mode_create) != 0)
		r = fat_open(io->instance, data->fd, &buf[0], "wb+");
	else
		r = fat_open(io->instance, data->fd, &buf[0], "rb+");

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

	unlock_fat(node);

	if (r) {
		allocated_node->n_release(&allocated_node);
		return translate_error(r);
	}

	*new_node = allocated_node;

	return 0;
}

static int n_open(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname)
{
	struct vfs_node *dir_node = NULL;
	int count = 0;
	void *last;
	int r;

	while (vname->components[count] != NULL)
		count += 1;

	if ((mode & vfs_mode_create) == 0 || count < 2)
		return n_open_internal(node, new_node, type, mode, vname);

	last = vname->components[count - 1];
	vname->components[count - 1] = NULL;

	r = n_open_internal(node, &dir_node, vfs_type_directory, 0, vname);
	r = (r == DE_NAME) ? DE_PATH : r;

	vname->components[count - 1] = last;

	if (dir_node) {
		r = n_open_internal(dir_node, new_node, type, mode, vname);
		dir_node->n_release(&dir_node);
	}

	return r;
}

static int n_read_write_common(struct vfs_node *node,
	uint64_t offset, size_t *size, addr_t buffer_addr, int write_mode)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	size_t retval = *size;
	int fat_offset[2];
	int r;

	*size = 0;

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

static int n_readdir(struct vfs_node *node,
	uint64_t offset, size_t size, void *record)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	int r, read_offset;
	size_t vname_size;
	char *vname;

	if (!size || !record)
		return DE_BUFFER;

	memset(record, 0, size);

	vname_size = size;
	vname = (char *)record;

	if (node->type != vfs_type_directory)
		return DE_TYPE;

	if (offset > 0x10000)
		return DE_OVERFLOW;

	read_offset = (int)offset * 32;

	lock_fat(node);

	r = fat_seek(instance, data->fd, read_offset, 0);

	if (!r) {
		unsigned char fat_record[32];
		size_t read_size = 32;
		int fat_attributes;

		r = fat_read(instance, data->fd, &read_size, &fat_record[0]);

		if (!r) {
			int base_size = 8;
			int ext_size = 3;
			int total_size;
			int i;

			if (read_size != 32 || fat_record[0] == 0)
				return unlock_fat(node), DE_OVERFLOW;

			fat_attributes = (int)fat_record[11];

			if (fat_record[0] == 0xE5)
				return unlock_fat(node), DE_PLACEHOLDER;

			if ((fat_attributes & 0x08) != 0)
				return unlock_fat(node), DE_PLACEHOLDER;

			for (i = 0; i < 11; i++) {
				int c = (int)fat_record[i];

				if (c == '\0') {
					unlock_fat(node);
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
				return unlock_fat(node), DE_PLACEHOLDER;

			for (i = 10; i >= 8; i--) {
				if (fat_record[i] != 0x20)
					break;
				ext_size -= 1;
			}

			total_size = base_size;

			if (ext_size != 0)
				total_size += (1 + ext_size);

			if ((size_t)total_size >= vname_size)
				return unlock_fat(node), DE_BUFFER;

			vname_size = 0;

			for (i = 0; i < base_size; i++)
				vname[vname_size++] = (char)fat_record[i];

			if (ext_size != 0)
				vname[vname_size++] = '.';

			for (i = 8; i < ext_size + 8; i++)
				vname[vname_size++] = (char)fat_record[i];
		}
	}

	unlock_fat(node);

	return (r != 0) ? translate_error(r) : 0;
}

static int n_rename(struct vfs_node *node,
	struct vfs_name *old_vname, struct vfs_name *new_vname)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	char *tmp_buf, *buf1, *buf2;
	int size1 = 0, size2 = 0;
	int i, r;

	if ((tmp_buf = malloc(512)) == NULL)
		return DE_MEMORY;

	buf1 = tmp_buf + 0;
	buf2 = tmp_buf + 256;

	buf1[0] = '\0', buf2[0] = '\0';

	for (i = 0; old_vname->components[i] != NULL; i++) {
		char *p = old_vname->components[i];

		while ((buf1[size1] = (char)tolower((int)*p++)) != '\0') {
			if (size1 > 253)
				return free(tmp_buf), DE_PATH;
			size1 += 1;
		}

		buf1[size1++] = '/', buf1[size1] = '\0';
	}

	for (i = 0; new_vname->components[i] != NULL; i++) {
		char *p = new_vname->components[i];

		while ((buf2[size2] = (char)tolower((int)*p++)) != '\0') {
			if (size2 > 253)
				return free(tmp_buf), DE_PATH;
			size2 += 1;
		}

		buf2[size2++] = '/', buf2[size2] = '\0';
	}

	if (buf1[0] == '\0' || buf2[0] == '\0')
		return free(tmp_buf), DE_PATH;

	buf1[size1 - 1] = '\0';
	buf2[size2 - 1] = '\0';

	if (!strcmp(&buf1[0], &buf2[0])) {
		struct vfs_node *test_node;

		if ((r = n_open(node, &test_node, 0, 0, old_vname)) != 0)
			return free(tmp_buf), r;

		test_node->n_release(&test_node);
		return free(tmp_buf), 0;
	}

	lock_fat(node);

	if (find_node(node, &buf1[0]) || find_node(node, &buf2[0]))
		return unlock_fat(node), free(tmp_buf), DE_BUSY;

	if (new_vname->type == vfs_type_directory) {
		buf1[size1 - 1] = '/';
		buf2[size2 - 1] = '/';
	}

	r = fat_rename(instance, &buf1[0], &buf2[0]);

	unlock_fat(node);

	if (r && r != FAT_FILE_NOT_FOUND) {
		int buf_modified = 0;

		for (size1 = size1 - 2; size1 >= 0; size1--) {
			if (buf1[size1] == '/') {
				buf1[size1] = '\0';
				buf_modified = 1;
				break;
			}
		}

		for (size2 = size2 - 2; size2 >= 0; size2--) {
			if (buf2[size2] == '/') {
				buf2[size2] = '\0';
				buf_modified = 1;
				break;
			}
		}

		if (buf_modified && strcmp(&buf1[0], &buf2[0]))
			return free(tmp_buf), DE_UNSUPPORTED;
	}

	free(tmp_buf);

	return (r != 0) ? translate_error(r) : 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	unsigned char record[32];
	int r, fat_date, fat_time;

	memset(stat, 0, sizeof(*stat));

	lock_fat(node);
	r = fat_control(instance, data->fd, 0, record);
	unlock_fat(node);

	if (r)
		return translate_error(r);

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
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	unsigned char record[32];
	int extend_file = 0;
	int r;

	if (size > 0xFFFFFFFF)
		return DE_OVERFLOW;

	lock_fat(node);

	if ((r = fat_control(instance, data->fd, 0, record)) == 0) {
		unsigned long fat_size = LE32(&record[28]);

		if (fat_size < (unsigned long)size)
			extend_file = 1;

		if (fat_size > (unsigned long)size) {
			fat_size = (unsigned long)size;
			W_LE32(&record[28], fat_size);
			r = fat_control(instance, data->fd, 1, record);
		}
	}

	unlock_fat(node);

	if (extend_file) {
		uint64_t offset = size - 1;
		unsigned char buf[1] = { 0 };
		addr_t addr = (addr_t)&buf[0];
		size_t one_byte = 1;

		r = n_read_write_common(node, offset, &one_byte, addr, 1);
	}

	return (r != 0) ? translate_error(r) : 0;
}

static int n_unlink(struct vfs_node *node, struct vfs_name *vname)
{
	struct fat_internal_data *data = node->internal_data;
	void *instance = data->io->instance;
	char buf[256];
	int size = 0;
	int i, r;

	buf[0] = '\0';

	for (i = 0; vname->components[i] != NULL; i++) {
		char *p = vname->components[i];

		while ((buf[size] = (char)tolower((int)*p++)) != '\0') {
			if (size > (int)(sizeof(buf) - 3))
				return DE_PATH;
			size += 1;
		}

		buf[size++] = '/', buf[size] = '\0';
	}

	if (buf[0] == '\0')
		return 0;

	buf[size - 1] = '\0';

	lock_fat(node);

	if (find_node(node, &buf[0]) != NULL)
		return unlock_fat(node), DE_BUSY;

	if (vname->type == vfs_type_directory)
		buf[size - 1] = '/';

	r = fat_remove(instance, &buf[0]);

	unlock_fat(node);

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
		node->n_readdir  = n_readdir;
		node->n_rename   = n_rename;
		node->n_stat     = n_stat;
		node->n_truncate = n_truncate;
		node->n_unlink   = n_unlink;

		data = node->internal_data;
		data->io = io;
		data->fd = -1;
	}

	return node;
}

static int fat_io_add(struct fat_io *io)
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
		vfs_decrement_count(dev_node);
		return free(io), r;
	}

	return *new_node = io->root_node, 0;
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

	/*
	 * Sector sizes 512, 1024, 2048, and 4096 are supported. The block
	 * size can not be bigger than the file system sector size, so the
	 * smallest size is used here.
	 */
	*block_size = 512;
	*block_total = (size_t)(stat.size / 512);

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
	uint64_t offset;
	int r;

	check_id(id);
	io = fat_io_array[id];

	if (io->media_changed)
		return 1;

	node = io->dev_node;

	offset = (uint64_t)(lba * 512);
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
	uint64_t offset;
	int r;

	check_id(id);
	io = fat_io_array[id];

	if (io->media_changed)
		return 1;

	node = io->dev_node;

	offset = (uint64_t)(lba * 512);
	r = node->n_write(node, offset, size, buf);

	if (r == DE_MEDIA_CHANGED)
		io->media_changed = 1;

	if (*size != requested_size)
		return 1;

	return (r != 0);
}
