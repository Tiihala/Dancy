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
 * libc/sys/fdset.c
 *      Synchronous I/O Multiplexing
 */

#include <string.h>
#include <sys/select.h>

void FD_CLR(int fd, fd_set *set)
{
	if (fd >= 0 && fd < FD_SETSIZE) {
		unsigned int u = (unsigned int)(1 << (fd & 7));
		set->_u._set[fd >> 3] &= (unsigned char)(~u);
	}
}

int FD_ISSET(int fd, fd_set *set)
{
	if (fd >= 0 && fd < FD_SETSIZE) {
		unsigned int u = (unsigned int)(1 << (fd & 7));
		return (set->_u._set[fd >> 3] & (unsigned char)u) != 0;
	}
	return 0;
}

void FD_SET(int fd, fd_set *set)
{
	if (fd >= 0 && fd < FD_SETSIZE) {
		unsigned int u = (unsigned int)(1 << (fd & 7));
		set->_u._set[fd >> 3] |= (unsigned char)u;
	}
}

void FD_ZERO(fd_set *set)
{
	memset(set, 0, sizeof(*set));
}
