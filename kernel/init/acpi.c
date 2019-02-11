/*
 * Copyright (c) 2019 Antti Tiihala
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
 * init/acpi.c
 *      Advanced Configuration and Power Interface Specification
 */

#include <dancy.h>

static struct acpi_information information;

static int checksum(const uint8_t *table, size_t size)
{
	unsigned sum = 0;
	size_t i;

	for (i = 0; i < size; i++)
		sum += (unsigned)table[i];
	return (int)(sum & 0xFFu);
}

struct acpi_information *acpi_get_information(void)
{
	static int init = 0;
	uint8_t revision;
	const uint8_t *ptr;
	uint32_t length, addr_size, i;

	if (init)
		return (init == 1) ? &information : NULL;

	memset(&information, 0, sizeof(information));
	init = -1;

	information.rsdp_addr = b_get_parameter(B_ACPI_POINTER);
	if (!information.rsdp_addr)
		return NULL;

	ptr = (const uint8_t *)information.rsdp_addr;
	if (checksum(ptr, 20))
		return NULL;

	revision = *(ptr + 15);
	information.rsdt_addr = *((const uint32_t *)(ptr + 16));

	if (revision != 0) {
		length = *((const uint32_t *)(ptr + 20));
		/*
		 * Currently there are no page mappings for physical
		 * address above 0xFFFFFFFF. The XsdtAddress field is
		 * ignored (RsdtAddress used) if the value is too big.
		 */
		if (length >= 33 && !checksum(ptr, (size_t)length)) {
			uint32_t addr_l = *((const uint32_t *)(ptr + 24));
			uint32_t addr_h = *((const uint32_t *)(ptr + 28));

			if (!addr_h) {
				information.xsdt_addr = addr_l;
			} else {
				const char *e = "Acpi: ignored XsdtAddress";
				b_print("%s %08X%08X\n", e, addr_h, addr_l);
				b_pause();
			}
		}
	}

	if (!information.xsdt_addr) {
		/*
		 * Root System Description Table (RSDT)
		 */
		ptr = (const uint8_t *)information.rsdt_addr;
		if (memcmp(ptr, "RSDT", 4)) {
			b_print("Acpi: signature RSDT is not found\n");
			return NULL;
		}
		addr_size = 4;
	} else {
		/*
		 * Extended System Description Table (XSDT)
		 */
		ptr = (const uint8_t *)information.xsdt_addr;
		if (memcmp(ptr, "XSDT", 4)) {
			b_print("Acpi: signature XSDT is not found\n");
			return NULL;
		}
		addr_size = 8;
	}

	length = *((const uint32_t *)(ptr + 4));

	/*
	 * The error here does not stop the process.
	 */
	if (checksum(ptr, (size_t)length)) {
		const char *table_name = (const char *)ptr;
		b_print("Acpi: checksum error (%.4s)\n", table_name);
		information.errors += 1;
	}

	/*
	 * Find physical addresses that point to other tables.
	 */
	for (i = 36; i < length; i += addr_size) {
		phys_addr_t addr = *((const uint32_t *)(ptr + i));
		const uint8_t *table;
		uint32_t table_length;

		if (addr_size == 8 && *((const uint32_t *)(ptr + i + 4))) {
			b_print("Acpi: skipping table (64-bit address)\n");
			information.errors += 1;
			continue;
		}

		table = (const uint8_t *)addr;
		table_length = *((const uint32_t *)(table + 4));

		if (checksum(table, (size_t)table_length)) {
			const char *table_name = (const char *)table;
			b_print("Acpi: checksum error (%.4s)\n", table_name);
			information.errors += 1;
		}

		/*
		 * Signature for FADT is 'FACP'.
		 */
		if (!memcmp(table, "FACP", 4))
			information.fadt_addr = addr;

		/*
		 * Signature for MADT is 'APIC'.
		 */
		if (!memcmp(table, "APIC", 4))
			information.madt_addr = addr;
	}

	/*
	 * Find values from FADT table.
	 */
	if (information.fadt_addr) {
		const uint8_t *fadt = (const uint8_t *)information.fadt_addr;
		length = *((const uint32_t *)(fadt + 4));

		if (length >= 108 + 1)
			information.rtc_century_idx = (unsigned)fadt[108];
		if (length >= 109 + 2) {
			unsigned a = (unsigned)fadt[109];
			unsigned b = (unsigned)fadt[110] << 8;
			information.iapc_boot_arch = a + b;
		}
	}
	return (init = 1), &information;
}
