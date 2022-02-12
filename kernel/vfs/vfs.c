/*
 * Copyright (c) 2021, 2022 Antti Tiihala
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

	node->n_release = vfs_default_release;
	node->n_open    = vfs_default_open;

	node->n_read    = vfs_default_read;
	node->n_write   = vfs_default_write;

	node->n_flush   = vfs_default_flush;
	node->n_readdir = vfs_default_readdir;
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

	if (node->count > 0)
		node->count -= 1;
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

static struct vfs_node *get_mount_node(struct vfs_name *vname)
{
	vname->offset = 0;

	return root_node;
}

int vfs_open(const char *name, struct vfs_node **node, int type, int mode)
{
	struct vfs_node *new_node, *mount_node;
	struct vfs_name vname;
	int r;

	*node = NULL;

	if ((r = vfs_build_path(name, &vname)) != 0)
		return r;

	if (vname.type == vfs_type_directory) {
		if (type == vfs_type_unknown)
			type = vfs_type_directory;
		if (type != vfs_type_directory)
			return DE_TYPE;
	}

	if ((mount_node = get_mount_node(&vname)) == NULL)
		return DE_UNINITIALIZED;

	new_node = mount_node;

	if (vname.components[vname.offset]) {
		r = mount_node->n_open(mount_node, &new_node,
			type, mode, &vname);

		if (r != DE_SUCCESS)
			return r;

		if (type != vfs_type_unknown && type != new_node->type) {
			new_node->n_release(&new_node);
			return DE_TYPE;
		}
	}

	return (*node = new_node), 0;
}
