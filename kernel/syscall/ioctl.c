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
 * syscall/ioctl.c
 *      Access device parameters
 */

#include <dancy.h>

static int check_pointer(size_t alignment, size_t size, int rw, long long arg)
{
	addr_t addr;

	if (arg <= 0)
		return DE_ACCESS;
#ifdef DANCY_32
	if (arg > 0xFFFFFFFF)
		return DE_ACCESS;
#endif
	addr = (addr_t)arg;

	if ((addr % (addr_t)alignment) != 0)
		return DE_ACCESS;

	if (rw) {
		if (pg_check_user_write((void *)addr, size))
			return DE_ACCESS;
	} else {
		if (pg_check_user_read((const void *)addr, size))
			return DE_ACCESS;
	}

	return 0;
}

int ioctl_internal(int fd, int request, long long arg, long long *retval)
{
	size_t alignment = sizeof(int);
	size_t size = 0;
	int rw = 0, r = 0;

	*retval = 0;

	switch (request) {
		case __DANCY_IOCTL_TCGETS:
			size = sizeof(struct __dancy_termios), rw = 1;
			break;
		case __DANCY_IOCTL_TCSETS:
			size = sizeof(struct __dancy_termios);
			break;
		case __DANCY_IOCTL_TCSETSW:
			size = sizeof(struct __dancy_termios);
			break;
		case __DANCY_IOCTL_TCSETSF:
			size = sizeof(struct __dancy_termios);
			break;
		case __DANCY_IOCTL_TIOCGWINSZ:
			alignment = sizeof(short);
			size = sizeof(struct __dancy_winsize), rw = 1;
			break;
		case __DANCY_IOCTL_TIOCSWINSZ:
			alignment = sizeof(short);
			size = sizeof(struct __dancy_winsize);
			break;
		case __DANCY_IOCTL_TIOCGPGRP:
			alignment = sizeof(size_t);
			size = sizeof(__dancy_pid_t), rw = 1;
			break;
		case __DANCY_IOCTL_TIOCSPGRP:
			alignment = sizeof(size_t);
			size = sizeof(__dancy_pid_t);
			break;
		case __DANCY_IOCTL_VT_GETSTATE:
			alignment = sizeof(short);
			size = sizeof(struct __dancy_vt_stat), rw = 1;
			break;
		case __DANCY_IOCTL_VT_ACTIVATE:
			break;
		case __DANCY_IOCTL_VT_WAITACTIVE:
			break;
		default:
			r = DE_UNSUPPORTED;
			break;
	}

	if (size != 0)
		r = check_pointer(alignment, size, rw, arg);

	if (r == 0)
		r = file_ioctl(fd, request, arg);

	return r;
}
