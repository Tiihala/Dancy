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
 * libc/unistd/ttyname.c
 *      Find the pathname of a terminal
 */

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

char *ttyname(int fd)
{
	static char name[16];

	if (ttyname_r(fd, &name[0], sizeof(name)))
		return NULL;

	return &name[0];
}

int ttyname_r(int fd, char *name, size_t size)
{
	struct __dancy_tty t;

	memset(&t, 0, sizeof(t));

	if (ioctl(fd, __DANCY_IOCTL_TTY_SYNC, &t) != 0) {
		if (errno != EBADF)
			errno = ENOTTY;
		return errno;
	}

	if (name == NULL || strlen(&t.name[0]) >= size)
		return (errno = ERANGE);

	strcpy(name, &t.name[0]);

	return 0;
}
