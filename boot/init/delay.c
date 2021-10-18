/*
 * Copyright (c) 2020 Antti Tiihala
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

#define DELAY_COUNTER 10
#define DELAY_RETRIES 400

static int delay_counter = DELAY_COUNTER;
static int delay_retries = DELAY_RETRIES;

static uint64_t delay_min_value = ULLONG_MAX;
static uint64_t delay_max_value = 0;
static uint64_t delay_prev_value = 0;

static void delay_clear(int end)
{
	delay_counter = DELAY_COUNTER;
	if (end)
		delay_retries = DELAY_RETRIES;

	delay_min_value = ULLONG_MAX;
	delay_max_value = 0;
	delay_prev_value = 0;
}

/*
 * void delay_calibrate(void)
 *
 * This function must be called every 1 millisecond until delay_ready is set.
 */
void delay_calibrate(void)
{
	uint32_t tsc_a, tsc_d;
	uint64_t tsc_diff, tsc_val;

	if (delay_error != 0)
		return;

	if (delay_retries <= 0) {
		delay_error = 1;
		return;
	}

	cpu_rdtsc(&tsc_a, &tsc_d);
	tsc_val = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;

	/*
	 * Read the reference value first.
	 */
	if (delay_prev_value == 0) {
		delay_prev_value = tsc_val;
		return;
	}

	/*
	 * The new value from the Time-Stamp Counter must
	 * be larger than the previously read value.
	 */
	if (delay_prev_value >= tsc_val) {
		delay_error = 2;
		return;
	}

	tsc_diff = tsc_val - delay_prev_value;
	delay_prev_value = tsc_val;

	/*
	 * Check the minimum value and continue sampling
	 * if performance is getting better.
	 */
	if (delay_min_value > tsc_diff) {
		delay_min_value = tsc_diff;
		delay_counter += 1;
	}

	if (delay_max_value < tsc_diff)
		delay_max_value = tsc_diff;

	if (delay_counter > 0) {
		delay_counter -= 1;
		return;
	}

	/*
	 * Check that sample values are stable enough.
	 */
	{
		uint64_t value_diff = delay_max_value - delay_min_value;
		int value_rsh = (delay_retries < (DELAY_RETRIES / 2)) ? 3 : 8;

		if (value_diff > (delay_min_value >> value_rsh)) {
			delay_clear(0);
			delay_retries -= 1;
			return;
		}

		/*
		 * Check that the max value can be an uint32_t. This is mostly
		 * theoretical because CPU specifications say that Time-Stamp
		 * Counters will not wraparound very soon. For example, Intel:
		 *
		 * "...will not wraparound within 10 years after being reset."
		 */
		if (delay_max_value > 0xFFFFFFFF) {
			delay_error = 3;
			return;
		}

		delay_tsc_hz = delay_max_value * 1000;
	}

	delay_clear(1);

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
