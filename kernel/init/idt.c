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
	unsigned entries = (unsigned)(sizeof(names) / sizeof(names[0]));
	const size_t size = 128;
	char message[1024], *ptr;
	int add, i;

	ptr = &message[0];
	add = snprintf(ptr, size, "   **** SYSTEM PANIC ****\n\n");
	ptr += ((add > 0) ? add : 0);

	if (num < entries)
		add = snprintf(ptr, size, "%s\n\n", names[num]);
	else
		add = snprintf(ptr, size, "Interrupt Vector %u\n\n", num);
	ptr += ((add > 0) ? add : 0);

	if (num == 14) {
		phys_addr_t cr2;

		pg_get_fault(&cr2);
		add = snprintf(ptr, size, "CR2: %p\n\n", cr2);
		ptr += ((add > 0) ? add : 0);
	}

	for (i = 1; i >= -1; i--) {
		const phys_addr_t *s = (const phys_addr_t *)stack;
		const void *val = (const void *)s[i];

		add = snprintf(ptr, size, "  [%p]  %p\n", (s + i), val);
		ptr += ((add > 0) ? add : 0);
	}

	snprintf(ptr, size, "\nPlease restart the computer. Halted.");
	gui_print_alert(&message[0]);
	cpu_halt(0);
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
	 * Nested exceptions are not allowed. If an exception
	 * occurs, the computer will reboot (a triple fault).
	 */
	if (num < 32) {
		idt_load_null();

		/*
		 * Non-Maskable Interrupt (on BSP).
		 */
		if (num == 2 && apic_bsp_id == apic_id()) {
			idt_nmi = 2;
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

		/*
		 * Application processors will set an error code. It
		 * is known that there are "race conditions" and the
		 * displayed error message is just one of the errors
		 * if APs trigger exceptions at the same time.
		 */
		if (apic_bsp_id != apic_id()) {
			uint32_t e =  (uint32_t)(num + 256);

			cpu_write32((uint32_t *)&init_ap_error, e);
			cpu_halt(0);
		}

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
	 * Check for AP errors on BSP.
	 */
	if (init_ap_error) {
		char message[32];

		snprintf(&message[0], 32, "AP Error: %08X", init_ap_error);
		panic(&message[0]);
	}

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
