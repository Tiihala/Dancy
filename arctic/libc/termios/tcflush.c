/*
 * Copyright (c) 2026 Antti Tiihala
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
 * libc/termios/tcflush.c
 *      Flush non-transmitted output data, non-read input data, or both
 */

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>

int tcflush(int fd, int queue_selector)
{
	if (queue_selector == TCIFLUSH)
		return ioctl(fd, __DANCY_IOCTL_TCIFLUSH);

	if (queue_selector == TCOFLUSH)
		return ioctl(fd, __DANCY_IOCTL_TCOFLUSH);

	if (queue_selector == TCIOFLUSH)
		return ioctl(fd, __DANCY_IOCTL_TCIOFLUSH);

	return (errno = EINVAL), -1;
}
