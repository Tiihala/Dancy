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
 * kernel/syscall.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <common/types.h>

/*
 * Declarations of arg.c
 */
int arg_create(void **arg_state, const void *argv, const void *envp);
int arg_copy(void *arg_state, addr_t *user_sp);
int arg_set_cmdline(struct vfs_node *node, addr_t user_sp);
void arg_delete(void *arg_state);

/*
 * Declarations of errno.c
 */
void errno_internal(int number, char buffer[64], int flags);

/*
 * Declarations of file.c
 */
struct file_table_entry {
	int lock[2];
	int count;
	int flags;

	uint64_t offset;
	struct vfs_node *node;
};

extern int file_table_count;
extern struct file_table_entry *file_table;

int file_init(void);
int file_open(int *fd, const char *name, int flags, mode_t mode);
int file_close(int fd);
int file_read(int fd, size_t *size, void *buffer);
int file_write(int fd, size_t *size, const void *buffer);
int file_lseek(int fd, off_t offset, uint64_t *new_offset, int whence);
int file_fcntl(int fd, int cmd, int arg, int *retval);
int file_dup(int fd, int *new_fd, int min_fd, int max_fd, int flags);
int file_pipe(int fd[2], int flags);
int file_chdir(const char *name);
int file_getcwd(void *buffer, size_t size);
int file_getdents(int fd, void *buffer, size_t size, int *count, int flags);
int file_realpath(const char *name, void *buffer, size_t size);
int file_poll(struct pollfd fds[], int nfds, int timeout, int *retval);
int file_ioctl(int fd, int request, long long arg);

int file_openpty(int fd[2], char name[16],
	const struct __dancy_termios *termios_p,
	const struct __dancy_winsize *winsize_p);

/*
 * Declarations of ioctl.c
 */
int ioctl_internal(int fd, int request, long long arg, long long *retval);

/*
 * Declarations of kill.c
 */
int kill_internal(__dancy_pid_t pid, int sig, int flags);

/*
 * Declarations of misc.c
 */
int getpgid_internal(uint64_t id, uint64_t *id_group);
int getsid_internal(uint64_t id, uint64_t *id_session);

/*
 * Declarations of proc.c
 */
int proclist_internal(size_t *size, __dancy_pid_t *out);
int procinfo_internal(__dancy_pid_t id, int request, size_t *size, void *out);

/*
 * Declarations of reboot.c
 */
int reboot_internal(int request, long long arg);

/*
 * Declarations of sleep.c
 */
int sleep_internal(clockid_t id, int flags,
	const struct timespec *request,  struct timespec *remain);

/*
 * Declarations of spawn.c
 */
int spawn_task(uint64_t *id, struct vfs_node *node,
	void *arg_state, const struct __dancy_spawn_options *options);

/*
 * Declarations of syscall.c
 */
int syscall_init(void);
long long syscall_handler(int arg0, ...);

#endif
