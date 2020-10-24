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
 * init/idt.c
 *      Interrupt handler
 */

#include <init.h>

void idt_handler(unsigned num, unsigned err_code, const void *stack)
{
	const unsigned pic_irq_base = 32;

	/*
	 * Ignore err_code and stack.
	 */
	(void)err_code;
	(void)stack;

	/*
	 * Nested exceptions are not allowed. If an exception
	 * occurs, the computer will reboot (a triple fault).
	 */
	if (num < 32) {
		idt_load_null();

		/*
		 * Debug Exception.
		 */
		if (num == 1) {
			idt_restore();
			return;
		}

		/*
		 * Page-Fault Exception.
		 */
		if (num == 14 && pg_handler() == 0) {
			idt_restore();
			return;
		}

		cpu_halt(0);
	}

	/*
	 * Spurious IRQ 7 (PIC 1).
	 */
	if (num == pic_irq_base + 7)
		return;

	/*
	 * Spurious IRQ 15 (PIC 2).
	 */
	if (num == pic_irq_base + 15) {
		cpu_out8(0x20, 0x20);
		return;
	}

	cpu_halt(0);
}
