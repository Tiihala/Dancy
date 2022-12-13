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
 * libc/sys/waitpid.c
 *      Wait a child process
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
	long long r;

#if __DANCY_SIZE_MAX == 18446744073709551615ull
	r = __dancy_syscall3(__dancy_syscall_waitpid, pid, status, options);
#else
	r = __dancy_syscall4(__dancy_syscall_waitpid, pid, status, options);
#endif
	if (r < 0)
		errno = -((int)r), r = -1;

	return (pid_t)r;
}