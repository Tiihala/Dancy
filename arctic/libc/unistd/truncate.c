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
 * libc/unistd/truncate.c
 *      Truncate a file to a specified length
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <unistd.h>

static int t(int fd, const char *path, off_t size)
{
	int r;

	r = (int)__dancy_syscall3e(__dancy_syscall_truncate, fd, path, size);

	if (r < 0)
		return (errno = -r), -1;

	return 0;
}

int ftruncate(int fd, off_t length)
{
	return t(fd, NULL, length);
}

int truncate(const char *path, off_t length)
{
	return t(-1, path, length);
}
