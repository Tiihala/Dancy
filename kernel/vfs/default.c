/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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

static int vfs_default_open(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	(void)node;
	(void)name;
	(void)new_node;
	(void)type;
	(void)mode;

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

static int vfs_default_poll(struct vfs_node *node, int events, int *revents)
{
	int r = 0;

	(void)node;

	if (revents == NULL)
		return DE_ARGUMENT;

	if ((events & POLLIN) != 0)
		r |= POLLIN;

	if ((events & POLLOUT) != 0)
		r |= POLLOUT;

	if ((events & POLLRDNORM) != 0)
		r |= POLLRDNORM;

	if ((events & POLLWRNORM) != 0)
		r |= POLLWRNORM;

	*revents = r;

	return 0;
}

static int vfs_default_ioctl(struct vfs_node *node,
	int request, long long arg)
{
	(void)node;
	(void)request;
	(void)arg;

	return DE_UNSUPPORTED;
}

static int vfs_default_sync(struct vfs_node *node)
{
	(void)node;

	return 0;
}

static int vfs_default_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	if (dent) {
		memset(dent, 0, sizeof(*dent));

		if (node->type == vfs_type_directory) {
			if (offset == 0)
				strcpy(&dent->name[0], ".");
			if (offset == 1)
				strcpy(&dent->name[0], "..");
			return 0;
		}
	}

	return DE_UNSUPPORTED;
}

static int vfs_default_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	(void)node;

	memset(stat, 0, sizeof(*stat));

	return 0;
}

static int vfs_default_truncate(struct vfs_node *node, uint64_t size)
{
	(void)node;
	(void)size;

	return 0;
}

static int vfs_default_remove(struct vfs_node *node,
	const char *name, int dir)
{
	(void)node;
	(void)name;
	(void)dir;

	return DE_UNSUPPORTED;
}

void vfs_default(struct vfs_node *node)
{
	node->n_release  = vfs_default_release;
	node->n_open     = vfs_default_open;
	node->n_read     = vfs_default_read;
	node->n_write    = vfs_default_write;
	node->n_append   = vfs_default_append;
	node->n_poll     = vfs_default_poll;
	node->n_ioctl    = vfs_default_ioctl;
	node->n_sync     = vfs_default_sync;
	node->n_readdir  = vfs_default_readdir;
	node->n_stat     = vfs_default_stat;
	node->n_truncate = vfs_default_truncate;
	node->n_remove   = vfs_default_remove;
}
