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
 * libc/sys/mmap.c
 *      Map pages of memory
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

void *mmap(void *address, size_t size,
	int prot, int flags, int fd, off_t offset)
{
	struct __dancy_mmap_options options;
	long long r;

	memset(&options, 0, sizeof(options));

	options.__address = address;
	options.__size = size;
	options.__prot = prot;
	options.__flags = flags;
	options.__fd = fd;
	options.__offset = offset;

	r = __dancy_syscall3(__dancy_syscall_mmap, address, size, &options);

	if (r < 0)
		return (errno = -((int)r)), MAP_FAILED;

	return (void *)((unsigned long long)r);
}
