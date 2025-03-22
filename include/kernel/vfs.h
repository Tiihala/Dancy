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
 * kernel/vfs.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include <common/types.h>

enum vfs_type {
	vfs_type_unknown   = 0,
	vfs_type_regular   = 1,
	vfs_type_buffer    = 2,
	vfs_type_directory = 3,
	vfs_type_character = 4,
	vfs_type_block     = 5,
	vfs_type_socket    = 6,
	vfs_type_message   = 7,
	vfs_type_last
};

enum vfs_mode {
	vfs_mode_create    = 0x0001,
	vfs_mode_truncate  = 0x0002,
	vfs_mode_exclusive = 0x0004,
	vfs_mode_read_only = 0x0100,
	vfs_mode_hidden    = 0x0200,
	vfs_mode_system    = 0x0400,
	vfs_mode_write     = 0x0800,
	vfs_mode_stat_only = 0x1000,
	vfs_mode_last
};

struct vfs_dent;
struct vfs_stat;

struct vfs_node {
	int lock;
	int count;

	int type;
	int mode;

	struct vfs_node *tree[3];
	struct vfs_node *mount;
	unsigned int tree_state;
	unsigned int mount_state;

	void *internal_data;
	event_t *internal_event;

	void (*_release)(struct vfs_node **node);
	void (*n_release)(struct vfs_node **node);

	int (*n_open)(struct vfs_node *node, const char *name,
		struct vfs_node **new_node, int type, int mode);

	int (*n_read)(struct vfs_node *node,
		uint64_t offset, size_t *size, void *buffer);

	int (*n_write)(struct vfs_node *node,
		uint64_t offset, size_t *size, const void *buffer);

	int (*n_append)(struct vfs_node *node,
		size_t *size, const void *buffer);

	int (*n_poll)(struct vfs_node *node, int events, int *revents);
	int (*n_ioctl)(struct vfs_node *node, int request, long long arg);
	int (*n_sync)(struct vfs_node *node);

	int (*n_readdir)(struct vfs_node *node,
		uint32_t offset, struct vfs_dent *dent);

	int (*n_stat)(struct vfs_node *node, struct vfs_stat *stat);
	int (*n_truncate)(struct vfs_node *node, uint64_t size);

	int (*n_remove)(struct vfs_node *node, const char *name, int dir);

	char name[256];
};

struct vfs_timespec {
	dancy_time_t tv_sec;
	long tv_nsec;
};

struct vfs_dent {
	char name[256];
};

struct vfs_stat {
	uint64_t size;
	struct vfs_timespec access_time;
	struct vfs_timespec creation_time;
	struct vfs_timespec write_time;
	size_t block_size;
};

/*
 * Declarations of default.c
 */
void vfs_default(struct vfs_node *node);

/*
 * Declarations of devfs.c
 */
int devfs_init(void);

/*
 * Declarations of fat_io.c
 */
int fat_io_create(struct vfs_node **new_node, struct vfs_node *dev_node);

/*
 * Declarations of pipe.c
 */
int vfs_pipe(struct vfs_node *nodes[2]);

/*
 * Declarations of pty.c
 */
int pty_init(void);
int pty_create(struct vfs_node *nodes[2], char name[16],
	const struct __dancy_termios *termios_p,
	const struct __dancy_winsize *winsize_p);

/*
 * Declarations of root.c
 */
int vfs_init_root(struct vfs_node **node);

/*
 * Declarations of vfs.c
 */
int vfs_init(void);
void vfs_init_node(struct vfs_node *node, size_t size);

void vfs_lock_tree(void);
void vfs_unlock_tree(void);

int vfs_increment_count(struct vfs_node *node);
int vfs_decrement_count(struct vfs_node *node);

int vfs_mount(const char *name, struct vfs_node *node);
int vfs_open(const char *name, struct vfs_node **node, int type, int mode);
int vfs_realpath(struct vfs_node *node, void *buffer, size_t size);

int vfs_remove(const char *name, int dir);
int vfs_rename(const char *old_name, const char *new_name);

#define vfs_unlink(name) (vfs_remove(name, 0))
#define vfs_rmdir(name) (vfs_remove(name, 1))

#endif
