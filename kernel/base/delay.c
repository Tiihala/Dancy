/*
 * Copyright (c) 2021 Antti Tiihala
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
 * base/delay.c
 *      Delay function
 */

#include <dancy.h>

void delay(uint32_t nanoseconds)
{
	uint32_t tsc_a, tsc_d;
	uint64_t tsc_add, tsc_base, tsc_val;

	task_switch_disable();

	cpu_rdtsc(&tsc_a, &tsc_d);

	/*
	 * tsc_add = (delay_tsc_hz * nanoseconds) / 1000000000;
	 */
	tsc_add = (uint64_t)(kernel->delay_tsc_hz / 1000);
	tsc_add *= (uint64_t)nanoseconds;
	tsc_add /= 1000000;

	tsc_base = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;
	tsc_val = tsc_base;

	while ((tsc_val - tsc_base) < tsc_add) {
		cpu_rdtsc(&tsc_a, &tsc_d);
		tsc_val = (((uint64_t)tsc_d << 16) << 16) | (uint64_t)tsc_a;
	}

	task_switch_enable();
}
