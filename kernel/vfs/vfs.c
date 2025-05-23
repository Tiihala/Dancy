/*
 * Copyright (c) 2021, 2022, 2023, 2024, 2025 Antti Tiihala
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

static void n_release(struct vfs_node **node);
static void n_release_panic(struct vfs_node **node);

#define VFS_PATH_COUNT  32
#define VFS_PATH_BUFFER 1024

static struct {
	char *components[VFS_PATH_COUNT];
	int type;

	char wd[VFS_PATH_BUFFER];
	char b[VFS_PATH_BUFFER];
} vname;

static int build_vname_locked(const char *name)
{
	char *b = &vname.b[0];
	int i, r, count = 0;

	for (i = 0; i < VFS_PATH_COUNT; i++)
		vname.components[i] = NULL;

	vname.type = 0;

	if (name[0] == '\0')
		return DE_PATH;

	if (name[0] != '/') {
		struct vfs_node *wd_node = task_current()->fd.wd_node;
		size_t wd_size = VFS_PATH_BUFFER - 16;
		char *w = &vname.wd[0];

		if ((r = vfs_realpath(wd_node, &vname.wd[0], wd_size)) != 0)
			return r;

		for (i = 0; /* void */; i++) {
			char c = vname.wd[i];

			if (c == '/' || c == '\0') {
				if (count + 4 >= VFS_PATH_COUNT)
					return DE_PATH;

				vname.wd[i] = '\0';

				if (w != &vname.wd[i])
					vname.components[count++] = w;

				w = &vname.wd[i + 1];
			}

			if (c == '\0')
				break;
		}
	}

	for (i = 0; /* void */; i++) {
		char c = name[i];

		if (i + 16 >= VFS_PATH_BUFFER)
			return DE_PATH;

		vname.b[i] = c;

		if (c == '/' || c == '\0') {
			if (count + 4 >= VFS_PATH_COUNT)
				return DE_PATH;

			vname.b[i] = '\0';

			if (b != &vname.b[i])
				vname.components[count++] = b;

			b = &vname.b[i + 1];
		}

		while (count > 0) {
			char *p = vname.components[count - 1];

			if (p[0] == '.' && p[1] == '\0') {
				vname.components[--count] = NULL;
				vname.type = vfs_type_directory;
				break;
			}

			if (p[0] == '.' && p[1] == '.' && p[2] == '\0') {
				vname.components[--count] = NULL;
				if (count > 0)
					vname.components[--count] = NULL;
				vname.type = vfs_type_directory;
				break;
			}

			vname.type = 0;
			break;
		}

		if (c == '\0') {
			if (i > 0 && name[i - 1] == '/')
				vname.type = vfs_type_directory;
			break;
		}
	}

	return 0;
}

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

	vfs_lock_tree();

	root_node->mount_state = 1;
	root_node->_release = n_release_panic;
	root_node->n_release = n_release;

	vfs_unlock_tree();

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

static void set_n_release(struct vfs_node *node)
{
	void *lock_local = &node->lock;

	if (node->n_release == n_release)
		return;

	spin_enter(&lock_local);

	if (node->n_release != n_release) {
		node->_release = node->n_release;
		node->n_release = n_release;
	}

	spin_leave(&lock_local);
}

static void unset_n_release(struct vfs_node *node)
{
	void *lock_local = &node->lock;

	if (node->n_release != n_release)
		return;

	spin_enter(&lock_local);

	if (node->n_release == n_release) {
		node->n_release = node->_release;
		node->_release = n_release_panic;
	}

	spin_leave(&lock_local);
}

static void set_tree_state(struct vfs_node *node)
{
	void *lock_local = &node->lock;

	if (node->tree_state > 0)
		return;

	if (node->n_release != n_release)
		kernel->panic(unexpected_tree_error);

	spin_enter(&lock_local);

	if (node->tree_state == 0)
		node->tree_state = 1;

	spin_leave(&lock_local);
}

int vfs_increment_count(struct vfs_node *node)
{
	void *lock_local = &node->lock;
	int r;

	spin_enter(&lock_local);

	if (node->count >= 0 && node->count < INT_MAX)
		node->count += 1;

	r = node->count;

	if (node->tree_state > 0 && node->tree_state < UINT_MAX)
		node->tree_state += 1;

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

	if (node->tree_state > 0)
		node->tree_state -= 1;

	spin_leave(&lock_local);

	return r;
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct vfs_node *owner;

	*node = NULL;

	vfs_lock_tree();
	vfs_decrement_count(n);

	if (n->mount_state != 0) {
		vfs_unlock_tree();
		return;
	}

	owner = n->tree[0];

	if (n->tree_state == 0 && n->tree[2] == NULL) {
		remove_leaf_node(n);
		unset_n_release(n);
		n->n_release(&n);
	}

	while (owner != root_node) {
		struct vfs_node *unused_node;

		if (owner->tree_state != 0 || owner->tree[2] != NULL)
			break;

		if (owner->mount_state != 0)
			break;

		unused_node = owner;
		owner = owner->tree[0];

		remove_leaf_node(unused_node);
		unset_n_release(unused_node);
		unused_node->n_release(&unused_node);
	}

	vfs_unlock_tree();
}

