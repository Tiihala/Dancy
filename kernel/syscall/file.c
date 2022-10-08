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
 * syscall/file.c
 *      File descriptors and tables
 */

#include <dancy.h>

int file_table_count;
struct file_table_entry *file_table;

static struct file_table_entry _file_table[4096];
static const uint64_t table_mask = (uint64_t)(0x0000FFFFFFFFFFFFull);

static void fd_release_func(struct task *task)
{
	int i;

	for (i = 0; i < (int)task->fd.state; i++) {
		if (task->fd.table[i] != 0)
			file_close(i);
	}
}

static int alloc_file_descriptor(struct task *task)
{
	int i;

	if (task->fd.state == 0) {
		task->fd.table = &task->fd._table[0];
		task->fd.release = fd_release_func;
	}

	for (i = 0; i < (int)task->fd.state; i++) {
		if (task->fd.table[i] == 0)
			return i;
	}

	if (task->fd.state < TASK_FD_STATIC_COUNT)
		return (int)(task->fd.state++);

	return -1;
}

static struct file_table_entry *alloc_file_entry(void)
{
	int i;

	for (i = 0; i < file_table_count; i++) {
		if (file_table[i].lock[0])
			continue;
		if (spin_trylock(&file_table[i].lock[0]))
			return &file_table[i];
	}

	for (i = 0; i < file_table_count; i++) {
		if (spin_trylock(&file_table[i].lock[0]))
			return &file_table[i];
	}

	return NULL;
}

static void file_decrement_count(struct file_table_entry *fte)
{
	void *lock_local = &fte->lock[1];
	struct vfs_node *node = NULL;

	spin_enter(&lock_local);

	if (fte->count > 0)
		fte->count -= 1;

	if (fte->count == 0) {
		fte->flags = 0;
		fte->offset = 0;

		node = fte->node;
		fte->node = NULL;
	}

	spin_leave(&lock_local);

	if (node) {
		node->n_release(&node);
		spin_unlock(&fte->lock[0]);
	}
}

static uint64_t get_file_size(struct vfs_node *node)
{
	struct vfs_stat stat;

	if (!node->n_stat(node, &stat))
		return stat.size;

	return 0;
}

int file_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	file_table_count = (int)(sizeof(_file_table) / sizeof(*_file_table));
	file_table = &_file_table[0];

	return 0;
}

int file_open(int *fd, const char *name, int flags, mode_t mode)
{

	struct task *task = task_current();
	struct file_table_entry *fte;
	int vtype = vfs_type_unknown;
	int vmode = 0;
	int r;

	(void)mode;
	*fd = -1;

	if ((fte = alloc_file_entry()) == NULL)
		return DE_MEMORY;

	if (!spin_trylock(&fte->lock[1]) || fte->node)
		return DE_UNEXPECTED;

	fte->count = 1;
	fte->flags = flags;
	fte->offset = 0;

	if ((flags & O_DIRECTORY) != 0)
		vtype = vfs_type_directory;

	if ((flags & O_CREAT) != 0)
		vmode |= vfs_mode_create;
	if ((flags & O_TRUNC) != 0)
		vmode |= vfs_mode_truncate;
	if ((flags & O_EXCL) != 0)
		vmode |= vfs_mode_exclusive;

	r = vfs_open(name, &fte->node, vtype, vmode);
	spin_unlock(&fte->lock[1]);

	if (r)
		return file_decrement_count(fte), r;

	if ((*fd = alloc_file_descriptor(task)) < 0)
		return file_decrement_count(fte), DE_OVERFLOW;

	task->fd.table[*fd] = (uint64_t)((addr_t)fte);

	return 0;
}

int file_close(int fd)
{
	struct task *task = task_current();

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		uint64_t t;

		if ((t = task->fd.table[fd]) != 0) {
			fte = (void *)((addr_t)(t & table_mask));
			file_decrement_count(fte);
			task->fd.table[fd] = 0;
			return 0;
		}
	}

	return DE_ARGUMENT;
}

int file_read(int fd, size_t *size, void *buffer)
{
	struct task *task = task_current();
	int r;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		struct vfs_node *n;
		uint64_t t, o;

		if ((t = task->fd.table[fd]) != 0) {
			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			n = fte->node;
			o = fte->offset;
			r = n->n_read(n, o, size, buffer);

			fte->offset += (uint64_t)(*size);
			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return DE_ARGUMENT;
}

int file_write(int fd, size_t *size, const void *buffer)
{
	struct task *task = task_current();
	int r;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		struct vfs_node *n;
		uint64_t t, o;

		if ((t = task->fd.table[fd]) != 0) {
			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			n = fte->node;
			o = fte->offset;

			if ((fte->flags & O_APPEND) != 0)
				r = n->n_append(n, size, buffer);
			else
				r = n->n_write(n, o, size, buffer);

			fte->offset += (uint64_t)(*size);
			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return DE_ARGUMENT;
}

int file_lseek(int fd, off_t offset, int whence)
{
	struct task *task = task_current();
	int r = DE_ARGUMENT;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		uint64_t t;

		if ((t = task->fd.table[fd]) != 0) {
			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			if (whence == SEEK_SET && offset >= 0) {
				fte->offset = (uint64_t)offset;
				r = 0;

			} else if (whence == SEEK_CUR && offset < 0) {
				uint64_t o = fte->offset;
				uint64_t s = (uint64_t)(-offset);

				if (o >= s) {
					fte->offset = o - s;
					r = 0;
				}

			} else if (whence == SEEK_END && offset < 0) {
				uint64_t o = get_file_size(fte->node);
				uint64_t s = (uint64_t)(-offset);

				if (o >= s) {
					fte->offset = o - s;
					r = 0;
				}

			} else if (whence == SEEK_CUR && offset >= 0) {
				uint64_t o = fte->offset;
				uint64_t a = (uint64_t)offset;

				if (o <= (o + a)) {
					fte->offset = o + a;
					r = 0;
				}

			} else if (whence == SEEK_END && offset >= 0) {
				uint64_t o = get_file_size(fte->node);
				uint64_t a = (uint64_t)offset;

				if (o <= (o + a)) {
					fte->offset = o + a;
					r = 0;
				}
			}

			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return DE_ARGUMENT;
}
