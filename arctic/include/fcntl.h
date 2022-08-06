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
 * fcntl.h
 *      File control options
 */

#ifndef ARCTIC_DANCY_FCNTL_H
#define ARCTIC_DANCY_FCNTL_H

#include <__dancy/core.h>
#include <__dancy/seek.h>

#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_ACCMODE           0x03

#define O_CLOEXEC           0x0010
#define O_CREAT             0x0020
#define O_DIRECTORY         0x0040
#define O_EXCL              0x0080
#define O_NOCTTY            0x0100
#define O_NOFOLLOW          0x0200
#define O_TRUNC             0x0400
#define O_TTY_INIT          0x0800

#define O_APPEND            0x1000
#define O_NONBLOCK          0x2000
#define O_SYNC              0x4000

#define F_DUPFD             0
#define F_GETFD             1
#define F_SETFD             2
#define F_GETFL             3
#define F_SETFL             4

#define FD_CLOEXEC          0x0001

#ifndef __DANCY_TYPEDEF_MODE_T
#define __DANCY_TYPEDEF_MODE_T
typedef __dancy_mode_t mode_t;
#endif

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

int open(const char *name, int flags, ...);
int fcntl(int fd, int cmd, ...);

#endif
