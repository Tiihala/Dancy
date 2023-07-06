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
 * libc/sys/ioctl.c
 *      Access device parameters
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/ioctl.h>

static long long ioctl_arg_int(va_list va);
static long long ioctl_arg_long(va_list va);
static long long ioctl_arg_long_long(va_list va);
static long long ioctl_arg_pointer(va_list va);

int ioctl(int fd, unsigned long request, ...)
{
	long long arg = 0;
	int r = -1;

	va_list va;
	va_start(va, request);

	if (request <= (unsigned long)(INT_MAX))
		r = (int)request;

	switch (r) {
		case __DANCY_IOCTL_TEST_A:
			arg = ioctl_arg_int(va);
			break;
		case __DANCY_IOCTL_TEST_B:
			arg = ioctl_arg_long(va);
			break;
		case __DANCY_IOCTL_TEST_C:
			arg = ioctl_arg_long_long(va);
			break;
		case __DANCY_IOCTL_TEST_D:
			arg = ioctl_arg_pointer(va);
			break;
		default:
			break;
	}

	va_end(va);

	if (r < 0)
		return (errno = EINVAL), -1;

	r = (int)__dancy_syscall3e(__dancy_syscall_ioctl, fd, r, arg);

	if (r < 0)
		errno = -r, r = -1;

	return r;
}

static long long ioctl_arg_int(va_list va)
{
	int r = va_arg(va, int);
	return (long long)r;
}

static long long ioctl_arg_long(va_list va)
{
	long r = va_arg(va, long);
	return (long long)r;
}

static long long ioctl_arg_long_long(va_list va)
{
	long long r = va_arg(va, long long);
	return r;
}

static long long ioctl_arg_pointer(va_list va)
{
	void *r = va_arg(va, void *);

	if ((unsigned long long)r > (unsigned long long)(LLONG_MAX))
		r = NULL;

	return (long long)((unsigned long long)r);
}
