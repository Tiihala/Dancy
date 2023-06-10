/*
 * Copyright (c) 2021, 2022, 2023 Antti Tiihala
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
 * vfs/vfs.c
 *      Virtual File System
 */

#include <dancy.h>

static struct vfs_node *root_node;

int vfs_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = vfs_init_root(&root_node)) != 0)
		return r;

	return 0;
}

void vfs_init_node(struct vfs_node *node, size_t size)
{
	size_t node_size = sizeof(struct vfs_node);

	if (node_size < size)
		node_size = size;

	memset(node, 0, node_size);

	vfs_default(node);
}

int vfs_increment_count(struct vfs_node *node)
{
	void *lock_local = &node->lock;
	int r;

	spin_enter(&lock_local);

	if (node->count >= 0 && node->count < INT_MAX)
		node->count += 1;
	r = node->count;

	spin_leave(&lock_local);

	return r;
}

int vfs_decrement_count(struct vfs_node *node)
{
	void *lock_local = &node->lock;
	int r;

	spin_enter(&lock_local);

	if (node->count >= 0)
		node->count -= 1;
	if (node->count < -1)
		node->count = -1;
	r = node->count;

	spin_leave(&lock_local);

	return r;
}

int vfs_mount(const char *name, struct vfs_node *node)
{
	(void)name;
	(void)node;

	return DE_UNSUPPORTED;
}

int vfs_open(const char *name, struct vfs_node **node, int type, int mode)
{
	(void)name;
	(void)node;
	(void)type;
	(void)mode;

	return DE_UNSUPPORTED;
}

int vfs_rename(const char *old_name, const char *new_name)
{
	(void)old_name;
	(void)new_name;

	return DE_UNSUPPORTED;
}

int vfs_unlink(const char *name)
{
	(void)name;

	return DE_UNSUPPORTED;
}

int vfs_rmdir(const char *name)
{
	(void)name;

	return DE_UNSUPPORTED;
}
