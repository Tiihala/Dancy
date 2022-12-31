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
 * include/__dancy/core.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_CORE_H
#define __DANCY_INTERNAL_CORE_H

#include <stddef.h>

#ifndef __Dancy_Header_Begin
#define __Dancy_Header_Begin
#endif

__Dancy_Header_Begin

#if defined(__CHAR_BIT__) && __CHAR_BIT__ != 8
#error "Unsupported __CHAR_BIT__"
#endif

#if defined(__INT_MAX__) && __INT_MAX__ != 0x7FFFFFFF
#error "Unsupported __INT_MAX__"
#endif

#if !defined(__DANCY_SIZE_MAX) && defined(__DANCY_32)
#define __DANCY_SIZE_MAX 4294967295u
#endif

#if !defined(__DANCY_SIZE_MAX) && defined(__DANCY_64)
#define __DANCY_SIZE_MAX 18446744073709551615ull
#endif

#if !defined(__DANCY_SIZE_MAX) && defined(__SIZE_MAX__)
#define __DANCY_SIZE_MAX __SIZE_MAX__
#endif

#if !defined(__DANCY_SIZE_MAX)
#error "Define __DANCY_SIZE_MAX"
#endif

#if defined(__SIZE_MAX__) && __SIZE_MAX__ != __DANCY_SIZE_MAX
#error "Unsupported __SIZE_MAX__ or __DANCY_SIZE_MAX"
#endif

#define __DANCY_OPEN_MAX 64

typedef int __dancy_mode_t;
typedef int __dancy_suseconds_t;
typedef int __dancy_sig_atomic_t;
typedef int __dancy_clockid_t;
typedef int __dancy_nlink_t;
typedef int __dancy_timer_t;
typedef int __dancy_blksize_t;

typedef int __dancy_uid_t;
typedef int __dancy_gid_t;

typedef long long __dancy_dev_t;
typedef long long __dancy_off_t;
typedef long long __dancy_id_t;
typedef long long __dancy_pid_t;

typedef long long __dancy_clock_t;
typedef long long __dancy_time_t;
typedef long long __dancy_blkcnt_t;

typedef unsigned long long __dancy_ino_t;
typedef unsigned long long __dancy_fsblkcnt_t;
typedef unsigned long long __dancy_fsfilcnt_t;
typedef unsigned long long __dancy_sigset_t;

void __dancy_assert(const char *expression, const char *name, int line);
int *__dancy_errno(void);

#ifndef __Dancy_Header_End
#define __Dancy_Header_End
#endif

__Dancy_Header_End

#endif
