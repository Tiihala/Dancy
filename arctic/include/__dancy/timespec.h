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
 * include/__dancy/timespec.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_TIMESPEC_H
#define __DANCY_INTERNAL_TIMESPEC_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_TIME_T
#define __DANCY_TYPEDEF_TIME_T
typedef __dancy_time_t time_t;
#endif

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

__Dancy_Header_End

#endif
