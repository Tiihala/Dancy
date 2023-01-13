/*
 * Copyright (c) 2023 Antti Tiihala
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
 * dirent.h
 *      The directory entries
 */

#ifndef __DANCY_DIRENT_H
#define __DANCY_DIRENT_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_INO_T
#define __DANCY_TYPEDEF_INO_T
typedef __dancy_ino_t ino_t;
#endif

struct dirent {
	ino_t d_ino;
	char d_name[256];
};

typedef struct { void *__data; } DIR;

DIR *opendir(const char *name);
DIR *fdopendir(int fd);

int closedir(DIR *dirp);
int dirfd(DIR *dirp);

struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);

__Dancy_Header_End

#endif
