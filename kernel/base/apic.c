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
	void *eoi = (void *)(kernel->apic_base_addr + 0xB0);

	cpu_write32(eoi, 0);
}

uint32_t apic_id(void)
{
	const void *id;

	if (!kernel->apic_enabled)
		return kernel->apic_bsp_id;

	id = (const void *)(kernel->apic_base_addr + 0x20);

	return (cpu_read32(id) >> 24);
}

void apic_send(uint32_t icr_low, uint32_t icr_high)
{
	void *icr_300 = (void *)(kernel->apic_base_addr + 0x300);
	void *icr_310 = (void *)(kernel->apic_base_addr + 0x310);

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
	const void *icr_300 = (const void *)(kernel->apic_base_addr + 0x300);
	unsigned wait_delivery_status = 1000;

	while (wait_delivery_status--) {
		const uint32_t delivery_status_bit = (1u << 12);

		if ((cpu_read32(icr_300) & delivery_status_bit) == 0)
			return 0;

		delay(1000000);
	}

	return 1;
}
