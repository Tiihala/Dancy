/*
 * Copyright (c) 2022, 2023, 2024 Antti Tiihala
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
	 *         clockid_t id,
	 *         struct timespec *tp);
	 */
	__dancy_syscall_time,

	/*
	 * long long __dancy_syscall_execve(
	 *         const char *path,
	 *         char *const argv[],
	 *         char *const envp[]);
	 */
	__dancy_syscall_execve,

	/*
	 * long long __dancy_syscall_spawn(
	 *         const char *path,
	 *         char *const argv[],
	 *         char *const envp[],
	 *         const void *options);
	 */
	__dancy_syscall_spawn,

	/*
	 * long long __dancy_syscall_wait(
	 *         int *status);
	 */
	__dancy_syscall_wait,

	/*
	 * long long __dancy_syscall_waitpid(
	 *         pid_t pid,
	 *         int *status,
	 *         int options);
	 */
	__dancy_syscall_waitpid,

	/*
	 * long long __dancy_syscall_open(
	 *         const char *path,
	 *         int flags,
	 *         mode_t mode);
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
	 *         off_t offset,
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

	/*
	 * long long __dancy_syscall_getcwd(
	 *         void *buffer,
	 *         size_t size);
	 */
	__dancy_syscall_getcwd,

	/*
	 * long long __dancy_syscall_getdents(
	 *         int fd,
	 *         void *buffer,
	 *         size_t size,
	 *         int count,
	 *         int flags);
	 */
	__dancy_syscall_getdents,

	/*
	 * long long __dancy_syscall_chdir(
	 *         const char *path);
	 */
	__dancy_syscall_chdir,

	/*
	 * long long __dancy_syscall_rmdir(
	 *         const char *path);
	 */
	__dancy_syscall_rmdir,

	/*
	 * long long __dancy_syscall_unlink(
	 *         const char *path);
	 */
	__dancy_syscall_unlink,

	/*
	 * long long __dancy_syscall_rename(
	 *         const char *old_path,
	 *         const char *new_path);
	 */
	__dancy_syscall_rename,

	/*
	 * long long __dancy_syscall_stat(
	 *         int fd,
	 *         const char *path,
	 *         struct stat *buffer,
	 *         int flags);
	 */
	__dancy_syscall_stat,

	/*
	 * long long __dancy_syscall_sleep(
	 *         clockid_t id,
	 *         int flags,
	 *         const struct timespec *request,
	 *         struct timespec *remain);
	 */
	__dancy_syscall_sleep,

	/*
	 * long long __dancy_syscall_mmap(
	 *         void *address,
	 *         size_t size,
	 *         const void *options);
	 */
	__dancy_syscall_mmap,

	/*
	 * long long __dancy_syscall_munmap(
	 *         void *address,
	 *         size_t size);
	 */
	__dancy_syscall_munmap,

	/*
	 * long long __dancy_syscall_mprotect(
	 *         void *address,
	 *         size_t size,
	 *         int prot);
	 */
	__dancy_syscall_mprotect,

	/*
	 * long long __dancy_syscall_msync(
	 *         void *address,
	 *         size_t size,
	 *         int flags);
	 */
	__dancy_syscall_msync,

	/*
	 * long long __dancy_syscall_getpid(void);
	 */
	__dancy_syscall_getpid,

	/*
	 * long long __dancy_syscall_getppid(void);
	 */
	__dancy_syscall_getppid,

	/*
	 * long long __dancy_syscall_realpath(
	 *         const char *path,
	 *         void *buffer,
	 *         size_t size);
	 */
	__dancy_syscall_realpath,

	/*
	 * long long __dancy_syscall_kill(
	 *         pid_t pid,
	 *         int sig,
	 *         int flags);
	 */
	__dancy_syscall_kill,

	/*
	 * long long __dancy_syscall_poll(
	 *         struct pollfd fds[],
	 *         nfds_t nfds,
	 *         int timeout);
	 */
	__dancy_syscall_poll,

	/*
	 * long long __dancy_syscall_ioctl(
	 *         int fd,
	 *         int request,
	 *         long long arg);
	 */
	__dancy_syscall_ioctl,

	/*
	 * long long __dancy_syscall_getpgid(
	 *         pid_t pid);
	 */
	__dancy_syscall_getpgid,

	/*
	 * long long __dancy_syscall_getsid(
	 *         pid_t pid);
	 */
	__dancy_syscall_getsid,

	/*
	 * long long __dancy_syscall_openpty(
	 *         int fd[2],
	 *         char *name,
	 *         const struct termios *termios_p,
	 *         const struct winsize *winsize_p);
	 */
	__dancy_syscall_openpty,

	/*
	 * long long __dancy_syscall_memusage(
	 *         int flags);
	 */
	__dancy_syscall_memusage,

	/*
	 * long long __dancy_syscall_reboot(
	 *         int request,
	 *         long long arg);
	 */
	__dancy_syscall_reboot,

	/*
	 * long long __dancy_syscall_sigpending(
	 *         sigset_t *out);
	 */
	__dancy_syscall_sigpending,

	/*
	 * long long __dancy_syscall_sigprocmask(
	 *         int how,
	 *         const sigset_t *set,
	 *         sigset_t *out);
	 */
	__dancy_syscall_sigprocmask,

	__dancy_syscall_argn__
};

long long __dancy_syscall0(int arg0);
long long __dancy_syscall1(int arg0, ...);
long long __dancy_syscall2(int arg0, ...);
long long __dancy_syscall3(int arg0, ...);
long long __dancy_syscall4(int arg0, ...);
long long __dancy_syscall5(int arg0, ...);

#if __DANCY_SIZE_MAX == 18446744073709551615ull

#define __dancy_syscall1e(...) __dancy_syscall1(__VA_ARGS__)
#define __dancy_syscall2e(...) __dancy_syscall2(__VA_ARGS__)
#define __dancy_syscall3e(...) __dancy_syscall3(__VA_ARGS__)
#define __dancy_syscall4e(...) __dancy_syscall4(__VA_ARGS__)

#define __dancy_syscall2p(...) __dancy_syscall2(__VA_ARGS__)
#define __dancy_syscall3p(...) __dancy_syscall3(__VA_ARGS__)

#else

#define __dancy_syscall1e(...) __dancy_syscall2(__VA_ARGS__)
#define __dancy_syscall2e(...) __dancy_syscall3(__VA_ARGS__)
#define __dancy_syscall3e(...) __dancy_syscall4(__VA_ARGS__)
#define __dancy_syscall4e(...) __dancy_syscall5(__VA_ARGS__)

#define __dancy_syscall2p(...) __dancy_syscall4(__VA_ARGS__)
#define __dancy_syscall3p(...) __dancy_syscall5(__VA_ARGS__)

#endif

__Dancy_Header_End

#endif
