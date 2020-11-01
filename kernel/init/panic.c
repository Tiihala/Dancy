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

volatile int panic_lock;

void panic(const char *message)
{
	const uint32_t bsc_bit = (1u << 8);
	char buffer[2048];
	uint32_t a, d;

	spin_lock((int *)&panic_lock);

	cpu_ints(0);
	idt_load_null();

	cpu_rdmsr(0x1B, &a, &d);

	if (message == NULL || (a & bsc_bit) == 0)
		cpu_halt(0);

	/*
	 * Print the message only on the boot processor.
	 */
	snprintf(&buffer[0], sizeof(buffer),
		"   **** SYSTEM PANIC ****\n\n%s\n\n"
		"Please restart the computer. Halted.", message);

	gui_print_alert(&buffer[0]);
	cpu_halt(0);
}
