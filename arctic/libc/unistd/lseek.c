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
 * libc/unistd/lseek.c
 *      Move the file offset
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <unistd.h>

#if __DANCY_SIZE_MAX == 4294967295u

off_t lseek(int fd, off_t offset, int whence)
{
	size_t o1 = (size_t)((offset >>  0) & 0xFFFFFFFF);
	size_t o2 = (size_t)((offset >> 32) & 0xFFFFFFFF);
	long long r;

	r = __dancy_syscall4(__dancy_syscall_lseek, fd, o1, o2, whence);

	if (r < 0)
		errno = -((int)r), r = -1;

	return (off_t)r;
}

#endif

#if __DANCY_SIZE_MAX == 18446744073709551615ull

off_t lseek(int fd, off_t offset, int whence)
{
	long long r;

	r = __dancy_syscall3(__dancy_syscall_lseek, fd, offset, whence);

	if (r < 0)
		errno = -((int)r), r = -1;

	return (off_t)r;
}

#endif
