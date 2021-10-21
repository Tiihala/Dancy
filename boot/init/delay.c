/*
 * Copyright (c) 2020, 2021 Antti Tiihala
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
 * init/delay.c
 *      Delay function
 */

#include <boot/init.h>

volatile int delay_ready = 0;
volatile int delay_error = 0;
volatile uint64_t delay_tsc_hz = 0;

static uint32_t delay_call_count = 0;
static uint64_t delay_prev_value = 0;

/*
 * void delay_calibrate(void)
 *
 * This function must be called every 1 millisecond until delay_ready is set.
 */
void delay_calibrate(void)
{
	const uint32_t base_count = 32;
	uint32_t tsc_a, tsc_d;
	uint64_t tsc_diff, tsc_val;

	if (delay_error != 0 || delay_ready != 0)
		return;

	if (cpu_add32(&delay_call_count, 1) < base_count)
		return;

	/*
	 * Read the reference value first.
	 */
	if (delay_call_count == base_count) {
		cpu_rdtsc(&tsc_a, &tsc_d);
		tsc_val = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;

		delay_prev_value = tsc_val;
		return;
	}

	if (delay_call_count < base_count + 100)
		return;

	/*
	 * Read the value about 100 milliseconds later.
	 */
	cpu_rdtsc(&tsc_a, &tsc_d);
	tsc_val = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;

	/*
	 * The new value from the Time-Stamp Counter must
	 * be larger than the previously read value.
	 */
	if (delay_prev_value >= tsc_val) {
		delay_error = 2;
		return;
	}

	tsc_diff = tsc_val - delay_prev_value;
	delay_tsc_hz = tsc_diff * 10;

	/*
	 * The CPU specifications say that Time-Stamp Counters will
	 * not wraparound very soon. For example, Intel:
	 *
	 * "...will not wraparound within 10 years after being reset."
	 */
	if ((delay_tsc_hz / 1000) > 0xFFFFFFFF) {
		delay_error = 3;
		return;
	}

	/*
	 * delay_ready = 1;
	 */
	spin_lock((int *)&delay_ready);
}

void delay(uint32_t nanoseconds)
{
	uint32_t tsc_a, tsc_d;
	uint64_t tsc_add, tsc_base, tsc_val;

	cpu_rdtsc(&tsc_a, &tsc_d);

	if (!delay_ready)
		cpu_halt(0);

	/*
	 * tsc_add = (delay_tsc_hz * nanoseconds) / 1000000000;
	 *
	 * The delay_tsc_hz has been checked so that
	 *
	 *     ((delay_tsc_hz / 1000) <= 0xFFFFFFFF)
	 */
	tsc_add = (uint64_t)(delay_tsc_hz / 1000);
	tsc_add *= (uint64_t)nanoseconds;
	tsc_add /= 1000000;

	tsc_base = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;
	tsc_val = tsc_base;

	while ((tsc_val - tsc_base) < tsc_add) {
		cpu_rdtsc(&tsc_a, &tsc_d);
		tsc_val = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;
	}
}
