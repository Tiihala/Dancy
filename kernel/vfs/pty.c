/*
 * Copyright (c) 2023 Antti Tiihala
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
 * vfs/pty.c
 *      Pseudoterminals
 */

#include <dancy.h>

#define PTY_BUFFER_SIZE 4096

struct pty_shared_data {
	int lock;
	int count;
	int pty_i;

	struct {
		event_t event;
		int start;
		int end;
		unsigned char base[PTY_BUFFER_SIZE];
	} buffer[2];
};

enum pty_type {
	pty_type_main,
	pty_type_secondary
};

struct pty_internal_data {
	struct pty_shared_data *shared_data;
	int type;
};

#define PTY_ARRAY_COUNT 64
static int pty_array_lock;
static struct vfs_node *pty_array[PTY_ARRAY_COUNT];

static int n_open_pts(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	void *lock_local = &pty_array_lock;
	int pty_i = 0;
	int r = DE_NAME;
	size_t i;

	(void)node;
	(void)type;

	*new_node = NULL;

	if (name[0] == '0') {
		if (name[1] != '\0')
			return DE_NAME;
	} else {
		for (i = 0; name[i] != '\0'; i++) {
			int c = (int)name[i];

			if (c < '0' || c > '9')
				return DE_NAME;

			pty_i *= 10;
			pty_i += (c - '0');
			r = 0;

			if (pty_i >= PTY_ARRAY_COUNT)
				return DE_NAME;
		}

		if (r != 0)
			return r;
	}

	spin_enter(&lock_local);

	if (pty_array[pty_i] == NULL)
		return spin_leave(&lock_local), DE_NAME;

	if ((mode & vfs_mode_exclusive) != 0)
		return spin_leave(&lock_local), DE_BUSY;

	vfs_increment_count(pty_array[pty_i]);
	*new_node = pty_array[pty_i];

	spin_leave(&lock_local);

	return 0;
}

static int n_readdir_pts(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	void *lock_local = &pty_array_lock;
	int r = 0;

	(void)node;
	memset(dent, 0, sizeof(*dent));

	if (offset == 0) {
		strcpy(&dent->name[0], ".");
		return 0;
	}

	if (offset == 1) {
		strcpy(&dent->name[0], "..");
		return 0;
	}

	if (offset - 2 >= PTY_ARRAY_COUNT)
		return 0;

	spin_enter(&lock_local);

	if (pty_array[offset - 2] == NULL)
		r = DE_PLACEHOLDER;

	spin_leave(&lock_local);

	if (r == 0)
		snprintf(&dent->name[0], 8, "%d", (int)(offset - 2));

	return r;
}

int pty_init(void)
{
	static int run_once;
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = vfs_open("/dev/pts/", &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	if ((node = malloc(sizeof(*node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(node, 0);

	node->count = 1;
	node->type = vfs_type_directory;
	node->n_open = n_open_pts;
	node->n_readdir = n_readdir_pts;

	r = vfs_mount("/dev/pts", node);
	node->n_release(&node);

	return r;
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct pty_internal_data *internal_data = n->internal_data;
	struct pty_shared_data *shared_data = internal_data->shared_data;

	*node = NULL;

	if (vfs_decrement_count(n) == 0) {
		void *lock_local_1 = &pty_array_lock;
		void *lock_local_2 = &shared_data->lock;
		int count;

		if (internal_data->type == pty_type_secondary) {
			spin_enter(&lock_local_1);
			pty_array[shared_data->pty_i] = NULL;
			spin_leave(&lock_local_1);
		}

		memset(n, 0, sizeof(*n));
		free(n);

		spin_enter(&lock_local_2);
		count = (shared_data->count -= 1);
		spin_leave(&lock_local_2);

		if (count <= 0) {
			event_delete(shared_data->buffer[0].event);
			event_delete(shared_data->buffer[1].event);
			memset(shared_data, 0, sizeof(*shared_data));
			free(shared_data);
		}
	}
}

static int n_read_main(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int n_write_main(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int n_read_secondary(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int n_write_secondary(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int n_poll(struct vfs_node *node, int events, int *revents)
{
	struct pty_internal_data *internal_data = node->internal_data;
	struct pty_shared_data *shared_data = internal_data->shared_data;
	void *lock_local = &shared_data->lock;

	int read_ok, write_ok, r = 0;
	int start, end;

	spin_enter(&lock_local);

	if (internal_data->type == pty_type_main) {
		start = shared_data->buffer[0].start;
		end = shared_data->buffer[0].end;
	} else {
		start = shared_data->buffer[1].start;
		end = shared_data->buffer[1].end;
	}

	spin_leave(&lock_local);

	read_ok = (start != end);

	{
		int count = end - start;

		if (count < 0)
			count = (PTY_BUFFER_SIZE + end) - start;

		write_ok = (count < (PTY_BUFFER_SIZE / 2));
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

static int n_ioctl(struct vfs_node *node,
	int request, long long arg)
{
	(void)node;
	(void)request;
	(void)arg;

	return DE_UNSUPPORTED;
}

static struct vfs_node *alloc_node(int type,
	struct pty_shared_data *shared_data)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += sizeof(struct pty_internal_data);

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;
		struct pty_internal_data *data;

		vfs_init_node(node, size);

		node->count = 1;
		node->type = vfs_type_buffer;

		node->internal_data = (void *)a;

		if (type == pty_type_main) {
			node->internal_event = &shared_data->buffer[0].event;
			node->n_read  = n_read_main;
			node->n_write = n_write_main;
		} else {
			node->internal_event = &shared_data->buffer[1].event;
			node->n_read  = n_read_secondary;
			node->n_write = n_write_secondary;
		}

		node->n_release = n_release;
		node->n_poll    = n_poll;
		node->n_ioctl   = n_ioctl;

		data = node->internal_data;

		data->shared_data = shared_data;
		data->type = type;
	}

	return node;
}

int pty_create(struct vfs_node *nodes[2])
{
	struct pty_shared_data *shared_data;
	struct vfs_node *main_node, *secondary_node;
	int i, r = 0;

	nodes[0] = NULL, nodes[1] = NULL;

	if ((shared_data = malloc(sizeof(*shared_data))) == NULL)
		return DE_MEMORY;

	memset(shared_data, 0, sizeof(*shared_data));

	shared_data->buffer[0].event = event_create(event_type_manual_reset);
	shared_data->buffer[1].event = event_create(event_type_manual_reset);
	shared_data->count = 2;
	shared_data->pty_i = -1;

	main_node = alloc_node(pty_type_main, shared_data);
	secondary_node = alloc_node(pty_type_secondary, shared_data);

	r |= (!shared_data->buffer[0].event ? DE_MEMORY : 0);
	r |= (!shared_data->buffer[1].event ? DE_MEMORY : 0);
	r |= (!main_node ? DE_MEMORY : 0);
	r |= (!secondary_node ? DE_MEMORY : 0);

	if (r == 0) {
		void *lock_local = &pty_array_lock;

		spin_enter(&lock_local);

		for (i = 0; i < PTY_ARRAY_COUNT; i++) {
			if (pty_array[i] == NULL) {
				shared_data->pty_i = i;
				pty_array[i] = secondary_node;
				break;
			}
		}

		spin_leave(&lock_local);

		if (shared_data->pty_i < 0)
			r = DE_FULL;
	}

	if (r != 0) {
		free(main_node), free(secondary_node);
		event_delete(shared_data->buffer[0].event);
		event_delete(shared_data->buffer[1].event);
		free(shared_data);
		return r;
	}

	nodes[0] = main_node;
	nodes[1] = secondary_node;

	return 0;
}
