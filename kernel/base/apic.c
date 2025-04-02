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
 * base/apic.c
 *      Advanced Programmable Interrupt Controller
 */

#include <dancy.h>

void apic_eoi(void)
{
	void *eoi = (void *)(kernel->apic_base_vaddr + 0xB0);

	cpu_write32(eoi, 0);
}

uint32_t apic_id(void)
{
	const void *id;

	if (!kernel->apic_enabled)
		return kernel->apic_bsp_id;

	id = (const void *)(kernel->apic_base_vaddr + 0x20);

	return (cpu_read32(id) >> 24);
}

void apic_send(uint32_t icr_low, uint32_t icr_high)
{
	void *icr_300 = (void *)(kernel->apic_base_vaddr + 0x300);
	void *icr_310 = (void *)(kernel->apic_base_vaddr + 0x310);

	unsigned wait_delivery_status = 100;

	while (wait_delivery_status--) {
		const uint32_t delivery_status_bit = (1u << 12);

		if ((cpu_read32(icr_300) & delivery_status_bit) == 0)
			break;

		delay(1000000);
	}

	cpu_write32(icr_310, icr_high);
	cpu_write32(icr_300, icr_low);
}

int apic_wait_delivery(void)
{
	const void *icr_300 = (const void *)(kernel->apic_base_vaddr + 0x300);
	unsigned wait_delivery_status = 1000;

	while (wait_delivery_status--) {
		const uint32_t delivery_status_bit = (1u << 12);

		if ((cpu_read32(icr_300) & delivery_status_bit) == 0)
			return 0;

		delay(1000000);
	}

	return 1;
}

static int ioapic_lock;

static void ioapic_access(int irq, int interrupt_mask, uint64_t *redtbl)
{
	const uint32_t ioapic_irq_base = 0x40;
	int i;

	for (i = 0; i < kernel->io_apic_count; i++) {
		phys_addr_t addr = kernel->io_apic[i].addr;
		uint32_t base_int = kernel->io_apic[i].base_int;
		uint32_t polarity = 0, trigger_mode = 0;

		uint32_t global_int, flags, ioapicver, max_redir;
		uint32_t j;

		if (irq < 16) {
			global_int = kernel->io_apic_override[irq].global_int;
			flags = kernel->io_apic_override[irq].flags;
		} else {
			global_int = (uint32_t)irq;
			flags = 0;
		}

		/*
		 * MPS INTI Flags
		 *
		 * Bit     Bit
		 * Length  Offset  Description
		 *   2       0       Polarity of APIC I/O signals
		 *                      01 Active high
		 *                      11 Active low
		 *   2       2       Trigger mode of APIC I/O signals
		 *                      01 Edge-triggered
		 *                      11 Level-triggered
		 */
		if ((flags & 3) == 3)
			polarity = 1;
		if (((flags >> 2) & 3) == 3)
			trigger_mode = 1;

		/*
		 * Read I/O APIC Version.
		 */
		cpu_write32((void *)addr, 0x01);
		ioapicver = cpu_read32((const void *)(addr + 0x10));
		max_redir = (ioapicver >> 16) & 0xFF;

		for (j = 0; j <= max_redir; j++) {
			uint32_t lo_value, hi_value;
			uint64_t redtbl_value;

			if (base_int + j != global_int)
				continue;

			/*
			 * Update the redirection table register (low dword).
			 */
			if (redtbl == NULL) {
				lo_value = ioapic_irq_base;
				lo_value += (uint32_t)irq;
				lo_value |= (polarity << 13);
				lo_value |= (trigger_mode << 15);

				if (interrupt_mask)
					lo_value |= (1u << 16);

				cpu_write32((void *)addr, (0x10 + (j * 2)));
				cpu_write32((void *)(addr + 0x10), lo_value);
				return;
			}

			/*
			 * Read the current redirection table register and
			 * ignore the interrupt_mask variable.
			 */
			cpu_write32((void *)addr, (0x10 + (j * 2)));
			lo_value = cpu_read32((const void *)(addr + 0x10));

			cpu_write32((void *)addr, (0x10 + (j * 2) + 1));
			hi_value = cpu_read32((const void *)(addr + 0x10));

			redtbl_value = ((uint64_t)hi_value << 16) << 16;
			redtbl_value |= (uint64_t)lo_value;

			*redtbl = redtbl_value;
			return;
		}
	}
}

void ioapic_disable(int irq)
{
	void *lock_local = &ioapic_lock;

	if (!kernel->io_apic_enabled)
		return;

	if (irq < 0 || irq == 2 || irq > 23)
		return;

	pg_enter_kernel();
	spin_enter(&lock_local);

	ioapic_access(irq, 1, NULL);

	spin_leave(&lock_local);
	pg_leave_kernel();
}

void ioapic_enable(int irq)
{
	void *lock_local = &ioapic_lock;

	if (!kernel->io_apic_enabled)
		return;

	if (irq < 0 || irq == 2 || irq > 23)
		return;

	pg_enter_kernel();
	spin_enter(&lock_local);

	ioapic_access(irq, 0, NULL);

	spin_leave(&lock_local);
	pg_leave_kernel();
}

uint64_t ioapic_redtbl(int irq)
{
	void *lock_local = &ioapic_lock;
	uint64_t redtbl = 0;

	if (!kernel->io_apic_enabled)
		return redtbl;

	if (irq < 0 || irq == 2 || irq > 23)
		return redtbl;

	pg_enter_kernel();
	spin_enter(&lock_local);

	ioapic_access(irq, 0, &redtbl);

	spin_leave(&lock_local);
	pg_leave_kernel();

	return redtbl;
}
