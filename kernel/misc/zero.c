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
 * misc/zero.c
 *      Devices /dev/null and /dev/zero
 */

#include <dancy.h>

static struct vfs_node null_node;
static struct vfs_node zero_node;

static int n_read_null(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	return *size = 0, 0;
}

static int n_read_zero(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;

	memset(buffer, 0, *size);

	return 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;
	(void)size;
	(void)buffer;

	return 0;
}

int zero_init(void)
{
	static int run_once;
	struct vfs_node *dev_node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = vfs_open("/dev/", &dev_node, 0, vfs_mode_create)) != 0)
		return r;

	dev_node->n_release(&dev_node);

	if ((r = vfs_open("/dev/null", &dev_node, 0, vfs_mode_create)) != 0)
		return r;

	dev_node->n_release(&dev_node);

	if ((r = vfs_open("/dev/zero", &dev_node, 0, vfs_mode_create)) != 0)
		return r;

	dev_node->n_release(&dev_node);

	vfs_init_node(&null_node, 0);
	null_node.type = vfs_type_character;
	null_node.n_read = n_read_null;
	null_node.n_write = n_write;

	vfs_init_node(&zero_node, 0);
	zero_node.type = vfs_type_character;
	zero_node.n_read = n_read_zero;
	zero_node.n_write = n_write;

	if ((r = vfs_mount("/dev/null", &null_node)) != 0)
		return r;

	if ((r = vfs_mount("/dev/zero", &zero_node)) != 0)
		return r;

	return 0;
}
