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

void vfs_default_release(struct vfs_node **node)
{
	*node = NULL;
}

int vfs_default_open(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname)
{
	(void)node;
	(void)new_node;
	(void)type;
	(void)mode;
	(void)vname;

	return DE_UNSUPPORTED;
}

long long vfs_default_read(struct vfs_node *node,
	uint64_t offset, size_t size, void *buffer)
{
	long long ret = -((long long)DE_UNSUPPORTED);

	(void)node;
	(void)offset;
	(void)size;
	(void)buffer;

	return ret;
}

long long vfs_default_write(struct vfs_node *node,
	uint64_t offset, size_t size, const void *buffer)
{
	long long ret = -((long long)DE_UNSUPPORTED);

	(void)node;
	(void)offset;
	(void)size;
	(void)buffer;

	return ret;
}

int vfs_default_flush(struct vfs_node *node)
{
	(void)node;

	return 0;
}

int vfs_default_readdir(struct vfs_node *node,
	uint64_t offset, size_t size, void *record)
{
	(void)node;
	(void)offset;

	if (record)
		memset(record, 0, size);

	return DE_UNSUPPORTED;
}

int vfs_default_rename(struct vfs_node *node,
	struct vfs_name *old_vname, struct vfs_name *new_vname)
{
	(void)node;
	(void)old_vname;
	(void)new_vname;

	return DE_UNSUPPORTED;
}

int vfs_default_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	(void)node;

	if (stat)
		memset(stat, 0, sizeof(*stat));

	return DE_UNSUPPORTED;
}

int vfs_default_truncate(struct vfs_node *node, uint64_t size)
{
	(void)node;
	(void)size;

	return DE_UNSUPPORTED;
}

int vfs_default_unlink(struct vfs_node *node, struct vfs_name *vname)
{
	(void)node;
	(void)vname;

	return DE_UNSUPPORTED;
}
