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
 * libc/sys/select.c
 *      Synchronous I/O Multiplexing
 */

#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <sys/select.h>

int select(int nfds,
	fd_set *readfds, fd_set *writefds, fd_set *errorfds,
	struct timeval *timeout)
{
	int timeout_ms = -1;
	int i, r = 0;

	nfds_t poll_nfds = 0;
	struct pollfd poll_fds[FD_SETSIZE];

	if (nfds < 0 || nfds > FD_SETSIZE)
		return (errno = EINVAL), -1;

	if (timeout != NULL) {
		long long t = -1;

		if (timeout->tv_sec < 0)
			return (errno = EINVAL), -1;

		if (timeout->tv_usec < 0 || timeout->tv_usec > 999999)
			return (errno = EINVAL), -1;

		if (timeout->tv_sec < 0x7FFFFFFF) {
			t = (long long)timeout->tv_sec * 1000LL;
			t += (long long)((int)timeout->tv_usec / 1000);

			if (((int)timeout->tv_usec % 1000) != 0)
				t += 1LL;
		}

		if (t >= 0 && t <= (long long)(INT_MAX))
			timeout_ms = (int)t;
	}

	for (i = 0; i < nfds; i++) {
		struct pollfd *f = &poll_fds[poll_nfds];

		f->fd = i;
		f->events = 0;
		f->revents = 0;

		if (readfds && FD_ISSET(i, readfds))
			f->events |= (POLLIN | POLLRDNORM);

		if (writefds && FD_ISSET(i, writefds))
			f->events |= (POLLOUT | POLLWRNORM);

		if (errorfds && FD_ISSET(i, errorfds))
			f->events |= (POLLERR);

		if (f->events != 0)
			poll_nfds += 1;
	}

	if (poll_nfds != 0 && poll(poll_fds, poll_nfds, timeout_ms) < 0)
		return -1;

	if (readfds)
		FD_ZERO(readfds);
	if (writefds)
		FD_ZERO(writefds);
	if (errorfds)
		FD_ZERO(errorfds);

	for (i = 0; i < (int)poll_nfds; i++) {
		struct pollfd *f = &poll_fds[i];

		if ((f->revents & (POLLIN | POLLRDNORM)) != 0)
			FD_SET(f->fd, readfds), r += 1;

		if ((f->revents & (POLLOUT | POLLWRNORM)) != 0)
			FD_SET(f->fd, writefds), r += 1;

		if ((f->revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
			FD_SET(f->fd, errorfds), r += 1;
	}

	return r;
}
