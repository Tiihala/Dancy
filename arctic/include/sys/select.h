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
 * sys/select.h
 *      Synchronous I/O Multiplexing
 */

#ifndef __DANCY_SYS_SELECT_H
#define __DANCY_SYS_SELECT_H

#include <__dancy/core.h>
#include <__dancy/timespec.h>
#include <__dancy/timeval.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_SIGSET_T
#define __DANCY_TYPEDEF_SIGSET_T
typedef __dancy_sigset_t sigset_t;
#endif

#define FD_SETSIZE 1024

struct __dancy_fd_set {
	union {
		unsigned long __align;
		unsigned char __set[FD_SETSIZE / 8];
	} __u;
};

typedef struct __dancy_fd_set fd_set;

void FD_CLR(int fd, fd_set *set);
int FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(int fd, fd_set *set);

int select(int nfds,
	fd_set *readfds, fd_set *writefds, fd_set *errorfds,
	struct timeval *timeout);

int pselect(int nfds,
	fd_set *readfds, fd_set *writefds, fd_set *errorfds,
	const struct timespec *timeout, const sigset_t *sigmask);

__Dancy_Header_End

#endif
