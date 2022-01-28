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
	node->n_create  = vfs_default_create;
	node->n_open    = vfs_default_open;

	node->n_read    = vfs_default_read;
	node->n_write   = vfs_default_write;

	node->n_flush   = vfs_default_flush;
	node->n_readdir = vfs_default_readdir;

	node->n_link    = vfs_default_link;
	node->n_unlink  = vfs_default_unlink;
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

int vfs_mount_node(const char *name, struct vfs_node *node)
{
	(void)name;
	(void)node;

	return DE_UNSUPPORTED;
}

int vfs_open_node(const char *name, struct vfs_node **node)
{
	struct vfs_node *new_node = NULL, *ret_node = NULL;
	struct vfs_name vname;
	int i, r;

	if (!name || !node)
		return DE_ARGUMENT;

	*node = ret_node;

	if ((r = vfs_build_path(name, &vname)) != 0)
		return r;

	if (!root_node)
		return DE_UNINITIALIZED;

	vfs_increment_count(root_node);
	ret_node = root_node;

	for (i = 0; /* void */; i++) {
		char *component = vname.components[i];
		int last_component;

		if (!component) {
			int is_dir = (vname.type == vfs_type_directory);

			if (is_dir && ret_node->type != vfs_type_directory) {
				r = DE_DIRECTORY;
				break;
			}

			return (*node = ret_node), 0;
		}

		vname.offset = i;
		r = ret_node->n_create(ret_node, &new_node, 0, 0, &vname);

		if (r != DE_SUCCESS)
			break;

		ret_node->n_release(&ret_node);
		ret_node = new_node;

		last_component = (vname.components[i + 1] == NULL) ? 1 : 0;

		if (!last_component && new_node->type != vfs_type_directory) {
			r = DE_DIRECTORY;
			break;
		}
	}

	ret_node->n_release(&ret_node);

	return r;
}
