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
 * include/__dancy/syscall.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_SYSCALL_H
#define __DANCY_INTERNAL_SYSCALL_H

#include <__dancy/core.h>

__Dancy_Header_Begin

enum __dancy_syscall_enum__ {
	__dancy_syscall_arg0__ = 0x10000000,

	/*
	 * long long __dancy_syscall_exit(
	 *         int retval);
	 */
	__dancy_syscall_exit,

	/*
	 * long long __dancy_syscall_time(
	 *         void);
	 */
	__dancy_syscall_time,

	/*
	 * long long __dancy_syscall_execve(
	 *         const char *path,
	 *         char *const argv[],
	 *         char *const envp[]);
	 */
	__dancy_syscall_execve,

	__dancy_syscall_spawn,

	/*
	 * long long __dancy_syscall_wait(
	 *         int *status);
	 */
	__dancy_syscall_wait,

	/*
	 * long long __dancy_syscall_waitpid(
	 *         __dancy_pid_t pid,
	 *         int *status,
	 *         int options);
	 */
	__dancy_syscall_waitpid,

	/*
	 * long long __dancy_syscall_open(
	 *         const char *path,
	 *         int flags,
	 *         __dancy_mode_t mode);
	 */
	__dancy_syscall_open,

	/*
	 * long long __dancy_syscall_close(
	 *         int fd);
	 */
	__dancy_syscall_close,

	/*
	 * long long __dancy_syscall_read(
	 *         int fd,
	 *         void *buffer,
	 *         size_t size);
	 */
	__dancy_syscall_read,

	/*
	 * long long __dancy_syscall_write(
	 *         int fd,
	 *         const void *buffer,
	 *         size_t size);
	 */
	__dancy_syscall_write,

	/*
	 * long long __dancy_syscall_pipe(
	 *         int fd[2],
	 *         int flags);
	 */
	__dancy_syscall_pipe,

	/*
	 * long long __dancy_syscall_dup(
	 *         int fd,
	 *         int min_fd,
	 *         int max_fd,
	 *         int flags);
	 */
	__dancy_syscall_dup,

	/*
	 * long long __dancy_syscall_lseek(
	 *         int fd,
	 *         __dancy_off_t offset,
	 *         int whence);
	 */
	__dancy_syscall_lseek,

	/*
	 * long long __dancy_syscall_fcntl(
	 *         int fd,
	 *         int cmd,
	 *         int arg);
	 */
	__dancy_syscall_fcntl,

	__dancy_syscall_argn__
};

long long __dancy_syscall0(int arg0);
long long __dancy_syscall1(int arg0, ...);
long long __dancy_syscall2(int arg0, ...);
long long __dancy_syscall3(int arg0, ...);
long long __dancy_syscall4(int arg0, ...);
long long __dancy_syscall5(int arg0, ...);

__Dancy_Header_End

#endif
