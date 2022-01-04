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

static struct vfs_node dev_null_node;
static struct vfs_session dev_null_session;

static void s_release_null(struct vfs_session **session)
{
	struct vfs_session *s = *session;

	s->node->n_release(&s->node);
	*session = NULL;
}

int vfs_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	dev_null_session.node = &dev_null_node;
	dev_null_session.offset = 0;
	dev_null_session.s_release = s_release_null;

	vfs_init_node(&dev_null_node);

	return 0;
}

static void n_release_null(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	void *lock_local = &n->lock;

	spin_enter(&lock_local);

	if (n->count > 0)
		n->count -= 1;

	spin_leave(&lock_local);

	*node = NULL;
}

static int n_create_null(struct vfs_node *node, struct vfs_node **new_node,
		int type, int mode, const char *name)
{
	(void)node;
	(void)new_node;
	(void)type;
	(void)mode;
	(void)name;

	return DE_UNSUPPORTED;
}

static int n_open_null(struct vfs_node *node, struct vfs_session **session)
{
	struct vfs_session *ret_session = &dev_null_session;
	void *lock_local = &node->lock;

	spin_enter(&lock_local);

	if (node->count < INT_MAX)
		node->count += 1;
	else
		ret_session = NULL;

	spin_leave(&lock_local);

	if ((*session = ret_session) == NULL)
		return DE_OVERFLOW;

	return 0;
}

static long long n_read_null(struct vfs_node *node,
		uint64_t offset, size_t size, void *buffer)
{
	const long long ret = -1;

	(void)node;
	(void)offset;
	(void)size;
	(void)buffer;

	return ret;
}

static long long n_write_null(struct vfs_node *node,
		uint64_t offset, size_t size, const void *buffer)
{
	long long ret;

	(void)node;
	(void)offset;
	(void)buffer;

#if SIZE_MAX < LLONG_MAX
	ret = (long long)size;
#else
	if (size < (unsigned long long)(LLONG_MAX))
		ret = (long long)size;
	else
		ret = LLONG_MAX;
#endif
	return ret;
}

static int n_flush_null(struct vfs_node *node)
{
	(void)node;

	return 0;
}

static int n_readdir_null(struct vfs_node *node,
		uint64_t offset, size_t size, void *record)
{
	(void)node;
	(void)offset;

	if (record)
		memset(record, 0, size);

	return DE_UNSUPPORTED;
}

static int n_link_null(struct vfs_node *node, const char *name)
{
	(void)node;
	(void)name;

	return DE_UNSUPPORTED;
}

static int n_unlink_null(struct vfs_node *node, const char *name)
{
	(void)node;
	(void)name;

	return DE_UNSUPPORTED;
}

void vfs_init_node(struct vfs_node *node)
{
	size_t size = sizeof(struct vfs_node);

	memset(node, 0, size);
	node->type = vfs_type_character;

	node->n_release = n_release_null;
	node->n_create = n_create_null;
	node->n_open = n_open_null;

	node->n_read = n_read_null;
	node->n_write = n_write_null;

	node->n_flush = n_flush_null;
	node->n_readdir = n_readdir_null;

	node->n_link = n_link_null;
	node->n_unlink = n_unlink_null;
}

int vfs_open_node(const char *name, struct vfs_node **node)
{
	char **path = vfs_build_path(name);
	struct vfs_node *ret_node = NULL;

	if (!node)
		return DE_ARGUMENT;

	*node = ret_node;

	if (!path)
		return DE_ARGUMENT;

	return DE_NAME;
}
