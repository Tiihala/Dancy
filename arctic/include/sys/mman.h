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
 * sys/mman.h
 *      Memory management declarations
 */

#ifndef __DANCY_SYS_MMAN_H
#define __DANCY_SYS_MMAN_H

#include <__dancy/core.h>
#include <__dancy/mman.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_MODE_T
#define __DANCY_TYPEDEF_MODE_T
typedef __dancy_mode_t mode_t;
#endif

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

#define PROT_NONE       __DANCY_PROT_NONE
#define PROT_READ       __DANCY_PROT_READ
#define PROT_WRITE      __DANCY_PROT_WRITE
#define PROT_EXEC       __DANCY_PROT_EXEC

#define MAP_SHARED      __DANCY_MAP_SHARED
#define MAP_PRIVATE     __DANCY_MAP_PRIVATE
#define MAP_FIXED       __DANCY_MAP_FIXED

#define MAP_ANON        __DANCY_MAP_ANON
#define MAP_ANONYMOUS   __DANCY_MAP_ANONYMOUS

#define MS_ASYNC        __DANCY_MS_ASYNC
#define MS_INVALIDATE   __DANCY_MS_INVALIDATE
#define MS_SYNC         __DANCY_MS_SYNC

#define MAP_FAILED      ((void *)(-1))

void *mmap(void *address, size_t size,
	int prot, int flags, int fd, off_t offset);

int munmap(void *address, size_t size);

int mprotect(void *address, size_t size, int prot);
int msync(void *address, size_t size, int flags);

__Dancy_Header_End

#endif