static void n_release_panic(struct vfs_node **node)
{
	(void)node, kernel->panic(unexpected_tree_error);
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct vfs_node *mount = node->mount;

	return mount->n_stat(mount, stat);
}

int vfs_mount(const char *name, struct vfs_node *node)
{
	struct vfs_node *target_node;
	int r;

	if ((r = vfs_open(name, &target_node, 0, 0)) != 0)
		return r;

	if (node == root_node || target_node == root_node)
		return target_node->n_release(&target_node), DE_BUSY;

	if (target_node->type != node->type) {
		if (target_node->type == vfs_type_directory)
			return target_node->n_release(&target_node), DE_TYPE;

		if (node->type == vfs_type_directory)
			return target_node->n_release(&target_node), DE_TYPE;
	}

	vfs_lock_tree();

	{
		int de_busy = 0;

		de_busy += (target_node->count > 2);
		de_busy += (target_node->tree[2] != NULL);

		de_busy += (target_node->tree_state != 1);
		de_busy += (target_node->mount_state != 0);

		de_busy += (node->tree[0] != NULL);
		de_busy += (node->tree[1] != NULL);
		de_busy += (node->tree[2] != NULL);

		de_busy += (node->tree_state != 0);
		de_busy += (node->mount_state != 0);

		if (de_busy) {
			vfs_unlock_tree();
			target_node->n_release(&target_node);
			return DE_BUSY;
		}
	}

	vfs_increment_count(node);
	strcpy(&node->name[0], &target_node->name[0]);

	node->tree[0] = target_node->tree[0];
	node->tree[1] = target_node->tree[1];

	{
		struct vfs_node *owner = target_node->tree[0];
		struct vfs_node *p1 = NULL;
		struct vfs_node *p2 = owner->tree[2];
		int i = 0;

		if (p2 == NULL)
			kernel->panic(unexpected_tree_error);

		if (p2 == target_node)
			owner->tree[2] = node;

		while (p2 != target_node) {
			p1 = p2;
			p2 = p2->tree[1];

			if (++i == INT_MAX || p2 == NULL)
				kernel->panic(unexpected_tree_error);
		}

		if (p1 != NULL)
			p1->tree[1] = node;
	}

	target_node->tree[0] = NULL;
	target_node->tree[1] = NULL;

	if (node->tree[2] != NULL || target_node->tree[2] != NULL)
		kernel->panic(unexpected_tree_error);

	node->mount = target_node;
	node->tree_state = 0;
	node->mount_state = 1;

	node->_release = n_release_panic;
	node->n_release = n_release;

	if (node->type == vfs_type_directory)
		node->n_stat = n_stat;

	vfs_unlock_tree();

	return 0;
}

