/*
 * Copyright (c) 2025 Antti Tiihala
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
 * misc/mouse.c
 *      Dancy mouse devices
 */

#include <dancy.h>

struct mse_device {
	struct vfs_node *pipe[2];
	struct vfs_node node;
	int state;
	const char *name;
};

static struct mse_device mse_devices[] = {
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-1" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-2" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-3" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-4" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-5" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-6" },
	{ { NULL, NULL }, { 0 }, 0, "/dev/dancy-mouse-gui" }
};

#define MSE_DEVICE_COUNT ((int)(sizeof(mse_devices) / sizeof(mse_devices[0])))

static struct vfs_node **get_pipe_nodes(struct vfs_node *node)
{
	uint32_t i = 0;

	if (node == NULL) {
		i = (kernel->keyboard.console_switch_data & 0xFF) - 1;

	} else {
		while (i < MSE_DEVICE_COUNT) {
			if (&mse_devices[i].node == node)
				break;
			i += 1;
		}
	}

	if (i >= (uint32_t)MSE_DEVICE_COUNT)
		i = 0;

	if (mse_devices[i].state == 0)
		return kernel->panic("Mouse: unexpected node state"), NULL;

	return &mse_devices[i].pipe[0];
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct vfs_node *pn = get_pipe_nodes(node)[0];

	(void)offset;

	if ((*size % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	if (((size_t)((addr_t)buffer) % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	return pn->n_read(pn, 0, size, buffer);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;

	if ((*size % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	if (((size_t)((addr_t)buffer) % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	return *size = 0, DE_FULL;
}

static int n_poll(struct vfs_node *node, int events, int *revents)
{
	struct vfs_node *pn = get_pipe_nodes(node)[0];
	int r;

	*revents = 0;

	if ((r = pn->n_poll(pn, events, revents)) == 0) {
		if ((events & POLLOUT) != 0)
			*revents |= POLLOUT;
		if ((events & POLLWRNORM) != 0)
			*revents |= POLLWRNORM;
	}

	return r;
}

int dancy_mse_init(void)
{
	static int run_once;

	int i, r = DE_UNEXPECTED;

	if (!spin_trylock(&run_once))
		return r;

	for (i = 0; i < MSE_DEVICE_COUNT; i++)  {
		const char *name = mse_devices[i].name;
		struct vfs_node *node;

		if (vfs_pipe(mse_devices[i].pipe) != 0)
			return kernel->panic("Mouse: vfs_pipe error"), r;

		if (vfs_open(name, &node, 0, vfs_mode_create) != 0)
			return kernel->panic("Mouse: vfs_open error"), r;

		node->n_release(&node);
		node = &mse_devices[i].node;

		vfs_init_node(node, 0);
		node->type = vfs_type_character;
		node->mode = vfs_mode_exclusive;
		node->internal_event = mse_devices[i].pipe[0]->internal_event;
		node->n_read = n_read;
		node->n_write = n_write;
		node->n_poll = n_poll;

		if (vfs_mount(name, node) != 0)
			return kernel->panic("Mouse: vfs_mount error"), r;

		spin_trylock(&mse_devices[i].state);
	}

	return 0;
}

void dancy_mse_clear(void)
{
	int i;

	for (i = 0; i < MSE_DEVICE_COUNT; i++) {
		while (mse_devices[i].node.count == 1) {
			size_t size = sizeof(int);
			struct vfs_node *pn = mse_devices[i].pipe[0];
			int read_data;

			(void)pn->n_read(pn, 0, &size, &read_data);

			if (size != sizeof(int))
				break;
		}
	}
}

int dancy_mse_write(size_t *size, void *buffer)
{
	struct vfs_node *pn = get_pipe_nodes(NULL)[1];

	return pn->n_write(pn, 0, size, buffer);
}
