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
 * libc/termios/setspeed.c
 *      Set speed values to the termios structure
 */

#include <errno.h>
#include <termios.h>

static int setspeed(struct termios *termios_p, speed_t speed)
{
	speed_t m1 = 0x100F;
	tcflag_t m2 = (tcflag_t)m1;

	if ((speed & (~m1)) != 0)
		return (errno = EINVAL), -1;

	termios_p->c_cflag &= (~m2);
	termios_p->c_cflag |= ((tcflag_t)speed);

	return 0;
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
	return setspeed(termios_p, speed);
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
	return setspeed(termios_p, speed);
}
