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
 * vfs/devfs.c
 *      File system for devices
 */

#include <dancy.h>

static mtx_t devfs_mtx;

struct devfs_data {
	char name[16];
	struct vfs_node *node;
};

#define DEVFS_COUNT 64

static struct devfs_data devfs_table[DEVFS_COUNT];

static struct vfs_node *alloc_node(int i);

static int n_open(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname)
{
	const char *name;
	int i;

	(void)node;
	*new_node = NULL;

	if (type == vfs_type_directory || vname->type == vfs_type_directory)
		return DE_TYPE;

	if ((mode & vfs_mode_exclusive) != 0)
		return DE_ARGUMENT;

	if (vname->components[0] == NULL || vname->components[1] != NULL)
		return DE_PATH;

	name = vname->components[0];

	if (strlen(name) >= sizeof(devfs_table[0].name))
		return DE_PATH;

	if (mtx_lock(&devfs_mtx) != thrd_success)
		return DE_UNEXPECTED;

	for (i = 1; i < DEVFS_COUNT && devfs_table[i].node; i++) {
		if (!strcmp(devfs_table[i].name, name)) {
			vfs_increment_count(devfs_table[i].node);
			*new_node = devfs_table[i].node;
			return mtx_unlock(&devfs_mtx), 0;
		}
	}

	if ((mode & vfs_mode_create) == 0)
		return mtx_unlock(&devfs_mtx), DE_NAME;

	for (i = 1; i < DEVFS_COUNT; i++) {
		if (devfs_table[i].node == NULL) {
			if ((devfs_table[i].node = alloc_node(i)) == NULL)
				return mtx_unlock(&devfs_mtx), DE_MEMORY;

			strcpy(devfs_table[i].name, name);
			*new_node = devfs_table[i].node;

			return mtx_unlock(&devfs_mtx), 0;
		}
	}

	return mtx_unlock(&devfs_mtx), DE_FULL;
}

static int n_readdir(struct vfs_node *node,
	uint32_t pointer, size_t size, void *record)
{
	(void)node;

	memset(record, 0, size);

	if (pointer == 0) {
		if (size < 2)
			return DE_BUFFER;
		strcpy(record, ".");
		return 0;
	}

	if (pointer == 1) {
		if (size < 3)
			return DE_BUFFER;
		strcpy(record, "..");
		return 0;
	}

	if (pointer - 1 >= DEVFS_COUNT)
		return DE_OVERFLOW;

	if (mtx_lock(&devfs_mtx) != thrd_success)
		return DE_UNEXPECTED;

	if (devfs_table[pointer - 1].node == NULL)
		return mtx_unlock(&devfs_mtx), DE_OVERFLOW;

	if (strlen(&devfs_table[pointer - 1].name[0]) >= size)
		return mtx_unlock(&devfs_mtx), DE_BUFFER;

	strcpy(record, &devfs_table[pointer - 1].name[0]);

	return mtx_unlock(&devfs_mtx), 0;
}

static struct vfs_node *alloc_node(int i)
{
	struct vfs_node *node;

	if ((node = malloc(sizeof(*node))) == NULL)
		return NULL;

	vfs_init_node(node, 0);
	node->count = 1;
	node->type = (i == 0) ? vfs_type_directory : vfs_type_regular;

	node->n_open = n_open;
	node->n_readdir = n_readdir;

	devfs_table[i].node = node;

	return node;
}

int devfs_init(void)
{
	static int run_once;
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&devfs_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if ((r = vfs_open("/dev/", &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	if ((node = alloc_node(0)) == NULL)
		return DE_MEMORY;

	r = vfs_mount("/dev/", node);
	node->n_release(&node);

	return r;
}
