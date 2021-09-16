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
 * pci/pci.c
 *      Peripheral Component Interconnect
 */

#include <dancy.h>

static int pci_lock;

static uint32_t pci_read32_locked(int b, int d, int f, int off)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000 | ((uint32_t)off & 0xFC);

	addr |= (((uint32_t)f & 0x07) << 8);
	addr |= (((uint32_t)d & 0x1F) << 11);
	addr |= (((uint32_t)b & 0xFF) << 16);

	cpu_out32(pci_config_addr, addr);
	return cpu_in32(pci_config_data);
}

static void pci_write32_locked(int b, int d, int f, int off, uint32_t val)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000 | ((uint32_t)off & 0xFC);

	addr |= (((uint32_t)f & 0x07) << 8);
	addr |= (((uint32_t)d & 0x1F) << 11);
	addr |= (((uint32_t)b & 0xFF) << 16);

	cpu_out32(pci_config_addr, addr);
	cpu_out32(pci_config_data, val);
}

uint32_t pci_read(struct pci_id *pci, int offset)
{
	uint32_t ret = 0;

	if (offset < 0 || (offset & 3) != 0)
		return ret;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)offset;

		if (offset <= 0x0FFC) {
			pg_enter_kernel();
			ret = cpu_read32((const uint32_t *)a);
			pg_leave_kernel();
		}

	} else {
		void *lock_local = &pci_lock;
		int b = pci->bus, d = pci->device, f = pci->func;

		if (offset <= 0xFC) {
			spin_enter(&lock_local);
			ret = pci_read32_locked(b, d, f, offset);
			spin_leave(&lock_local);
		}
	}

	return ret;
}

void pci_write(struct pci_id *pci, int offset, uint32_t value)
{
	if (offset < 0 || (offset & 3) != 0)
		return;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)offset;

		if (offset <= 0x0FFC) {
			pg_enter_kernel();
			cpu_write32((uint32_t *)a, value);
			pg_leave_kernel();
		}

	} else {
		void *lock_local = &pci_lock;
		int b = pci->bus, d = pci->device, f = pci->func;

		if (offset <= 0xFC) {
			spin_enter(&lock_local);
			pci_write32_locked(b, d, f, offset, value);
			spin_leave(&lock_local);
		}
	}
}
