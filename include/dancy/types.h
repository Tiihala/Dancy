/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
 * dancy/types.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_TYPES_H
#define DANCY_TYPES_H

#include <dancy/limits.h>
#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#if defined (DANCY_32)

typedef unsigned int addr_t;
typedef unsigned int phys_addr_t;
#define DANCY_SIZE_MAX (4294967295ul)

#elif defined (DANCY_64)

typedef unsigned long long addr_t;
typedef unsigned long long phys_addr_t;
#define DANCY_SIZE_MAX (18446744073709551615ull)

#endif

#if !defined (SIZE_MAX)
#define SIZE_MAX DANCY_SIZE_MAX
#endif

#if SIZE_MAX != DANCY_SIZE_MAX
#error Definition of SIZE_MAX is not compatible
#endif

#endif
