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
 * syscall/sleep.c
 *      Interruptible sleep function
 */

#include <dancy.h>

static int f(uint64_t *data)
{
	uint64_t t = timer_read();

	if (t > data[0])
		return 0;

	return ((t - data[1]) < 100);
}

static int s(uint64_t start_ms, uint64_t request_ms, struct timespec *remain)
{
	uint64_t d0 = start_ms + request_ms;
	uint64_t d1 = 0;
	int r = 0;

	if (d0 < start_ms)
		d0 = (uint64_t)(ULLONG_MAX);

	while (d0 > (d1 = timer_read())) {
		if (task_current()->asm_data3 != 0) {
			r = DE_INTERRUPT;
			break;
		}

		task_write_event(f, d0, d1);

		do {
			task_yield();
		} while (task_read_event());
	}

	if (r == DE_INTERRUPT && remain != NULL) {
		uint64_t ms64 = d0 - d1;
		uint32_t ms32 = (uint32_t)ms64;

		memset(remain, 0, sizeof(*remain));

		remain->tv_sec = (time_t)(ms64 / 1000);
		remain->tv_nsec = (long)(ms32 % 1000) * 1000000L;
	}

	return r;
}

int sleep_internal(clockid_t id, int flags,
	const struct timespec *request,  struct timespec *remain)
{
	uint64_t start_ms = timer_read();
	uint64_t request_ms;

	if (id != CLOCK_REALTIME && id != CLOCK_MONOTONIC)
		return DE_UNSUPPORTED;

	if (flags != 0 && flags != TIMER_ABSTIME)
		return DE_UNSUPPORTED;

	if (request->tv_sec < 0 || request->tv_nsec < 0)
		return DE_ARGUMENT;

	if ((unsigned long long)request->tv_sec < 0x0000FFFFFFFFFFFFull) {
		request_ms = (uint64_t)(request->tv_sec * 1000);
		request_ms += (uint64_t)(request->tv_nsec / 1000000);
	} else {
		request_ms = (uint64_t)(ULLONG_MAX);
	}

	if (request_ms < 2)
		request_ms = 2;

	if (flags == 0)
		return s(start_ms, request_ms, remain);

	while ((flags & TIMER_ABSTIME) != 0) {
		uint64_t ms64;
		uint32_t ms32;
		struct timespec t;
		time_t d;

		if (id == CLOCK_REALTIME)
			ms64 = (uint64_t)epoch_read_ms();
		else
			ms64 = (uint64_t)timer_read();

		ms32 = (uint32_t)ms64;
		memset(&t, 0, sizeof(t));

		t.tv_sec = (time_t)(ms64 / 1000);
		t.tv_nsec = (long)(ms32 % 1000) * 1000000L;

		if (request->tv_sec < t.tv_sec)
			break;

		d = request->tv_sec - t.tv_sec;

		if (d > 0) {
			start_ms = timer_read();
			request_ms = (uint64_t)((d > 1) ? 1000 : 10);

			if (task_current()->asm_data3 != 0)
				return DE_INTERRUPT;

			s(start_ms, request_ms, NULL);
			continue;
		}

		if (request->tv_nsec <= t.tv_nsec)
			break;

		task_yield();
	}

	return 0;
}
