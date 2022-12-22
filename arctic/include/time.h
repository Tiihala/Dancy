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
 * time.h
 *      The C Standard Library
 */

#ifndef __DANCY_TIME_H
#define __DANCY_TIME_H

#include <__dancy/core.h>
#include <__dancy/timespec.h>
#include <__dancy/tm.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_CLOCK_T
#define __DANCY_TYPEDEF_CLOCK_T
typedef __dancy_clock_t clock_t;
#endif

time_t time(time_t *tloc);

__Dancy_Header_End

#endif
