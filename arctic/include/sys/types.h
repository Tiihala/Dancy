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

#include <__dancy/core.h>
#include <__dancy/ssize.h>

#ifndef __DANCY_TYPEDEF_MODE_T
#define __DANCY_TYPEDEF_MODE_T
typedef __dancy_mode_t mode_t;
#endif

#ifndef __DANCY_TYPEDEF_SUSECONDS_T
#define __DANCY_TYPEDEF_SUSECONDS_T
typedef __dancy_suseconds_t suseconds_t;
#endif

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

#ifndef __DANCY_TYPEDEF_ID_T
#define __DANCY_TYPEDEF_ID_T
typedef __dancy_id_t id_t;
#endif

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

#ifndef __DANCY_TYPEDEF_CLOCK_T
#define __DANCY_TYPEDEF_CLOCK_T
typedef __dancy_clock_t clock_t;
#endif

#ifndef __DANCY_TYPEDEF_TIME_T
#define __DANCY_TYPEDEF_TIME_T
typedef __dancy_time_t time_t;
#endif

#endif
