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
 * sys/ioctl.h
 *      Access device parameters
 */

#ifndef __DANCY_SYS_IOCTL_H
#define __DANCY_SYS_IOCTL_H

#include <__dancy/core.h>
#include <__dancy/ioctl.h>

__Dancy_Header_Begin

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

struct vt_stat {
	unsigned short v_active;
	unsigned short v_signal;
	unsigned short v_state;
};

#define TCGETS                  __DANCY_IOCTL_TCGETS
#define TCSETS                  __DANCY_IOCTL_TCSETS
#define TCSETSW                 __DANCY_IOCTL_TCSETSW
#define TCSETSF                 __DANCY_IOCTL_TCSETSF

#define TIOCGWINSZ              __DANCY_IOCTL_TIOCGWINSZ
#define TIOCSWINSZ              __DANCY_IOCTL_TIOCSWINSZ

#define TIOCGPGRP               __DANCY_IOCTL_TIOCGPGRP
#define TIOCSPGRP               __DANCY_IOCTL_TIOCSPGRP

#define VT_GETSTATE             __DANCY_IOCTL_VT_GETSTATE
#define VT_ACTIVATE             __DANCY_IOCTL_VT_ACTIVATE
#define VT_WAITACTIVE           __DANCY_IOCTL_VT_WAITACTIVE

int ioctl(int fd, unsigned long request, ...);

__Dancy_Header_End

#endif
