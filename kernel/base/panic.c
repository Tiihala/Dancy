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
 * base/panic.c
 *      Unrecoverable errors
 */

#include <dancy.h>

int panic_lock;

void panic(const char *message)
{
	uint32_t current_id;
	int i;

	cpu_ints(0);

	if (!spin_trylock(&panic_lock))
		cpu_halt(0);

	current_id = apic_id();

	/*
	 * Send a non-maskable interrupt to every other
	 * application processor (except the current one).
	 */
	for (i = 0; i < kernel->smp_ap_count; i++) {
		uint32_t ap_id = kernel->smp_ap_id[i];
		uint32_t icr_low, icr_high;

		if (ap_id == current_id || ap_id > 0xFE)
			continue;

		icr_low = 0x00004400;
		icr_high = ap_id << 24;

		apic_send(icr_low, icr_high);
	}

	/*
	 * Send a non-maskable interrupt to the BSP if
	 * running on an application processor.
	 */
	if (current_id != kernel->apic_bsp_id) {
		uint32_t icr_low, icr_high;

		icr_low = 0x00004400;
		icr_high = kernel->apic_bsp_id << 24;

		apic_send(icr_low, icr_high);
	}

	apic_wait_delivery();
	delay(1000000);

	/*
	 * Print the message.
	 */
	if (message != NULL) {
		static char buffer[4096];

		snprintf(&buffer[0], sizeof(buffer),
			"\t**** SYSTEM PANIC ****\n\n%s\n\n"
			"Please restart the computer. Halted.", message);

		con_panic(&buffer[0]);
	}

	delay(2000000000);
	cpu_halt(0);
}
