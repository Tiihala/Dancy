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
static const uint64_t fd_cloexec = (uint64_t)(0x0001000000000000ull);

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
	int count;

	spin_enter(&lock_local);

	if (fte->count > 0)
		fte->count -= 1;

	if (fte->count == 0) {
		fte->flags = 0;
		fte->offset = 0;

		node = fte->node;
		fte->node = NULL;
	}

	count = fte->count;

	spin_leave(&lock_local);

	if (node)
		node->n_release(&node);
	if (!count)
		spin_unlock(&fte->lock[0]);
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
	int accmode = flags & O_ACCMODE;
	int vtype = vfs_type_unknown;
	int vmode = 0;
	int r;

	(void)mode;
	*fd = -1;

	if (accmode != O_RDONLY && accmode != O_WRONLY && accmode != O_RDWR)
		return DE_ARGUMENT;

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

	if ((flags & O_CLOEXEC) != 0)
		task->fd.table[*fd] |= fd_cloexec;

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
	size_t requested_size = *size;
	int r;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		struct vfs_node *n;
		uint64_t t, o;

		if ((t = task->fd.table[fd]) != 0) {
			int block_capability = 0;

			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			if ((fte->flags & O_ACCMODE) == O_WRONLY) {
				spin_unlock(&fte->lock[1]);
				return *size = 0, DE_ARGUMENT;
			}

			n = fte->node;
			o = fte->offset;
			r = n->n_read(n, o, size, buffer);

			if (n->type == vfs_type_buffer)
				block_capability = 1;
			else if (n->type == vfs_type_character)
				block_capability = 1;

			while (block_capability) {
				if (*size != 0 || r != DE_RETRY)
					break;

				if ((fte->flags & O_NONBLOCK) != 0)
					break;

				if (n->internal_event)
					event_wait(*n->internal_event, 2000);

				*size = requested_size;
				r = n->n_read(n, o, size, buffer);
			}

			fte->offset += (uint64_t)(*size);
			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return *size = 0, DE_ARGUMENT;
}

int file_write(int fd, size_t *size, const void *buffer)
{
	struct task *task = task_current();
	const unsigned char *ptr = buffer;
	size_t requested_size = *size;
	int r;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		struct vfs_node *n;
		uint64_t t, o;

		if ((t = task->fd.table[fd]) != 0) {
			int block_capability = 0;

			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			if ((fte->flags & O_ACCMODE) == O_RDONLY) {
				spin_unlock(&fte->lock[1]);
				return *size = 0, DE_ARGUMENT;
			}

			n = fte->node;
			o = fte->offset;

			if ((fte->flags & O_APPEND) != 0) {
				r = n->n_append(n, size, buffer);
				fte->offset = get_file_size(n);
			} else {
				r = n->n_write(n, o, size, buffer);
				fte->offset += (uint64_t)(*size);
			}

			if (n->type == vfs_type_buffer)
				block_capability = 1;
			else if (n->type == vfs_type_character)
				block_capability = 1;

			while (block_capability) {
				if (*size == requested_size || r != DE_RETRY)
					break;

				if ((fte->flags & O_NONBLOCK) != 0)
					break;

				if (*size > requested_size) {
					r = DE_UNEXPECTED;
					break;
				}

				if (*size == 0)
					task_yield();

				requested_size -= (*size);
				ptr += (*size);

				*size = requested_size;

				if ((fte->flags & O_APPEND) != 0) {
					r = n->n_append(n, size, ptr);
					fte->offset = get_file_size(n);
				} else {
					r = n->n_write(n, o, size, ptr);
					fte->offset += (uint64_t)(*size);
				}
			}

			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return *size = 0, DE_ARGUMENT;
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

int file_fcntl(int fd, int cmd, int arg, int *retval)
{
	struct task *task = task_current();
	int r = DE_ARGUMENT;

	*retval = -1;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		uint64_t t;

		if ((t = task->fd.table[fd]) != 0) {
			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			if (cmd == F_DUPFD || cmd == F_DUPFD_CLOEXEC) {
				int flags = 0;

				if (cmd == F_DUPFD_CLOEXEC)
					flags = O_CLOEXEC;

				spin_unlock(&fte->lock[1]);
				r = file_dup(fd, retval, arg, INT_MAX, flags);
				return r;

			} else if (cmd == F_GETFD) {
				if ((t & fd_cloexec) != 0)
					*retval = FD_CLOEXEC;
				r = 0;

			} else if (cmd == F_SETFD) {
				if ((arg & FD_CLOEXEC) != 0)
					task->fd.table[fd] |= fd_cloexec;
				r = 0;

			} else if (cmd == F_GETFL) {
				*retval = fte->flags;
				r = 0;

			} else if (cmd == F_SETFL) {
				unsigned int flags = (unsigned int)fte->flags;

				flags &= (~((unsigned int)O_APPEND));
				flags |= (unsigned int)(arg & O_APPEND);

				flags &= (~((unsigned int)O_NONBLOCK));
				flags |= (unsigned int)(arg & O_NONBLOCK);

				fte->flags = (int)flags;
				r = 0;
			}

			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return DE_ARGUMENT;
}

int file_dup(int fd, int *new_fd, int min_fd, int max_fd, int flags)
{
	struct task *task = task_current();
	int r = DE_OVERFLOW;
	int i;

	*new_fd = -1;

	if (min_fd < 0 || min_fd > max_fd)
		return DE_ARGUMENT;

	if (fd == min_fd && min_fd == max_fd)
		return DE_ARGUMENT;

	if (fd >= 0 && fd < (int)task->fd.state) {
		struct file_table_entry *fte;
		uint64_t t;

		if ((t = task->fd.table[fd]) != 0) {
			int empty_fd = -1;

			fte = (void *)((addr_t)(t & table_mask));

			while (!spin_trylock(&fte->lock[1]))
				task_yield();

			for (i = min_fd; i < max_fd; i++) {
				if (i >= TASK_FD_STATIC_COUNT)
					break;
				if (task->fd.table[i] == 0) {
					empty_fd = i;
					break;
				}
			}

			if (empty_fd < 0 && min_fd == max_fd) {
				if (min_fd < TASK_FD_STATIC_COUNT)
					file_close(empty_fd = min_fd);
			}

			if (empty_fd >= 0 && fte->count < INT_MAX) {
				uint32_t state = (uint32_t)(empty_fd + 1);

				if (task->fd.state < state)
					task->fd.state = state;

				if (task->fd.table[empty_fd] != 0) {
					spin_unlock(&fte->lock[1]);
					return DE_UNEXPECTED;
				}

				*new_fd = empty_fd;
				task->fd.table[*new_fd] = t & table_mask;

				if ((flags & O_CLOEXEC) != 0)
					task->fd.table[*new_fd] |= fd_cloexec;

				fte->count += 1;
				r = 0;
			}

			spin_unlock(&fte->lock[1]);

			return r;
		}
	}

	return DE_ARGUMENT;
}
