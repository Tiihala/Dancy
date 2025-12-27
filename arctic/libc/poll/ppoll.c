/*
 * Copyright (c) 2025 Antti Tiihala
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
 * libc/poll/ppoll.c
 *      Input/Output Multiplexing
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>

int ppoll(struct pollfd fds[], nfds_t nfds,
	const struct timespec *timeout,
	const sigset_t *sigmask)
{
	int r, milliseconds = -1;

	if (timeout) {
		unsigned long long u = ULLONG_MAX;
		unsigned long long m = 0x0000FFFFFFFFFFFFull;

		if (timeout->tv_sec < 0 || timeout->tv_nsec < 0)
			return (errno = EINVAL), -1;

		if ((unsigned long long)timeout->tv_sec < m) {
			u = (unsigned long long)(timeout->tv_sec * 1000);
			u += (unsigned long long)(timeout->tv_nsec / 1000000);
		}

		if (u > (unsigned long long)(INT_MAX))
			return (errno = EINVAL), -1;

		milliseconds = (int)u;
	}

	r = (int)__dancy_syscall4(__dancy_syscall_poll,
		fds, nfds, milliseconds, sigmask);

	if (r < 0)
		errno = -r, r = -1;

	return r;
}
