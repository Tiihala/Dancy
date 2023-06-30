/*
 * Copyright (c) 2021, 2023 Antti Tiihala
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
 * ps2/mouse.c
 *      PS/2 Mouse
 */

#include <dancy.h>

static int probe_state = 0;

static int mse_ready = 0;
static int mse_mode = 0;

static const int data_none = -1;

static struct vfs_node *mse_pipe_nodes[2];
static struct vfs_node mse_node;

static int send_command(int command, int data, int count, int *response)
{
	uint32_t ticks_timeout = (uint32_t)((command == 0xFF) ? 2000 : 100);
	uint32_t ticks;
	int resend = 0;
	int b, i;

	for (i = 0; i < count; i++)
		response[i] = 0;

	ps2_send_port2((uint8_t)command);

	ticks = timer_ticks;

	while ((b = ps2_receive_port2()) != 0xFA) {
		if (b == 0xFE && resend < 32) {
			ps2_send_port2((uint8_t)command);
			resend += 1;
			continue;
		}
		if ((timer_ticks - ticks) >= ticks_timeout)
			return 1;
	}

	if (data != data_none) {
		ps2_send_port2((uint8_t)data);
		ticks = timer_ticks;

		while ((b = ps2_receive_port2()) != 0xFA) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 2;
		}
	}

	for (i = 0; i < count; i++) {
		ticks = timer_ticks;

		while ((b = ps2_receive_port2()) < 0) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 3;
		}
		response[i] = b;
	}

	while (ps2_receive_port2() >= 0) { /* void */ }

	return 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct vfs_node *pn = mse_pipe_nodes[0];

	(void)node;
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
	struct vfs_node *pn = mse_pipe_nodes[0];
	int r;

	(void)node;
	*revents = 0;

	if ((r = pn->n_poll(pn, events, revents)) == 0) {
		if ((events & POLLOUT) != 0)
			*revents |= POLLOUT;
		if ((events & POLLWRNORM) != 0)
			*revents |= POLLWRNORM;
	}

	return r;
}

int ps2_mse_init(void)
{
	int response[2];

	mse_ready = 0;
	mse_mode = 0;

	/*
	 * Try first with a faster command before doing the
	 * full initialization sequence.
	 */
	if (send_command(0xE6, data_none, 0, NULL))
		return 1;

	/*
	 * Reset the device.
	 */
	(void)send_command(0xFF, data_none, 2, &response[0]);

	/*
	 * Try to enable the "scrolling wheel" mode.
	 */
	{
		(void)send_command(0xF3, 200, 0, NULL);
		(void)send_command(0xF3, 100, 0, NULL);
		(void)send_command(0xF3, 80, 0, NULL);

		if (send_command(0xF2, data_none, 1, &response[0]))
			return 1;

		mse_mode = response[0];
	}

	/*
	 * Set the resolution.
	 */
	if (send_command(0xE8, 3, 0, NULL))
		return 1;

	/*
	 * Set the scale.
	 */
	if (send_command(0xE6, data_none, 0, NULL))
		return 1;

	/*
	 * Set the sample rate.
	 */
	if (send_command(0xF3, 40, 0, NULL))
		return 1;

	/*
	 * Enable data reporting.
	 */
	(void)send_command(0xF4, data_none, 0, NULL);

	/*
	 * Create the device node.
	 */
	{
		const char *name = "/dev/dancy-mouse";
		struct vfs_node *node;

		if (vfs_pipe(mse_pipe_nodes) != 0)
			return 1;

		if (vfs_open(name, &node, 0, vfs_mode_create) != 0)
			return 1;

		node->n_release(&node);

		vfs_init_node(&mse_node, 0);
		mse_node.type = vfs_type_character;
		mse_node.internal_event = mse_pipe_nodes[0]->internal_event;
		mse_node.n_read = n_read;
		mse_node.n_write = n_write;
		mse_node.n_poll = n_poll;

		if (vfs_mount(name, &mse_node) != 0)
			return 1;
	}

	mse_ready = 1;

	return 0;
}

void ps2_mse_handler(void)
{
	static int data = 0;
	static int state = 0;
	int b;

	while (mse_ready == 0) {
		if (ps2_receive_port2() < 0)
			return;
	}

	while (mse_node.count == 1) {
		size_t size = sizeof(int);
		struct vfs_node *pn = mse_pipe_nodes[0];
		int read_data;

		(void)pn->n_read(pn, 0, &size, &read_data);

		if (size != sizeof(int))
			break;
	}

	while ((b = ps2_receive_port2()) >= 0) {
		switch (state) {
		case 0:
			if ((b & (1 << 3)) != 0) {
				data = b;
				state = 1;
			}
			break;
		case 1:
			data |= (b << 8);
			state = 2;
			break;
		case 2:
			data |= (b << 16);
			state = (mse_mode == 3) ? 3 : 4;
			break;
		case 3:
			data |= ((b & 0x0F) << 24);
			state = 4;
			break;
		default:
			state = 0;
			break;
		}

		while (state == 4) {
			size_t size = sizeof(int);
			struct vfs_node *pn = mse_pipe_nodes[1];
			int r = pn->n_write(pn, 0, &size, &data);

			if (size == sizeof(int) || r != DE_RETRY)
				state = 0;
			else
				task_yield();
		}
	}

	while (mse_node.count == 1) {
		size_t size = sizeof(int);
		struct vfs_node *pn = mse_pipe_nodes[0];
		int read_data;

		(void)pn->n_read(pn, 0, &size, &read_data);

		if (size != sizeof(int))
			break;
	}
}

void ps2_mse_probe(void)
{
	int response;

	if (mse_ready == 0) {
		ps2_mse_init();
		return;
	}

	if (probe_state < 2) {
		probe_state += 1;
		return;
	}

	mse_ready = 0;

	/*
	 * Compare the device ID with the previous value.
	 */
	if (!send_command(0xF2, data_none, 1, &response)) {
		if (mse_mode == response)
			mse_ready = 1;
	}

	probe_state = 0;
}
