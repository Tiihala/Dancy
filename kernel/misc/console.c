/*
 * Copyright (c) 2022, 2024 Antti Tiihala
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
 * misc/console.c
 *      Device /dev/console
 */

#include <dancy.h>

static struct vfs_node console_node;

static int console_task(void *arg)
{
	task_set_cmdline(task_current(), NULL, "[console]");

	while (arg == NULL) {
		const uint16_t ms = 0xFFFF;
		uint32_t data;

		if (event_wait(kernel->keyboard.console_switch_event, ms) < 0)
			continue;

		data = cpu_read32(&kernel->keyboard.console_switch_data);

		if (data <= 0xFF)
			con_switch((int)data);
	}

	return 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	return *size = 0, 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;

	if (kernel->rebooting)
		return 0;

	return con_write(buffer, *size), 0;
}

static int n_ioctl(struct vfs_node *node,
	int request, long long arg)
{
	const unsigned int count = 2;

	(void)node;

	if (request == __DANCY_IOCTL_VT_GETSTATE) {
		struct __dancy_vt_stat *d = (void *)((addr_t)arg);
		size_t size = sizeof(*d);

		unsigned int v_active = kernel->keyboard.console_switch_data;
		unsigned int v_state = (((1U << count) - 1) << 1) + 1;

		if (v_active < 1 || v_active > count)
			v_active = 1;

		memset(d, 0, size);
		d->v_active = (unsigned short)v_active;
		d->v_state = (unsigned short)v_state;

		return 0;
	}

	if (request == __DANCY_IOCTL_VT_ACTIVATE) {
		uint32_t f;

		if (arg < 1 || arg > (long long)count)
			return DE_UNSUPPORTED;

		f = (uint32_t)arg;
		cpu_write32(&kernel->keyboard.console_switch_data, f);
		event_signal(kernel->keyboard.console_switch_event);

		return 0;
	}

	if (request == __DANCY_IOCTL_VT_WAITACTIVE) {
		uint32_t f;

		if (arg < 1 || arg > (long long)count)
			return DE_UNSUPPORTED;

		f = (uint32_t)arg;

		while (kernel->keyboard.console_switch_data != f) {
			if (task_signaled(task_current()))
				return DE_INTERRUPT;
			task_sleep(100);
		}

		return 0;
	}

	return DE_UNSUPPORTED;
}

int console_init(void)
{
	static int run_once;
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = vfs_open("/dev/console", &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	vfs_init_node(&console_node, 0);
	console_node.type = vfs_type_character;
	console_node.n_read = n_read;
	console_node.n_write = n_write;
	console_node.n_ioctl = n_ioctl;

	if ((r = vfs_mount("/dev/console", &console_node)) != 0)
		return r;

	if (!task_create(console_task, NULL, task_detached))
		return DE_MEMORY;

	return 0;
}
