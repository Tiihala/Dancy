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
 * unistd.h
 *      Standard symbolic constants and types
 */

#ifndef ARCTIC_DANCY_UNISTD_H
#define ARCTIC_DANCY_UNISTD_H

#include <__dancy/core.h>
#include <__dancy/seek.h>
#include <__dancy/ssize.h>

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

#undef STDIN_FILENO
#undef STDOUT_FILENO
#undef STDERR_FILENO

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int close(int fd);

int fsync(int fd);
off_t lseek(int fd, off_t offset, int whence);

ssize_t read(int fd, void *buffer, size_t size);
ssize_t write(int fd, const void *buffer, size_t size);

int chdir(const char *path);
int rmdir(const char *path);
int unlink(const char *path);

int dup(int fd);
int dup2(int fd, int new_fd);
int pipe(int fd[2]);

int ftruncate(int fd, off_t length);
int truncate(const char *path, off_t length);

pid_t getpid(void);
void _exit(int status);

#endif
