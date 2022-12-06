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
 * include/__dancy/ssize.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_SSIZE_H
#define __DANCY_INTERNAL_SSIZE_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#undef __DANCY_SSIZE_TYPE

#if __DANCY_SIZE_MAX == 4294967295u
#define __DANCY_SSIZE_TYPE int

#elif defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 8
#define __DANCY_SSIZE_TYPE long

#elif defined(__LONG_WIDTH__) && __LONG_WIDTH__ == 64
#define __DANCY_SSIZE_TYPE long

#elif __DANCY_SIZE_MAX == 18446744073709551615ull
#define __DANCY_SSIZE_TYPE long long

#endif

#if !defined(__DANCY_SSIZE_TYPE)
#error "Type ssize_t could not be defined"
#endif

typedef __DANCY_SSIZE_TYPE ssize_t;

__Dancy_Header_End

#endif
