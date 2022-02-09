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
 * kernel/vfs.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include <dancy/types.h>

enum vfs_type {
	vfs_type_unknown   = 0,
	vfs_type_regular   = 1,
	vfs_type_buffer    = 2,
	vfs_type_directory = 3,
	vfs_type_character = 4,
	vfs_type_block     = 5,
	vfs_type_socket    = 6,
	vfs_type_last
};

enum vfs_mode {
	vfs_mode_read_only = 0x01,
	vfs_mode_hidden    = 0x02,
	vfs_mode_system    = 0x04,
	vfs_mode_directory = 0x08,
	vfs_mode_create    = 0x10,
	vfs_mode_truncate  = 0x20,
	vfs_mode_last
};

struct vfs_name;
struct vfs_record;

struct vfs_node {
	int lock;
	int count;

	int type;
	int mode;

	uint64_t id;
	void *internal_data;

	void (*n_release)(struct vfs_node **node);

	int (*n_create)(struct vfs_node *node, struct vfs_node **new_node,
		int type, int mode, struct vfs_name *vname);

	long long (*n_read)(struct vfs_node *node,
		uint64_t offset, size_t size, void *buffer);

	long long (*n_write)(struct vfs_node *node,
		uint64_t offset, size_t size, const void *buffer);

	int (*n_flush)(struct vfs_node *node);

	int (*n_readdir)(struct vfs_node *node,
		uint64_t offset, size_t size, void *record);

	int (*n_link)(struct vfs_node *node, struct vfs_name *vname);
	int (*n_unlink)(struct vfs_node *node, struct vfs_name *vname);
};

struct vfs_name {
	char **components;
	int offset;
	int type;
};

struct vfs_record {
	uint64_t id;
	const char *name;

	int type;
	int mode;
};

/*
 * Declarations of default.c
 */
void vfs_default_release(struct vfs_node **node);

int vfs_default_create(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname);

long long vfs_default_read(struct vfs_node *node,
	uint64_t offset, size_t size, void *buffer);

long long vfs_default_write(struct vfs_node *node,
	uint64_t offset, size_t size, const void *buffer);

int vfs_default_flush(struct vfs_node *node);

int vfs_default_readdir(struct vfs_node *node,
	uint64_t offset, size_t size, void *record);

int vfs_default_link(struct vfs_node *node, struct vfs_name *vname);
int vfs_default_unlink(struct vfs_node *node, struct vfs_name *vname);

/*
 * Declarations of fat_io.c
 */
struct fat_io {
	int (*get_size)(int id, size_t *block_size, size_t *block_total);
	int (*io_read)(int id, size_t lba, size_t *size, void *buf);
	int (*io_write)(int id, size_t lba, size_t *size, const void *buf);

	struct vfs_node *root_node;

	mtx_t fat_mtx;
	void *instance;
	int id;

	int node_count;
	struct vfs_node *node_array[1024];
};

int fat_io_add(struct fat_io *io);

/*
 * Declarations of path.c
 */
int vfs_build_path(const char *name, struct vfs_name *vname);
int vfs_chdir(const char *name);

/*
 * Declarations of root.c
 */
int vfs_init_root(struct vfs_node **node);

/*
 * Declarations of vfs.c
 */
int vfs_init(void);
void vfs_init_node(struct vfs_node *node, size_t size);

int vfs_increment_count(struct vfs_node *node);
int vfs_decrement_count(struct vfs_node *node);

int vfs_mount_node(const char *name, struct vfs_node *node);
int vfs_open_node(const char *name, struct vfs_node **node, int mode);

#endif
