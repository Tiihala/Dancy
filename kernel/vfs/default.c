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
 * vfs/default.c
 *      Default function implementations
 */

#include <dancy.h>

static void vfs_default_release(struct vfs_node **node)
{
	vfs_decrement_count(*node);
	*node = NULL;
}

static int vfs_default_open(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname)
{
	(void)node;
	(void)new_node;
	(void)type;
	(void)mode;
	(void)vname;

	return DE_UNSUPPORTED;
}

static int vfs_default_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int vfs_default_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;
	(void)buffer;

	*size = 0;

	return DE_UNSUPPORTED;
}

static int vfs_default_append(struct vfs_node *node,
	size_t *size, const void *buffer)
{
	const unsigned long long offset = 0xFFFFFFFFFFFFFFFFull;

	return node->n_write(node, (uint64_t)offset, size, buffer);
}

static int vfs_default_sync(struct vfs_node *node)
{
	(void)node;

	return 0;
}

static int vfs_default_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	(void)node;
	(void)offset;

	if (dent)
		memset(dent, 0, sizeof(*dent));

	return DE_UNSUPPORTED;
}

static int vfs_default_rename(struct vfs_node *node,
	struct vfs_name *old_vname, struct vfs_name *new_vname)
{
	(void)node;
	(void)old_vname;
	(void)new_vname;

	return DE_UNSUPPORTED;
}

static int vfs_default_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	if (node->node_stat)
		memcpy(stat, node->node_stat, sizeof(*stat));
	else
		memset(stat, 0, sizeof(*stat));

	return 0;
}

static int vfs_default_truncate(struct vfs_node *node, uint64_t size)
{
	(void)node;
	(void)size;

	return DE_UNSUPPORTED;
}

static int vfs_default_unlink(struct vfs_node *node, struct vfs_name *vname)
{
	(void)node;
	(void)vname;

	return DE_UNSUPPORTED;
}

void vfs_default(struct vfs_node *node)
{
	node->n_release  = vfs_default_release;
	node->n_open     = vfs_default_open;
	node->n_read     = vfs_default_read;
	node->n_write    = vfs_default_write;
	node->n_append   = vfs_default_append;
	node->n_sync     = vfs_default_sync;
	node->n_readdir  = vfs_default_readdir;
	node->n_rename   = vfs_default_rename;
	node->n_stat     = vfs_default_stat;
	node->n_truncate = vfs_default_truncate;
	node->n_unlink   = vfs_default_unlink;
}
