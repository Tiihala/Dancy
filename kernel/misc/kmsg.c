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
 * misc/kmsg.c
 *      Device /dev/kmsg
 */

#include <dancy.h>

static struct vfs_node kmsg_node;
static size_t kmsg_find_i;

static int kmsg_check(size_t i, uint64_t offset)
{
	volatile char *p = (volatile char *)kernel->kmsg.buffer;
	uint64_t counter = 0;
	int r = DE_SEARCH;

	if (p[i] != '\t')
		return DE_SEARCH;
	if (++i >= kernel->kmsg.size)
		i = 0;

	while (p[i] >= '0' && p[i] <= '9') {
		if (++i >= kernel->kmsg.size)
			i = 0;
	}

	if (p[i] != ',')
		return DE_SEARCH;
	if (++i >= kernel->kmsg.size)
		i = 0;

	while (p[i] >= '0' && p[i] <= '9') {
		counter *= 10;
		counter += (uint64_t)(p[i] - '0');

		if (++i >= kernel->kmsg.size)
			i = 0;
		r = 0;
	}

	if (p[i] != ',' || counter != offset)
		return DE_SEARCH;

	return r;
}

static char *kmsg_find(size_t i, uint64_t offset)
{
	volatile char *p = (volatile char *)kernel->kmsg.buffer;
	size_t start_i = i;

	if (start_i >= kernel->kmsg.size)
		start_i = i = 0;

	for (;;) {
		char c = p[i];

		if (c == '\t' && kmsg_check(i, offset) == 0)
			return (char *)&p[i];

		if (++i >= kernel->kmsg.size)
			i = 0;

		if (i == start_i)
			break;
	}

	return NULL;
}

static int kmsg_read(char *p, uint64_t offset, size_t *size, char *buffer)
{
	size_t i = (size_t)(p - kernel->kmsg.buffer);
	size_t buffer_size = *size;

	if (kmsg_check(i, offset) != 0)
		return DE_RETRY;

	if (++i >= kernel->kmsg.size)
		i = 0;

	p = kernel->kmsg.buffer;
	*size = 0;

	for (;;) {
		char c = p[i];

		if (*size >= buffer_size) {
			memset(buffer, 0, buffer_size);
			return *size = 0, DE_BUFFER;
		}

		buffer[*size] = c, (*size += 1);

		if (c == '\n')
			break;

		if (c < 0x20 || c > 0x7E) {
			memset(buffer, 0, buffer_size);
			return *size = 0, DE_UNEXPECTED;
		}

		if (++i >= kernel->kmsg.size)
			i = 0;
	}

	kmsg_find_i = i;

	return 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	void *lock_local = &kernel->kmsg.lock;
	int r = DE_RETRY;
	size_t i;

	(void)node;

	spin_enter(&lock_local);

	if (offset >= kernel->kmsg.counter)
		*size = 0, r = 0;

	i = kmsg_find_i;

	spin_leave(&lock_local);

	while (r == DE_RETRY) {
		char *p = kmsg_find(i, offset);

		if (p == NULL)
			return *size = 0, DE_PLACEHOLDER;

		spin_enter(&lock_local);

		r = kmsg_read(p, offset, size, buffer);

		spin_leave(&lock_local);
	}

	return r;
}

int kmsg_init(void)
{
	static int run_once;
	const char *name = "/dev/kmsg";
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = vfs_open(name, &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	if (!(kernel->kmsg.event = event_create(0)))
		return DE_MEMORY;

	vfs_init_node(&kmsg_node, 0);
	kmsg_node.type = vfs_type_message;
	kmsg_node.internal_event = &kernel->kmsg.event;
	kmsg_node.n_read = n_read;

	if ((r = vfs_mount(name, &kmsg_node)) != 0)
		return r;

	return 0;
}
