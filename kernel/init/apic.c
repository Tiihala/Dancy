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
 * init/apic.c
 *      Advanced Programmable Interrupt Controller
 */

#include <init.h>

int apic_mode = 0;
const unsigned apic_spurious_vector = 0xFF;
const unsigned ioapic_irq_base = 0x40;

static phys_addr_t apic_base;

int apic_init(void)
{
	const struct acpi_information *acpi = acpi_get_information();
	phys_addr_t local_apic_base = apic_base;
	unsigned flags;

	/*
	 * The ACPI tables and I/O APIC must be available
	 * if using the local APIC on this system.
	 */
	if (acpi == NULL || acpi->num_io_apic == 0)
		return 1;

	/*
	 * Check that APIC On-Chip is available.
	 */
	{
		uint32_t a, c, d, b;

		cpu_id((a = 1, &a), &c, &d, &b);

		if ((d & (1u << 9)) == 0)
			return 1;
	}

	/*
	 * Read the APIC base from the ACPI.
	 */
	if (local_apic_base == 0) {
		local_apic_base = acpi->local_apic_base;
		if ((local_apic_base & 0xFFF) != 0)
			local_apic_base = 0;
	}

	/*
	 * Read the APIC base from the MSR.
	 */
	{
		int unexpected_error = 0;
		phys_addr_t local_apic_msr, flags_mask = 0xFFF;
		uint32_t a, d, d_mask = 0xFFFFF;

		cpu_rdmsr(0x1B, &a, &d);

		local_apic_msr = a | (((d & d_mask) << 16) << 16);

		flags = (unsigned)(local_apic_msr & flags_mask);

		/*
		 * The global enable bit must be set.
		 */
		if ((flags & (1u << 11)) == 0)
			return 1;

		local_apic_msr &= (~flags_mask);

		if (local_apic_base == 0)
			local_apic_base = local_apic_msr;
#ifdef DANCY_32
		/*
		 * Do not allow high addresses on 32-bit systems.
		 */
		if ((d & d_mask) != 0)
			return 1;
#endif
		if (local_apic_base != local_apic_msr)
			unexpected_error = 1;

		if (local_apic_base == 0 || (local_apic_base & 0xFFF) != 0)
			unexpected_error = 1;

		if (unexpected_error) {
			unsigned bsp_bit = (flags >> 8) & 1;
			char message[128];

			if (!bsp_bit)
				return 1;

			snprintf(&message[0], sizeof(message),
				"Unexpected Addresses\n\n"
				"APIC_BASE %p (ACPI)\nAPIC_BASE %p (MSR)",
				(void *)local_apic_base,
				(void *)local_apic_msr);

			panic(&message[0]);
		}
	}

	apic_base = local_apic_base;

	/*
	 * Task Priority Register (TPR).
	 */
	{
		void *r = (void *)(apic_base + 0x80);

		cpu_write32(r, 0x00000000);
	}

	/*
	 * Logical Destination Register.
	 */
	{
		void *r = (void *)(apic_base + 0xD0);

		cpu_write32(r, 0x00000000);
	}

	/*
	 * Destination Format Register.
	 */
	{
		void *r = (void *)(apic_base + 0xE0);

		cpu_write32(r, 0xFFFFFFFF);
	}

	/*
	 * LVT Timer Register.
	 */
	{
		void *r = (void *)(apic_base + 0x320);

		cpu_write32(r, 0x00010000);
	}

	/*
	 * LVT LINT0 Register.
	 */
	{
		void *r = (void *)(apic_base + 0x350);
		unsigned bsp_bit = (flags >> 8) & 1;

		if (!bsp_bit)
			cpu_write32(r, 0x00010000);
	}

	/*
	 * LVT LINT1 Register.
	 */
	{
		void *r = (void *)(apic_base + 0x360);
		unsigned bsp_bit = (flags >> 8) & 1;

		if (!bsp_bit)
			cpu_write32(r, 0x00010000);
	}

	/*
	 * LVT Error Register.
	 */
	{
		void *r = (void *)(apic_base + 0x370);

		cpu_write32(r, 0x00010000);
	}

	/*
	 * Initial Count Register (Timer).
	 */
	{
		void *r = (void *)(apic_base + 0x380);

		cpu_write32(r, 0x7FFFFFFF);
	}

	/*
	 * Divide Configuration Register (Timer).
	 */
	{
		void *r = (void *)(apic_base + 0x3E0);

		cpu_write32(r, 0x00000000);
	}

	/*
	 * Set the spurious vector and software enable bit.
	 */
	{
		void *r = (void *)(apic_base + 0xF0);

		cpu_write32(r, (uint32_t)(apic_spurious_vector | 0x100));
	}

	return apic_mode = 1, 0;
}

