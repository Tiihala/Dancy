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

#include <init.h>

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

	b_log("Advanced Configuration and Power Interface (ACPI)\n");
	b_log("\tRSDP found at %08X\n", (unsigned)information.rsdp_addr);

	ptr = (const uint8_t *)information.rsdp_addr;
	if (checksum(ptr, 20)) {
		const char *e = "RSDP checksum error\n";
		b_print("Acpi: %s", e), b_log("\t%s", e);
		b_pause();
	}

	revision = *(ptr + 15);
	information.rsdt_addr = (phys_addr_t)LE32(ptr + 16);

	if (revision != 0) {
		length = (uint32_t)LE32(ptr + 20);
		/*
		 * Currently there are no page mappings for physical
		 * address above 0xFFFFFFFF. The XsdtAddress field is
		 * ignored (RsdtAddress used) if the value is too big.
		 */
		if (length >= 33 && !checksum(ptr, (size_t)length)) {
			uint32_t addr_l = (uint32_t)LE32(ptr + 24);
			uint32_t addr_h = (uint32_t)LE32(ptr + 28);

			if (!addr_h) {
				information.xsdt_addr = addr_l;
			} else {
				const char *e = "Acpi: XsdtAddress ignored";
				b_print("%s %08X%08X\n", e, addr_h, addr_l);
				b_log("\t%s %08X%08X\n",
					e + 6, addr_h, addr_l);
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
			const char *e = "RSDT signature is not found\n";
			b_print("Acpi: %s", e), b_log("\t%s", e);
			return b_pause(), NULL;
		}
		b_log("\tRSDT found at %08X\n", (unsigned)(phys_addr_t)ptr);
		addr_size = 4;
	} else {
		/*
		 * Extended System Description Table (XSDT)
		 */
		ptr = (const uint8_t *)information.xsdt_addr;
		if (memcmp(ptr, "XSDT", 4)) {
			const char *e = "XSDT signature is not found\n";
			b_print("Acpi: %s", e), b_log("\t%s", e);
			return b_pause(), NULL;
		}
		b_log("\tXSDT found at %08X\n", (unsigned)(phys_addr_t)ptr);
		addr_size = 8;
	}

	length = (uint32_t)LE32(ptr + 4);

	/*
	 * The error here does not stop the process.
	 */
	if (checksum(ptr, (size_t)length)) {
		const char *table_name = (const char *)ptr;
		b_print("Acpi: %.4s checksum error\n", table_name);
		b_log("\t%.4s checksum error\n", table_name);
	}

	/*
	 * Find physical addresses that point to other tables.
	 */
	for (i = 36; i < length; i += addr_size) {
		phys_addr_t addr = (phys_addr_t)LE32(ptr + i);
		const uint8_t *table;
		uint32_t table_length;

		if (addr_size == 8 && LE32(ptr + i + 4)) {
			const char *e = "skipping table (64-bit)\n";
			b_print("Acpi: %s", e), b_log("\t%s", e);
			continue;
		}

		table = (const uint8_t *)addr;
		table_length = (uint32_t)LE32(table + 4);

		if (checksum(table, (size_t)table_length)) {
			const char *table_name = (const char *)ptr;
			b_print("Acpi: %.4s checksum error\n", table_name);
			b_log("\t%.4s checksum error\n", table_name);
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

		if ((char)table[0] >= 'A' && (char)table[0] <= 'Z') {
			const char *m = "\t%.4s found at %08X\n";
			b_log(m, (const char *)table, (unsigned)addr);
		}
	}

	/*
	 * Find values from FADT table.
	 */
	if (information.fadt_addr) {
		const uint8_t *fadt = (const uint8_t *)information.fadt_addr;
		length = (uint32_t)LE32(fadt + 4);

		if (length >= 45 + 1) {
			static const char *pm_prof[9] = {
				"unspecified",
				"desktop",
				"mobile",
				"workstation",
				"enterprise server",
				"SOHO server",
				"appliance PC",
				"performance server",
				"tablet"
			};
			unsigned a = (unsigned)fadt[45];
			const char *b = (a < 9) ? pm_prof[a] : pm_prof[0];

			b_log("\tPreferred PM Profile is \"%s\"\n", b);
		}
		if (length >= 108 + 1) {
			unsigned a = (unsigned)fadt[108];
			information.rtc_century_idx = a;
			if (a)
				b_log("\tCMOS RTC century index is %u\n", a);
		}
		if (length >= 109 + 2) {
			unsigned a = (unsigned)fadt[109];
			unsigned b = a + ((unsigned)fadt[110] << 8);
			information.iapc_boot_arch = b;

			if (b & INIT_ARCH_LEGACY_DEVICES)
				b_log("\tLegacy Devices flag is set\n");
			if (b & INIT_ARCH_8042)
				b_log("\t8042 is supported\n");
			if (b & INIT_ARCH_VGA_NOT_PRESENT)
				b_log("\tVGA is not present\n");
			if (b & INIT_ARCH_MSI_NOT_SUPPORTED)
				b_log("\tMSI is not supported\n");
			if (b & INIT_ARCH_PCIE_ASPM_CONTROLS)
				b_log("\tPCIe ASPM Controls flag is set\n");
			if (b & INIT_ARCH_CMOS_RTC_NOT_PRESENT)
				b_log("\tCMOS RTC is not present\n");
		}
	}
	b_log("\n");
	return (init = 1), &information;
}
