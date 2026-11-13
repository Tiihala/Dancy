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
 * libc/sys/time.c
 *      Time functions
 */

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

int gettimeofday(struct timeval *t, void *z)
{
	struct timespec ts;

	memset(&ts, 0, sizeof(ts));

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
		return -1;

	if (t != NULL) {
		memset(t, 0, sizeof(*t));

		t->tv_sec = ts.tv_sec;
		t->tv_usec = (suseconds_t)(ts.tv_nsec / 1000);
	}

	return (void)z, 0;
}

int settimeofday(const struct timeval *t, const void *z)
{
	(void)t, (void)z;
	return (errno = EPERM), -1;
}
