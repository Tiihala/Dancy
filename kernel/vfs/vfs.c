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

static mtx_t mount_mtx;
static struct vfs_node *root_node;

struct mount_node {
	const char *name;
	struct vfs_node *node;
	struct mount_node *next;
	struct mount_node *memb;
};

static struct mount_node *mount_tree;

static int create_mount_node(struct mount_node **mnode)
{
	size_t size = sizeof(struct mount_node);

	if ((*mnode = malloc(size)) == NULL)
		return DE_MEMORY;

	memset(*mnode, 0, size);

	return 0;
}

int vfs_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&mount_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if ((r = vfs_init_root(&root_node)) != 0)
		return r;

	if ((r = create_mount_node(&mount_tree)) != 0)
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

static int mount_locked(struct vfs_name *vname, struct vfs_node *node)
{
	struct mount_node *mnode = mount_tree;
	int i, r;

	if (!vname->components[0])
		return DE_BUSY;

	for (i = 0; vname->components[i] != NULL; i++) {
		char *p = vname->components[i];

		if (!mnode->memb) {
			if ((r = create_mount_node(&mnode->memb)) != 0)
				return r;

			if ((mnode->memb->name = strdup(p)) == NULL)
				return DE_MEMORY;
		}

		mnode = mnode->memb;

		for (;;) {
			if (!strcmp(mnode->name, p))
				break;

			if (mnode->next) {
				mnode = mnode->next;
				continue;
			}

			if ((r = create_mount_node(&mnode->next)) != 0)
				return r;

			mnode = mnode->next;

			if ((mnode->name = strdup(p)) == NULL)
				return DE_MEMORY;
		}

		if (vname->components[i + 1] == NULL) {
			if (mnode->node || mnode->memb)
				return DE_BUSY;

			vfs_increment_count(node);
			mnode->node = node;
		}
	}

	return r;
}

int vfs_mount(const char *name, struct vfs_node *node)
{
	size_t stat_size = sizeof(struct vfs_stat);
	struct vfs_stat target_node_stat;
	struct vfs_node *target_node;
	struct vfs_name vname;
	int r;

	if ((r = vfs_open(name, &target_node, 0, 0)) != 0)
		return r;

	if (target_node->n_stat(target_node, &target_node_stat))
		memset(&target_node_stat, 0, stat_size);

	if (target_node->type != node->type) {
		int type_incompatible = 0;

		if (target_node->type == vfs_type_directory)
			type_incompatible = 1;

		if (node->type == vfs_type_directory)
			type_incompatible = 1;

		if (type_incompatible) {
			target_node->n_release(&target_node);
			return DE_TYPE;
		}
	}

	if ((r = vfs_build_path(name, &vname)) != 0) {
		target_node->n_release(&target_node);
		return r;
	}

	if (mtx_lock(&mount_mtx) != thrd_success) {
		target_node->n_release(&target_node);
		return DE_UNEXPECTED;
	}

	if ((r = mount_locked(&vname, node)) == 0) {
		if ((node->node_stat = malloc(stat_size)) != NULL)
			memcpy(node->node_stat, &target_node_stat, stat_size);
	}

	mtx_unlock(&mount_mtx);

	target_node->n_release(&target_node);

	return r;
}

static struct vfs_node *get_mount_node_locked(struct vfs_name *vname)
{
	char **components = vname->components;
	struct mount_node *mnode = mount_tree;
	struct vfs_node *node = root_node;
	int i;

	if (!components[0]) {
		vfs_increment_count(node);
		return node;
	}

	for (i = 0; components[i] != NULL; i++) {
		char *p = components[i];

		if ((mnode = mnode->memb) == NULL)
			break;

		do {
			if (!strcmp(mnode->name, p))
				break;
		} while ((mnode = mnode->next) != NULL);

		if (!mnode)
			break;

		if (mnode->node) {
			vname->components = &components[i + 1];
			node = mnode->node;
		}
	}

	vfs_increment_count(node);

	return node;
}

static struct vfs_node *get_mount_node(struct vfs_name *vname)
{
	struct vfs_node *node;

	if (mtx_lock(&mount_mtx) != thrd_success)
		return NULL;

	node = get_mount_node_locked(vname);
	mtx_unlock(&mount_mtx);

	return node;
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

	if (vname.components[0] != NULL) {
		r = mount_node->n_open(mount_node, &new_node,
			type, mode, &vname);

		if (r != DE_SUCCESS) {
			mount_node->n_release(&mount_node);
			return r;
		}

		if (type == vfs_type_directory) {
			if (new_node->type != vfs_type_directory) {
				new_node->n_release(&new_node);
				mount_node->n_release(&mount_node);
				return DE_FILE;
			}
		}

		if (type != vfs_type_unknown && type != new_node->type) {
			new_node->n_release(&new_node);
			mount_node->n_release(&mount_node);
			return DE_TYPE;
		}
	}

	if (new_node != mount_node)
		mount_node->n_release(&mount_node);

	return (*node = new_node), 0;
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
