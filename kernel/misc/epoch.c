/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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
 * misc/epoch.c
 *      Thursday 1 January 1970 00:00:00
 */

#include <dancy.h>

static int epoch_lock;

static uint64_t cached_epoch;
static uint64_t cached_timer;

static uint64_t fast_read(uint64_t max_msec_diff)
{
	void *lock_local = &epoch_lock;
	uint64_t msec, retval = 0;

	spin_enter(&lock_local);
	msec = timer_read();

	if (cached_epoch) {
		uint64_t msec_diff = msec - cached_timer;

		if (msec_diff < max_msec_diff)
			retval = cached_epoch + msec_diff / 1000;
	}

	spin_leave(&lock_local);

	return retval;
}

static uint64_t slow_read(void)
{
	void *lock_local = &epoch_lock;
	struct rtc_time rt;
	uint64_t retval;

	if (rtc_read(&rt))
		return 0;

	/*
	 * Convert the rtc_time structure to seconds.
	 */
	{
		char buf[32];
		long long s;

		snprintf(&buf[0], 32, "%04d-%02d-%02dT%02d:%02d:%02d",
			(int)rt.year, (int)rt.month, (int)rt.day,
			(int)rt.hour, (int)rt.minute, (int)rt.second);

		if ((s = epoch_seconds(&buf[0])) < 0)
			return 0;

		retval = (uint64_t)s;
	}

	spin_enter(&lock_local);

	cached_timer = timer_read();
	cached_epoch = retval;

	spin_leave(&lock_local);

	return retval;
}

int epoch_init(void)
{
	typedef unsigned long long (*epoch_read_func)(void);
	int state = 0;
	int i;

#ifdef DANCY_32
	const char *name[3] = {
		"_epoch_read", "_epoch_read_ms", "_epoch_sync" };
#else
	const char *name[3] = {
		"epoch_read", "epoch_read_ms", "epoch_sync" };
#endif

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strcmp(kernel->symbol[i].name, name[0])) {
			addr_t a = (addr_t)kernel->symbol[i].value;
			kernel->epoch_read = (epoch_read_func)a;
			state += 1;
			break;
		}
	}

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strcmp(kernel->symbol[i].name, name[1])) {
			addr_t a = (addr_t)kernel->symbol[i].value;
			kernel->epoch_read_ms = (epoch_read_func)a;
			state += 1;
			break;
		}
	}

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strcmp(kernel->symbol[i].name, name[2])) {
			addr_t a = (addr_t)kernel->symbol[i].value;
			kernel->epoch_sync = (void (*)(void))a;
			state += 1;
			break;
		}
	}

	if (state != 3)
		return DE_UNEXPECTED;

	return 0;
}

unsigned long long epoch_read(void)
{
	uint64_t retval;

	if ((retval = fast_read(60000)) != 0)
		return (unsigned long long)retval;

	if ((retval = slow_read()) != 0)
		return (unsigned long long)retval;

	retval = fast_read(ULLONG_MAX);

	return (unsigned long long)retval;
}

unsigned long long epoch_read_ms(void)
{
	return 0;
}

void epoch_sync(void)
{
	void *lock_local = &epoch_lock;

	slow_read();
	spin_enter(&lock_local);

	cached_timer = 0;
	cached_epoch = 0;

	spin_leave(&lock_local);
	slow_read();
}
