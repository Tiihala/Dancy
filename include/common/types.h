/*
 * Copyright (c) 2018, 2019, 2021, 2022 Antti Tiihala
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
 * common/types.h
 *      Header of Dancy Operating System
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <limits.h>
#include <stdint.h>

#include <arctic/include/sys/types.h>

#ifdef DANCY_32
typedef unsigned int addr_t;
typedef unsigned int phys_addr_t;
typedef unsigned int cpu_native_t;
#endif

#ifdef DANCY_64
typedef unsigned long long addr_t;
typedef unsigned long long phys_addr_t;
typedef unsigned long long cpu_native_t;
#endif

typedef long long dancy_time_t;
typedef void *event_t;

#endif
