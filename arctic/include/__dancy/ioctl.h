/*
 * Copyright (c) 2023, 2024 Antti Tiihala
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
 * include/__dancy/ioctl.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_IOCTL_H
#define __DANCY_INTERNAL_IOCTL_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define __DANCY_IOCTL(num, arg) \
	(0x7D000000 | (((num) & 0xFFFF) << 8) | ((arg) & 0xFF))

#define __DANCY_IOCTL_ARG_NULL      (0x00)
#define __DANCY_IOCTL_ARG_INT       (0x10)
#define __DANCY_IOCTL_ARG_LONG      (0x20)
#define __DANCY_IOCTL_ARG_LONG_LONG (0x40)
#define __DANCY_IOCTL_ARG_POINTER   (0x80)
#define __DANCY_IOCTL_ARG_MASK      (0xF0)

struct __dancy_winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

struct __dancy_vt_stat {
	unsigned short v_active;
	unsigned short v_signal;
	unsigned short v_state;
};

#define __DANCY_IOCTL_TCGETS \
	__DANCY_IOCTL(0x1000, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_TCSETS \
	__DANCY_IOCTL(0x1001, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_TCSETSW \
	__DANCY_IOCTL(0x1002, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_TCSETSF \
	__DANCY_IOCTL(0x1003, __DANCY_IOCTL_ARG_POINTER)

#define __DANCY_IOCTL_TIOCGWINSZ \
	__DANCY_IOCTL(0x1010, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_TIOCSWINSZ \
	__DANCY_IOCTL(0x1011, __DANCY_IOCTL_ARG_POINTER)

#define __DANCY_IOCTL_TIOCGPGRP \
	__DANCY_IOCTL(0x1020, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_TIOCSPGRP \
	__DANCY_IOCTL(0x1021, __DANCY_IOCTL_ARG_POINTER)

#define __DANCY_IOCTL_VT_GETSTATE \
	__DANCY_IOCTL(0x1030, __DANCY_IOCTL_ARG_POINTER)
#define __DANCY_IOCTL_VT_ACTIVATE \
	__DANCY_IOCTL(0x1031, __DANCY_IOCTL_ARG_INT)
#define __DANCY_IOCTL_VT_WAITACTIVE \
	__DANCY_IOCTL(0x1032, __DANCY_IOCTL_ARG_INT)

__Dancy_Header_End

#endif
