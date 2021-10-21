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

/*
 * void delay_calibrate(void)
 *
 * This function must be called every 1 millisecond until delay_ready is set.
 */
void delay_calibrate(void)
{
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
