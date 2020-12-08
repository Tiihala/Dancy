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
 * init/panic.c
 *      Unrecoverable errors
 */

#include <init.h>

static int panic_lock;

void panic(const char *message)
{
	void *panic_lock_local = &panic_lock;
	uint32_t i;

	/*
	 * Application processors halt immediately.
	 */
	if (apic_bsp_id != apic_id())
		cpu_halt(0);

	/*
	 * The panic() function is not reentrant.
	 */
	spin_enter(&panic_lock_local);

	/*
	 * Send a non-maskable interrupt to every
	 * application processor.
	 */
	for (i = 0; i < smp_ap_count; i++) {
		uint32_t ap_id = smp_ap_id[i];
		uint32_t icr_low, icr_high;

		if (ap_id == apic_bsp_id || ap_id > 0xFE)
			continue;

		icr_low = 0x00004400;
		icr_high = ap_id << 24;

		apic_send(icr_low, icr_high);
	}

	/*
	 * Print the message.
	 */
	if (message != NULL) {
		static char buffer[2048];

		snprintf(&buffer[0], sizeof(buffer),
			"   **** SYSTEM PANIC ****\n\n%s\n\n"
			"Please restart the computer. Halted.", message);

		gui_print_alert(&buffer[0]);
	}

	/*
	 * This function does not return.
	 */
	cpu_halt(0);
}
