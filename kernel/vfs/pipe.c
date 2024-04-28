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
 * vfs/pipe.c
 *      Data channel
 */

#include <dancy.h>

#define PIPE_SIZE 4096

struct pipe_shared_data {
	event_t event;
	int lock;
	int count;
	int start;
	int end;
	unsigned char base[PIPE_SIZE];
};

enum pipe_type {
	pipe_type_read,
	pipe_type_write
};

struct pipe_internal_data {
	struct pipe_shared_data *shared_data;
	int type;
};

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct pipe_internal_data *internal_data = n->internal_data;
	struct pipe_shared_data *shared_data = internal_data->shared_data;
	void *lock_local = &shared_data->lock;
	int count;

	*node = NULL;

	spin_enter(&lock_local);
	event_signal(shared_data->event);
	spin_leave(&lock_local);

	if (vfs_decrement_count(n) == 0) {
		memset(n, 0, sizeof(*n));
		free(n);

		spin_enter(&lock_local);
		count = (shared_data->count -= 1);
		spin_leave(&lock_local);

		if (count <= 0) {
			event_delete(shared_data->event);
			memset(shared_data, 0, sizeof(*shared_data));
			free(shared_data);
		}
	}
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	size_t requested_size = *size;
	struct pipe_internal_data *internal_data = node->internal_data;
	struct pipe_shared_data *shared_data = internal_data->shared_data;
	unsigned char *ptr = buffer;
	int b = 0;

	(void)offset;
	*size = 0;

	if (internal_data->type != pipe_type_read)
		return DE_ACCESS;

	do {
		void *lock_local = &shared_data->lock;
		int atomicity = 0;

		spin_enter(&lock_local);

		while (*size < requested_size) {
			int start = shared_data->start;

			if (start != shared_data->end) {
				b = (int)shared_data->base[start];
				shared_data->base[start] = 0;
				shared_data->start = (start + 1) % PIPE_SIZE;
			} else {
				b = -1;
				if (shared_data->count == 2)
					event_reset(shared_data->event);
			}

			if (b < 0)
				break;

			*ptr++ = (unsigned char)b;
			*size += 1;

			if ((++atomicity) == 512)
				break;
		}

		if (shared_data->count != 2) {
			event_signal(shared_data->event);

			if (*size == 0) {
				spin_leave(&lock_local);
				return DE_EMPTY;
			}
		}

		spin_leave(&lock_local);

	} while (b >= 0 && *size < requested_size);

	return 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	size_t requested_size = *size;
	struct pipe_internal_data *internal_data = node->internal_data;
	struct pipe_shared_data *shared_data = internal_data->shared_data;
	const unsigned char *ptr = buffer;

	(void)offset;
	*size = 0;

	if (internal_data->type != pipe_type_write)
		return DE_ACCESS;

	do {
		void *lock_local = &shared_data->lock;
		int atomicity = 0;
		int original_end;

		spin_enter(&lock_local);

		if (shared_data->count != 2) {
			spin_leave(&lock_local);
			return DE_UNINITIALIZED;
		}

		original_end = shared_data->end;

		while (*size < requested_size) {
			int start = shared_data->start;
			int old_end = shared_data->end;
			int new_end = (old_end + 1) % PIPE_SIZE;

			if (start == new_end)
				break;

			shared_data->base[old_end] = *ptr++;
			shared_data->end = new_end;
			*size += 1;

			if ((++atomicity) == 512)
				break;
		}

		if (requested_size <= 512) {
			if (requested_size != (size_t)atomicity) {
				shared_data->end = original_end;
				spin_leave(&lock_local);
				return *size = 0, DE_RETRY;
			}
		}

		if (atomicity)
			event_signal(shared_data->event);

		spin_leave(&lock_local);

		if (!atomicity)
			break;

	} while (*size < requested_size);

	return 0;
}

static int n_poll(struct vfs_node *node, int events, int *revents)
{
	struct pipe_internal_data *internal_data = node->internal_data;
	struct pipe_shared_data *shared_data = internal_data->shared_data;
	void *lock_local = &shared_data->lock;

	int read_ok = 0, write_ok = 0, r = 0;
	int start, end;

	spin_enter(&lock_local);

	start = shared_data->start;
	end = shared_data->end;

	spin_leave(&lock_local);

	if (internal_data->type == pipe_type_read)
		read_ok = (start != end);

	if (internal_data->type == pipe_type_write) {
		int count = end - start;

		if (count < 0)
			count = (PIPE_SIZE + end) - start;

		write_ok = (count < (PIPE_SIZE / 2));
	}

	if (read_ok && (events & POLLIN) != 0)
		r |= POLLIN;

	if (read_ok && (events & POLLRDNORM) != 0)
		r |= POLLRDNORM;

	if (write_ok && (events & POLLOUT) != 0)
		r |= POLLOUT;

	if (write_ok && (events & POLLWRNORM) != 0)
		r |= POLLWRNORM;

	*revents = r;

	return 0;
}

static struct vfs_node *alloc_node(int type,
	struct pipe_shared_data *shared_data)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += sizeof(struct pipe_internal_data);

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;
		struct pipe_internal_data *data;

		vfs_init_node(node, size);

		node->count = 1;
		node->type = vfs_type_buffer;

		node->internal_data = (void *)a;
		node->internal_event = &shared_data->event;

		node->n_release = n_release;
		node->n_read    = n_read;
		node->n_write   = n_write;
		node->n_poll    = n_poll;

		data = node->internal_data;

		data->shared_data = shared_data;
		data->type = type;
	}

	return node;
}

int vfs_pipe(struct vfs_node *nodes[2])
{
	struct pipe_shared_data *shared_data;
	struct vfs_node *read_node, *write_node;

	nodes[0] = NULL, nodes[1] = NULL;

	if ((shared_data = malloc(sizeof(*shared_data))) == NULL)
		return DE_MEMORY;

	memset(shared_data, 0, sizeof(*shared_data));

	shared_data->event = event_create(event_type_manual_reset);
	shared_data->count = 2;

	read_node = alloc_node(pipe_type_read, shared_data);
	write_node = alloc_node(pipe_type_write, shared_data);

	if (!shared_data->event || !read_node || !write_node) {
		free(write_node), free(read_node);
		event_delete(shared_data->event);
		free(shared_data);
		return DE_MEMORY;
	}

	nodes[0] = read_node;
	nodes[1] = write_node;

	return 0;
}
