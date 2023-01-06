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
 * include/__dancy/mman.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_MMAN_H
#define __DANCY_INTERNAL_MMAN_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define __DANCY_PROT_NONE       (0x0000)
#define __DANCY_PROT_READ       (0x0001)
#define __DANCY_PROT_WRITE      (0x0002)
#define __DANCY_PROT_EXEC       (0x0004)

#define __DANCY_MAP_SHARED      (0x0001)
#define __DANCY_MAP_PRIVATE     (0x0002)
#define __DANCY_MAP_FIXED       (0x0010)

#define __DANCY_MAP_ANON        (0x0020)
#define __DANCY_MAP_ANONYMOUS   (0x0020)

#define __DANCY_MS_ASYNC        (0x0001)
#define __DANCY_MS_INVALIDATE   (0x0002)
#define __DANCY_MS_SYNC         (0x0004)

struct __dancy_mmap_options {
	void *__address;
	size_t __size;
	int __state;
	int __prot;
	int __flags;
	int __fd;
	long long __offset;
};

__Dancy_Header_End

#endif
