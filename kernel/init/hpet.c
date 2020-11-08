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
 * init/hpet.c
 *      High Precision Event Timer
 */

#include <init.h>

int hpet_mode = 0;

static phys_addr_t hpet_base;
static const unsigned hpet_hz = 1000;

static uint32_t hpet_num_tim_cap;
static uint32_t hpet_leg_rt_cap;
static uint32_t hpet_counter_clk_period;

void hpet_init(void)
{
	const struct acpi_information *acpi = acpi_get_information();
	uint32_t timer0_comparator_val;
	uint32_t i, val;
	void *ptr;

	if (apic_mode == 0 || acpi == NULL || acpi->hpet_addr == 0)
		return;

	hpet_base = acpi->hpet_base;
	if (hpet_base == 0 || (hpet_base & 7) != 0)
		panic("HPET: Base Address");

	/*
	 * Read General Capabilities and ID Register.
	 */
	ptr = (void *)(hpet_base + 0x00);
	val = cpu_read32(ptr);
	hpet_num_tim_cap = (val >> 8) & 0x1Fu;
	hpet_leg_rt_cap = (val >> 15) & 1u;

	if (hpet_leg_rt_cap == 0)
		panic("HPET: LegacyReplacement Route Capable bit is not set");

	ptr = (void *)(hpet_base + 0x04);
	hpet_counter_clk_period = cpu_read32(ptr);

	if (hpet_counter_clk_period == 0)
		panic("HPET: Main Counter Tick Period is zero");
	if (hpet_counter_clk_period > 100000000)
		panic("HPET: Main Counter Tick Period is too high");

	/*
	 * Calculate the comparator value for the first timer.
	 */
	{
		unsigned long long val_ull = 1000000000000000ull;

		val_ull /= hpet_counter_clk_period;
		val_ull /= hpet_hz;

		if (val_ull > 0xFFFFFFFFull)
			panic("HPET: Main Counter Tick Period is too low");

		timer0_comparator_val = (uint32_t)val_ull;
	}

	/*
	 * Clear ENABLE_CNF.
	 */
	ptr = (void *)(hpet_base + 0x10);
	cpu_write32(ptr, (cpu_read32(ptr) & 0xFFFFFFFEu));

	/*
	 * Disable all interrupts.
	 */
	for (i = 0; i <= hpet_num_tim_cap; i++) {
		ptr = (void *)(hpet_base + 0x100 + (i * 0x20));

		/*
		 * Set defined configuration value.
		 */
		val = 0x00000000u;

		/*
		 * Get the read-only bits.
		 */
		val |= (cpu_read32(ptr) & 0x00008030u);

		cpu_write32(ptr, val);
	}

	/*
	 * Clear comparator registers.
	 */
	for (i = 0; i <= hpet_num_tim_cap; i++) {
		phys_addr_t comparator = hpet_base + 0x108 + (i * 0x20);

		val = 0x00000000u;
		cpu_write32((void *)(comparator + 0), val);
		cpu_write32((void *)(comparator + 4), val);
	}

	/*
	 * Clear the main counter.
	 */
	val = 0x00000000u;
	cpu_write32((void *)(hpet_base + 0xF0), val);
	cpu_write32((void *)(hpet_base + 0xF4), val);

	/*
	 * Set the first timer.
	 */
	{
		ptr = (void *)(hpet_base + 0x100);

		/*
		 * Edge triggered, enabled, periodic, and 32-bit mode.
		 */
		val = 0x0000010Cu;

		/*
		 * Get the read-only bits.
		 */
		val |= (cpu_read32(ptr) & 0x00008030u);

		if ((val & 0x10) == 0)
			panic("HPET: Periodic Interrupt Capable is not set");

		cpu_write32(ptr, val);

		/*
		 * Set T0_VAL_SET_CNF.
		 */
		cpu_write32(ptr, (val | 0x40u));

		/*
		 * Set the comparator value.
		 */
		ptr = (void *)(hpet_base + 0x108);
		cpu_write32(ptr, timer0_comparator_val);
	}

	/*
	 * Set LEG_RT_CNF and ENABLE_CNF.
	 */
	ptr = (void *)(hpet_base + 0x10);
	cpu_write32(ptr, (cpu_read32(ptr) | 2u));
	cpu_write32(ptr, (cpu_read32(ptr) | 3u));

	hpet_mode = 1;
}
