/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/termios/setattr.c
 *      Set the terminal parameters
 */

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
	if (optional_actions == TCSANOW)
		return ioctl(fd, TCSETS, termios_p);

	if (optional_actions == TCSADRAIN)
		return ioctl(fd, TCSETSW, termios_p);

	if (optional_actions == TCSAFLUSH)
		return ioctl(fd, TCSETSF, termios_p);

	return (errno = EINVAL), -1;
}
