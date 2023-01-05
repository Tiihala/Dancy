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
 * sys/stat.h
 *      Data returned by the stat() function
 */

#ifndef __DANCY_SYS_STAT_H
#define __DANCY_SYS_STAT_H

#include <__dancy/core.h>
#include <__dancy/mode.h>
#include <__dancy/stat.h>

__Dancy_Header_Begin

#define S_IFMT    __DANCY_S_IFMT
#define S_IFIFO   __DANCY_S_IFIFO
#define S_IFCHR   __DANCY_S_IFCHR
#define S_IFDIR   __DANCY_S_IFDIR
#define S_IFBLK   __DANCY_S_IFBLK
#define S_IFREG   __DANCY_S_IFREG
#define S_IFLNK   __DANCY_S_IFLNK
#define S_IFSOCK  __DANCY_S_IFSOCK

#define S_IXOTH   __DANCY_S_IXOTH
#define S_IWOTH   __DANCY_S_IWOTH
#define S_IROTH   __DANCY_S_IROTH
#define S_IRWXO   __DANCY_S_IRWXO

#define S_IXGRP   __DANCY_S_IXGRP
#define S_IWGRP   __DANCY_S_IWGRP
#define S_IRGRP   __DANCY_S_IRGRP
#define S_IRWXG   __DANCY_S_IRWXG

#define S_IXUSR   __DANCY_S_IXUSR
#define S_IWUSR   __DANCY_S_IWUSR
#define S_IRUSR   __DANCY_S_IRUSR
#define S_IRWXU   __DANCY_S_IRWXU

#define S_ISVTX   __DANCY_S_ISVTX
#define S_ISGID   __DANCY_S_ISGID
#define S_ISUID   __DANCY_S_ISUID

#define S_ISFIFO(mode)  (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFIFO)
#define S_ISCHR(mode)   (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFCHR)
#define S_ISDIR(mode)   (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFDIR)
#define S_ISBLK(mode)   (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFBLK)
#define S_ISREG(mode)   (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFREG)
#define S_ISLNK(mode)   (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFLNK)
#define S_ISSOCK(mode)  (((mode) & __DANCY_S_IFMT) == __DANCY_S_IFSOCK)

int stat(const char *path, struct stat *buffer);
int lstat(const char *path, struct stat *buffer);
int fstat(int fd, struct stat *buffer);

__Dancy_Header_End

#endif
