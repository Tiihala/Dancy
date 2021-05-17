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
 * base/irq.c
 *      Interrupt Request
 */

#include <dancy.h>

static int irq_nop_apic(int irq)
{
	return (irq < 0) ? 0 : 1;
}

static int irq_nop_pic(int irq)
{
	/*
	 * Spurious IRQ 7 (PIC 1).
	 */
	if (irq == 7)
		return 0;

	/*
	 * Spurious IRQ 15 (PIC 2).
	 */
	if (irq == 15) {
		cpu_out8(0x20, 0x20);
		return 0;
	}

	return (irq < 0) ? 0 : 1;
}

int (*irq_handler_apic)(int irq) = irq_nop_apic;
int (*irq_handler_pic)(int irq) = irq_nop_pic;

int irq_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	return 0;
}
