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
 * libc/fcntl/open.c
 *      Establish the connection between a file and a file descriptor
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

int open(const char *path, int flags, ...)
{
	mode_t mode = 0;
	int r;

	if ((flags & O_CREAT) != 0) {
		va_list va;
		va_start(va, flags);
		mode = va_arg(va, mode_t);
		va_end(va);
	}

	r = (int)__dancy_syscall3(__dancy_syscall_open, path, flags, mode);

	if (r < 0)
		errno = -r, r = -1;

	return r;
}
