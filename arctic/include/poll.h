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
 * poll.h
 *      Definitions for the poll function
 */

#ifndef __DANCY_POLL_H
#define __DANCY_POLL_H

#include <__dancy/core.h>
#include <__dancy/pollfd.h>
#include <__dancy/timespec.h>

__Dancy_Header_Begin

#define POLLIN      (0x0001)
#define POLLPRI     (0x0002)
#define POLLOUT     (0x0004)
#define POLLERR     (0x0008)
#define POLLHUP     (0x0010)
#define POLLNVAL    (0x0020)

#define POLLRDNORM  (0x0040)
#define POLLRDBAND  (0x0080)
#define POLLWRNORM  (0x0100)
#define POLLWRBAND  (0x0200)

#ifndef __DANCY_TYPEDEF_SIGSET_T
#define __DANCY_TYPEDEF_SIGSET_T
typedef __dancy_sigset_t sigset_t;
#endif

typedef unsigned int nfds_t;

int poll(struct pollfd fds[], nfds_t nfds, int timeout);

int ppoll(struct pollfd fds[], nfds_t nfds,
	const struct timespec *timeout,
	const sigset_t *sigmask);

__Dancy_Header_End

#endif
