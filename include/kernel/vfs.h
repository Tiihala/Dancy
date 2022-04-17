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

#include <dancy/time.h>
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
	vfs_mode_create    = 0x0001,
	vfs_mode_truncate  = 0x0002,
	vfs_mode_exclusive = 0x0004,
	vfs_mode_read_only = 0x0100,
	vfs_mode_hidden    = 0x0200,
	vfs_mode_system    = 0x0400,
	vfs_mode_last
};

struct vfs_name;
struct vfs_record;
struct vfs_stat;

struct vfs_node {
	int lock;
	int count;

	int type;
	int mode;

	void *internal_data;

	void (*n_release)(struct vfs_node **node);

	int (*n_open)(struct vfs_node *node, struct vfs_node **new_node,
		int type, int mode, struct vfs_name *vname);

	long long (*n_read)(struct vfs_node *node,
		uint64_t offset, size_t size, void *buffer);

	long long (*n_write)(struct vfs_node *node,
		uint64_t offset, size_t size, const void *buffer);

	int (*n_flush)(struct vfs_node *node);

	int (*n_readdir)(struct vfs_node *node,
		uint64_t offset, size_t size, void *record);

	int (*n_rename)(struct vfs_node *node,
		struct vfs_name *old_vname, struct vfs_name *new_vname);

	int (*n_stat)(struct vfs_node *node, struct vfs_stat *stat);
	int (*n_truncate)(struct vfs_node *node, uint64_t size);
	int (*n_unlink)(struct vfs_node *node, struct vfs_name *vname);
};

struct vfs_name {
	void *buffer;
	char **components;
	int pointer;
	int type;
};

struct vfs_record {
	const char *name;
	int type;
	int mode;
};

struct vfs_stat {
	uint64_t size;
	struct timespec access_time;
	struct timespec creation_time;
	struct timespec write_time;
};

#ifdef DANCY_32
#define VFS_SIZE_TO_LLONG(a) \
	((long long)(a))
#endif

#ifdef DANCY_64
#define VFS_SIZE_TO_LLONG(a) \
	((long long)((a) > (unsigned long long)(LLONG_MAX) ? LLONG_MAX : (a)))
#endif

/*
 * Declarations of default.c
 */
void vfs_default_release(struct vfs_node **node);

int vfs_default_open(struct vfs_node *node, struct vfs_node **new_node,
	int type, int mode, struct vfs_name *vname);

long long vfs_default_read(struct vfs_node *node,
	uint64_t offset, size_t size, void *buffer);

long long vfs_default_write(struct vfs_node *node,
	uint64_t offset, size_t size, const void *buffer);

int vfs_default_flush(struct vfs_node *node);

int vfs_default_readdir(struct vfs_node *node,
	uint64_t offset, size_t size, void *record);

int vfs_default_rename(struct vfs_node *node,
	struct vfs_name *old_vname, struct vfs_name *new_vname);

int vfs_default_stat(struct vfs_node *node, struct vfs_stat *stat);
int vfs_default_truncate(struct vfs_node *node, uint64_t size);
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
int vfs_duplicate_path(struct vfs_name *vname);

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

int vfs_mount(const char *name, struct vfs_node *node);
int vfs_open(const char *name, struct vfs_node **node, int type, int mode);
int vfs_rename(const char *old_name, const char *new_name);
int vfs_unlink(const char *name);

#endif
