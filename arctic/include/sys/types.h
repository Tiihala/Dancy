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
 * sys/types.h
 *      Data types
 */

#ifndef ARCTIC_DANCY_SYS_TYPES_H
#define ARCTIC_DANCY_SYS_TYPES_H

#include <limits.h>
#include <stddef.h>

#if UINT_MAX != 4294967295ul
#error "Definition of UINT_MAX is not compatible"
#endif

#ifndef ULLONG_MAX
#error "Definition of ULLONG_MAX is not available"
#endif

#if ULLONG_MAX != 18446744073709551615ull
#error "Definition of ULLONG_MAX is not compatible"
#endif

#ifdef DANCY_32
#define __DANCY_SIZE_MAX UINT_MAX
#endif

#ifdef DANCY_64
#define __DANCY_SIZE_MAX ULLONG_MAX
#endif

#ifndef __DANCY_SIZE_MAX
#error "Define __DANCY_SIZE_MAX"
#endif

#ifndef SIZE_MAX
#define SIZE_MAX __DANCY_SIZE_MAX
#endif

#if SIZE_MAX != __DANCY_SIZE_MAX
#error "Definition of SIZE_MAX is not compatible"
#endif

#if SIZE_MAX == UINT_MAX
typedef int ssize_t;
#elif SIZE_MAX == ULONG_MAX
typedef long ssize_t;
#elif SIZE_MAX == ULLONG_MAX
typedef long long ssize_t;
#else
#error "Type ssize_t could not be defined"
#endif

typedef int mode_t;
typedef int suseconds_t;

typedef long long off_t;
typedef long long id_t;
typedef long long pid_t;

typedef long long clock_t;
typedef long long time_t;

#endif
