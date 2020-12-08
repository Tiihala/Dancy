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

volatile unsigned idt_irq0;

static void idt_panic(unsigned num, const void *stack)
{
	static const char *names[] = {
		"#DE - Divide-by-Zero Exception",
		"#DB - Debug Exception",
		"NMI - Non-Maskable-Interrupt Exception",
		"#BP - Breakpoint Exception",

		"#OF - Overflow Exception",
		"#BR - Bound-Range Exception",
		"#UD - Invalid-Opcode Exception",
		"#NM - Device-Not-Available Exception",

		"#DF - Double-Fault Exception",
		"Coprocessor-Segment-Overrun Exception",
		"#TS - Invalid-TSS Exception",
		"#NP - Segment-Not-Present Exception",

		"#SS - Stack Exception",
		"#GP - General-Protection Exception",
		"#PF - Page-Fault Exception"
	};
	char message[512];
	char *ptr = &message[0];
	const unsigned entries = (unsigned)(sizeof(names) / sizeof(names[0]));
	const size_t size = 128;
	int add, i;

	if (num < entries)
		add = snprintf(ptr, size, "%s\n", names[num]);
	else
		add = snprintf(ptr, size, "Interrupt Vector %u\n", num);
	ptr += ((add > 0) ? add : 0);

	if (num == 14) {
		phys_addr_t cr2;

		pg_get_fault(&cr2);
		add = snprintf(ptr, size, "\nCR2: %p\n", cr2);
		ptr += ((add > 0) ? add : 0);
	}

	for (i = 1; i >= -1; i--) {
		const phys_addr_t *s = (const phys_addr_t *)stack;
		const void *val = (const void *)s[i];

		add = snprintf(ptr, size, "\n  [%p]  %p", (s + i), val);
		ptr += ((add > 0) ? add : 0);
	}

	panic(&message[0]);
}

static void end(unsigned irq)
{
	if (apic_mode) {
		apic_eoi();
		return;
	}

	/*
	 * PIC 8259 End Of Interrupt.
	 */
	if (irq >= 8)
		cpu_out8(0xA0, 0x20);
	cpu_out8(0x20, 0x20);
}

void idt_handler(unsigned num, const void *stack)
{
	static unsigned idt_nmi;
	const unsigned pic_irq_base = 32;
	unsigned irq;

	/*
	 * Handle CPU exceptions.
	 */
	if (num < 32) {
		/*
		 * Non-Maskable Interrupt (on BSP).
		 */
		if (num == 2 && apic_bsp_id == apic_id()) {
			idt_nmi = 2;
			return;
		}

		/*
		 * Page-Fault Exception.
		 */
		if (num == 14 && pg_handler() == 0)
			return;

		idt_panic(num, stack);
	}

	/*
	 * Ignore AP interrupts.
	 */
	if (apic_bsp_id != apic_id())
		return;

	/*
	 * Check for Non-Maskable Interrupt.
	 */
	if (idt_nmi)
		idt_panic(idt_nmi, stack);

	/*
	 * Translate num to irq. Unsigned integer modulo wrapping is defined.
	 */
	irq = apic_mode ? (num - ioapic_irq_base) : (num - pic_irq_base);

	/*
	 * Timer interrupt (IRQ 0).
	 */
	if (irq == 0) {
		idt_irq0 += 1;

		if (!delay_ready)
			delay_calibrate();

		end(irq);

		/*
		 * Simple threads for pre-kernel environment.
		 */
		thrd_yield();
		return;
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

	/*
	 * Spurious APIC interrupt.
	 */
	if (num == apic_spurious_vector)
		return;

	idt_panic(num, stack);
}
