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

static int irq_lock;

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

void irq_disable(int irq)
{
	void *lock_local;

	if (kernel->io_apic_enabled) {
		ioapic_disable(irq);
		return;
	}

	lock_local = &irq_lock;
	spin_enter(&lock_local);

	if (irq >= 0 && irq != 2 && irq <= 7) {
		unsigned int val = (unsigned int)cpu_in8(0x21);

		val |= (unsigned int)(1 << irq);
		cpu_out8(0x21, (uint8_t)val);

	} else if (irq >= 8 && irq <= 15) {
		unsigned int val = (unsigned int)cpu_in8(0xA1);

		val |= (unsigned int)(1 << (irq - 8));
		cpu_out8(0xA1, (uint8_t)val);
	}

	spin_leave(&lock_local);
}

void irq_enable(int irq)
{
	void *lock_local;

	if (kernel->io_apic_enabled) {
		ioapic_enable(irq);
		return;
	}

	lock_local = &irq_lock;
	spin_enter(&lock_local);

	if (irq >= 0 && irq != 2 && irq <= 7) {
		unsigned int val = (unsigned int)cpu_in8(0x21);

		val &= ~((unsigned int)(1 << irq));
		cpu_out8(0x21, (uint8_t)val);

	} else if (irq >= 8 && irq <= 15) {
		unsigned int val = (unsigned int)cpu_in8(0xA1);

		val &= ~((unsigned int)(1 << (irq - 8)));
		cpu_out8(0xA1, (uint8_t)val);
	}

	spin_leave(&lock_local);
}