void apic_eoi(void)
{
	void *eoi = (void *)(apic_base + 0xB0);

	cpu_write32(eoi, 0);
}

void ioapic_init(void)
{
	const struct acpi_information *acpi = acpi_get_information();
	uint32_t bsc_apic_id;
	uint32_t i, j;

	if (!apic_mode)
		return;

	/*
	 * Get the APIC ID.
	 */
	{
		const void *r = (const void *)(apic_base + 0x20);
		uint32_t id = cpu_read32(r);

		bsc_apic_id = id >> 24;
	}

	/*
	 * Initialize all I/O APICs.
	 */
	for (i = 0; i < acpi->num_io_apic; i++) {
		struct acpi_io_apic io_apic;
		uint32_t ioapicver, max_redir;
		phys_addr_t addr;

		if (acpi_get_io_apic(i, &io_apic))
			panic("I/O APIC: MADT");

		addr = io_apic.addr;

		/*
		 * Read I/O APIC Version.
		 */
		cpu_write32((void *)addr, 0x01);
		ioapicver = cpu_read32((const void *)(addr + 0x10));
		max_redir = (ioapicver >> 16) & 0xFF;

		/*
		 * Use defined values for every redirection table register.
		 */
		for (j = 0; j <= max_redir; j++) {
			uint32_t lo_value = 0x00010000;
			uint32_t hi_value = (bsc_apic_id << 24);

			cpu_write32((void *)addr, (0x10 + (j * 2)));
			cpu_write32((void *)(addr + 0x10), lo_value);

			cpu_write32((void *)addr, (0x10 + (j * 2) + 1));
			cpu_write32((void *)(addr + 0x10), hi_value);
		}
	}
}

void ioapic_enable(unsigned isa_irq)
{
	const struct acpi_information *acpi = acpi_get_information();
	uint32_t i, j;

	if (!apic_mode && isa_irq >= 16)
		return;

	/*
	 * Find the ISA IRQ.
	 */
	for (i = 0; i < acpi->num_io_apic; i++) {
		struct acpi_io_apic io_apic;
		uint32_t base_int, flags, global_int;
		uint32_t polarity = 0, trigger_mode = 0;
		uint32_t ioapicver, max_redir;
		phys_addr_t addr;

		if (acpi_get_io_apic(i, &io_apic))
			break;

		addr = io_apic.addr;
		base_int = io_apic.base_int;

		/*
		 * Get the Interrupt Source Override information.
		 */
		global_int = io_apic.irq[isa_irq].global_int;
		flags = io_apic.irq[isa_irq].flags;

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
			if (base_int + j == global_int) {
				uint32_t lo_value = (uint32_t)ioapic_irq_base;

				lo_value += (uint32_t)isa_irq;
				lo_value |= (polarity << 13);
				lo_value |= (trigger_mode << 15);

				cpu_write32((void *)addr, (0x10 + (j * 2)));
				cpu_write32((void *)(addr + 0x10), lo_value);
				return;
			}
		}
	}

	panic("I/O APIC: ISA IRQ");
}
