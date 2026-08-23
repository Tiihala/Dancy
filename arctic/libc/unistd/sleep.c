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
 * libc/unistd/sleep.c
 *      Sleep for a specified number of seconds
 */

#include <time.h>
#include <unistd.h>

static unsigned int call_sleep(time_t tv_sec, long tv_nsec)
{
	struct timespec request = { tv_sec, tv_nsec };
	struct timespec remain = { 0, 0 };

	int r = clock_nanosleep(CLOCK_MONOTONIC, 0, &request, &remain);

	if (r == 0)
		return 0;

	return (unsigned int)remain.tv_sec + (remain.tv_nsec != 0 ? 1u : 0u);
}

unsigned int sleep(unsigned int seconds)
{
	return call_sleep((time_t)seconds, 0L);
}
