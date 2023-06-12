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

static mtx_t tree_mtx;
static struct vfs_node *root_node;

int vfs_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&tree_mtx, mtx_plain) != thrd_success)
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

static const char *unexpected_lock_error = "VFS: unexpected lock error";
static const char *unexpected_tree_error = "VFS: unexpected tree structure";

void vfs_lock_tree(void)
{
	if (mtx_lock(&tree_mtx) != thrd_success)
		kernel->panic(unexpected_lock_error);
}

void vfs_unlock_tree(void)
{
	mtx_unlock(&tree_mtx);
}

static void add_node(struct vfs_node *owner, struct vfs_node *node)
{
	if (owner == node)
		kernel->panic(unexpected_tree_error);

	node->tree[0] = owner;
	node->tree[1] = owner->tree[2];
	node->tree[2] = NULL;

	owner->tree[2] = node;
}

static void remove_leaf_node(struct vfs_node *node)
{
	struct vfs_node *owner = node->tree[0];
	struct vfs_node *p1 = NULL;
	struct vfs_node *p2 = owner->tree[2];
	int i = 0;

	if (node->tree_state != 0 || node->tree[2] != NULL || p2 == NULL)
		kernel->panic(unexpected_tree_error);

	while (p2 != node) {
		p1 = p2;
		p2 = p2->tree[1];

		if (++i == INT_MAX || p2 == NULL)
			kernel->panic(unexpected_tree_error);
	}

	if (p1 == NULL)
		owner->tree[2] = p2->tree[1];
	else
		p1->tree[1] = p2->tree[1];
}

static struct vfs_node *find_node(struct vfs_node *owner, const char *name)
{
	struct vfs_node *p = owner->tree[2];
	int i = 0;

	while (p != NULL) {
		if (!strcmp(&p->name[0], name))
			return p;

		p = p->tree[1];

		if (++i == INT_MAX)
			kernel->panic(unexpected_tree_error);
	}

	return NULL;
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

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct vfs_node *owner = n->tree[0];

	*node = NULL;

	vfs_lock_tree();

	if (n->tree_state > 0) {
		n->tree_state -= 1;
		vfs_decrement_count(n);
	}

	if (n->tree_state == 0 && n->tree[2] == NULL)
		remove_leaf_node(n);

	n->__release(&n);

	while (owner != root_node) {
		struct vfs_node *unused_node;

		if (owner->tree_state != 0 || owner->tree[2] != NULL)
			break;

		unused_node = owner;
		owner = owner->tree[0];

		remove_leaf_node(unused_node);
		unused_node->__release(&unused_node);
	}

	vfs_unlock_tree();
}

int vfs_open(const char *name, struct vfs_node **node, int type, int mode)
{
	struct vfs_node *owner = root_node;
	struct vfs_node *new_node = NULL;
	struct vfs_name vname;
	int i, r;

	*node = NULL;

	if ((r = vfs_build_path(name, &vname)) != 0)
		return r;

	if (vname.type == vfs_type_directory) {
		if (type == vfs_type_unknown)
			type = vfs_type_directory;

		if (type != vfs_type_directory)
			return DE_TYPE;
	}

	if (vname.components[0] == NULL) {
		if (type != vfs_type_unknown && type != vfs_type_directory)
			return DE_TYPE;

		return *node = root_node, 0;
	}

	vfs_lock_tree();

	for (i = 0; vname.components[i] != NULL; i++) {
		const char *n = vname.components[i];
		int last_component = (vname.components[i + 1] == NULL);

		if ((new_node = find_node(owner, n)) != NULL) {
			if (last_component) {
				int new_mode = new_node->mode;

				if ((new_mode & vfs_mode_exclusive) != 0) {
					r = DE_BUSY;
					break;
				}

				new_node->tree_state += 1;
				vfs_increment_count(new_node);

				*node = new_node;
				break;
			}

			owner = new_node;
			continue;
		}

		if ((r = owner->n_open(owner, n, &new_node, type, mode)) != 0)
			break;

		new_node->__release = new_node->n_release;
		new_node->n_release = n_release;
		strcpy(&new_node->name[0], n);

		add_node(owner, new_node);

		if (last_component) {
			new_node->tree_state += 1;
			vfs_increment_count(new_node);

			*node = new_node;
			break;
		}

		owner = new_node;
	}

	while (r != 0 && owner != root_node) {
		struct vfs_node *unused_node;

		if (owner->tree_state != 0 || owner->tree[2] != NULL)
			break;

		unused_node = owner;
		owner = owner->tree[0];

		remove_leaf_node(unused_node);
		unused_node->__release(&unused_node);
	}

	vfs_unlock_tree();

	if (*node == NULL)
		return (r != 0) ? r : DE_UNEXPECTED;

	if (type == vfs_type_directory && type != new_node->type) {
		new_node->n_release(&new_node);
		return (*node = NULL), DE_FILE;
	}

	if (type != vfs_type_unknown && type != new_node->type) {
		new_node->n_release(&new_node);
		return (*node = NULL), DE_TYPE;
	}

	return 0;
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
