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
 * include/__dancy/timedef.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_TIMEDEF_H
#define __DANCY_INTERNAL_TIMEDEF_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_CLOCK_T
#define __DANCY_TYPEDEF_CLOCK_T
typedef __dancy_clock_t clock_t;
#endif

#ifndef __DANCY_TYPEDEF_CLOCKID_T
#define __DANCY_TYPEDEF_CLOCKID_T
typedef __dancy_clockid_t clockid_t;
#endif

#ifndef __DANCY_TYPEDEF_TIME_T
#define __DANCY_TYPEDEF_TIME_T
typedef __dancy_time_t time_t;
#endif

#define CLOCKS_PER_SEC              ((clock_t)1000000)

#define CLOCK_REALTIME              0
#define CLOCK_MONOTONIC             1
#define CLOCK_PROCESS_CPUTIME_ID    2
#define CLOCK_THREAD_CPUTIME_ID     3

__Dancy_Header_End

#endif