int vfs_open(const char *name, struct vfs_node **node, int type, int mode)
{
	struct vfs_node *owner = root_node;
	struct vfs_node *new_node = NULL;
	int i, r;

	*node = NULL;

	vfs_lock_tree();

	if ((r = build_vname_locked(name)) != 0)
		return vfs_unlock_tree(), r;

	if (vname.type == vfs_type_directory) {
		if (type == vfs_type_unknown)
			type = vfs_type_directory;

		if (type != vfs_type_directory)
			return vfs_unlock_tree(), DE_TYPE;
	}

	if ((mode & vfs_mode_write) != 0 && type == vfs_type_directory)
		return vfs_unlock_tree(), DE_DIRECTORY;

	if ((mode & vfs_mode_exclusive) != 0 && type == vfs_type_directory) {
		if ((mode & vfs_mode_create) == 0)
			return vfs_unlock_tree(), DE_ARGUMENT;
	}

	if (vname.components[0] == NULL) {
		if (type != vfs_type_unknown && type != vfs_type_directory)
			return vfs_unlock_tree(), DE_TYPE;

		vfs_increment_count(root_node);
		set_tree_state(root_node);

		vfs_unlock_tree();

		return *node = root_node, 0;
	}

	for (i = 0; vname.components[i] != NULL; i++) {
		const char *n = vname.components[i];
		int last_component = (vname.components[i + 1] == NULL);

		if ((new_node = find_node(owner, n)) != NULL) {
			if (last_component) {
				int new_mode = new_node->mode;
				int de_busy = 0;

				de_busy |= (mode & vfs_mode_exclusive);
				de_busy |= (new_mode & vfs_mode_exclusive);

				if (new_node->mount_state != 0) {
					if ((mode & vfs_mode_create) == 0) {
						if (new_node->count == 1)
							de_busy = 0;
					}
				}

				if ((mode & vfs_mode_stat_only) != 0)
					de_busy = 0;

				if (de_busy) {
					r = DE_BUSY;
					break;
				}

				vfs_increment_count(new_node);
				set_tree_state(new_node);

				*node = new_node;
				break;
			}

			owner = new_node;
			continue;
		}

		if (last_component) {
			r = owner->n_open(owner, n, &new_node, type, mode);

			if (r != 0)
				break;
		} else {
			r = owner->n_open(owner, n, &new_node, 0, 0);

			if (r != 0)
				break;
		}

		set_n_release(new_node);
		strcpy(&new_node->name[0], n);

		add_node(owner, new_node);

		if (last_component) {
			vfs_increment_count(new_node);
			set_tree_state(new_node);

			*node = new_node;
			break;
		}

		owner = new_node;
	}

	while (r != 0 && owner != root_node) {
		struct vfs_node *unused_node;

		if (owner->tree_state != 0 || owner->tree[2] != NULL)
			break;

		if (owner->mount_state != 0)
			break;

		unused_node = owner;
		owner = owner->tree[0];

		remove_leaf_node(unused_node);
		unset_n_release(unused_node);
		unused_node->n_release(&unused_node);
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

	if ((mode & vfs_mode_truncate) != 0) {
		if ((r = new_node->n_truncate(new_node, 0)) != 0) {
			new_node->n_release(&new_node);
			return (*node = NULL), r;
		}
	}

	return 0;
}

int vfs_realpath(struct vfs_node *node, void *buffer, size_t size)
{
	struct vfs_node *n = node;
	char *b = buffer;
	size_t s = 0;
	int depth = 0;
	int i, j;

	if (size < 2) {
		if (size != 0)
			b[s] = '\0';
		return DE_OVERFLOW;
	}

	if (node == NULL)
		return b[s] = '\0', 0;

	while (n->tree[0] != NULL)
		n = n->tree[0], depth += 1;

	if (depth == 0) {
		if (node != root_node)
			return memset(buffer, 0, size), DE_UNEXPECTED;
		b[s++] = '/';
	}

	for (i = 0; i < depth; i++) {
		const char *p = &node->name[0];
		char c = '/';

		for (j = i + 1, n = node; j < depth; j++)
			n = n->tree[0], p = &n->name[0];

		if (p[0] == '\0')
			return memset(buffer, 0, size), DE_UNEXPECTED;

		do {
			if ((s + 1) >= size)
				return memset(buffer, 0, size), DE_OVERFLOW;

			b[s++] = c;

		} while ((c = *p++) != '\0');
	}

	b[s] = '\0';

	return 0;
}

int vfs_remove(const char *name, int dir)
{
	struct vfs_node *owner = root_node;
	int i, r;

	vfs_lock_tree();

	if ((r = build_vname_locked(name)) != 0)
		return vfs_unlock_tree(), r;

	if (!dir && vname.type == vfs_type_directory)
		return vfs_unlock_tree(), DE_TYPE;

	if (vname.components[0] == NULL)
		return vfs_unlock_tree(), DE_BUSY;

	for (i = 0; vname.components[i] != NULL; i++) {
		const char *n = vname.components[i];
		int last_component = (vname.components[i + 1] == NULL);
		struct vfs_node *new_node;

		if ((new_node = find_node(owner, n)) != NULL) {
			if (last_component) {
				r = DE_BUSY;
				break;
			}

			owner = new_node;
			continue;
		}

		if (last_component) {
			r = owner->n_remove(owner, n, dir);
			break;
		}

		if ((r = owner->n_open(owner, n, &new_node, 0, 0)) != 0)
			break;

		set_n_release(new_node);
		strcpy(&new_node->name[0], n);

		add_node(owner, new_node);
		owner = new_node;
	}

	while (owner != root_node) {
		struct vfs_node *unused_node;

		if (owner->tree_state != 0 || owner->tree[2] != NULL)
			break;

		if (owner->mount_state != 0)
			break;

		unused_node = owner;
		owner = owner->tree[0];

		remove_leaf_node(unused_node);
		unset_n_release(unused_node);
		unused_node->n_release(&unused_node);
	}

	vfs_unlock_tree();

	return r;
}

int vfs_rename(const char *old_name, const char *new_name)
{
	(void)old_name;
	(void)new_name;

	return DE_UNSUPPORTED;
}
