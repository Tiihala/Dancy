/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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

		/*
		 * Signature for PCI Express memory mapped configuration
		 * space base address Description Table is 'MCFG'.
		 */
		if (!memcmp(table, "MCFG", 4))
			information.mcfg_addr = addr;

		/*
		 * Signature for HPET (High Precision Event Timer). Use the
		 * first table if there are many, and prefer HPET Number 0.
		 */
		if (!memcmp(table, "HPET", 4)) {
			if (information.hpet_addr) {
				if (table_length >= 56 && table[52] == 0)
					information.hpet_addr = addr;
			} else {
				information.hpet_addr = addr;
			}
		}

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

		b_log("\n\t---- FADT ----\n");

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

	/*
	 * Find values from MADT table.
	 */
	if (information.madt_addr) {
		const uint8_t *madt = (const uint8_t *)information.madt_addr;
		length = (uint32_t)LE32(madt + 4);

		b_log("\n\t---- MADT ----\n");

		if (length >= 44) {
			uint32_t a = (unsigned)LE32(&madt[36]);

			b_log("\tLocal Interrupt Controller at %08X\n", a);
			information.local_apic_base = a;

			if ((madt[40] & 1) != 0) {
				b_log("\tPC-AT Dual-8259 Setup\n");
				information.dual_8259_setup = 1;
			}

			madt += 44;
			length -= 44;
		} else {
			length = 0;
		}

		while (length >= 2) {
			unsigned type = (unsigned)madt[0];
			unsigned len = (unsigned)madt[1];

			if (len > length)
				break;

			switch (type) {
			case 0:
				if (len < 8)
					break;
				b_log("\tProcessor Local APIC (");
				b_log("UID %02X, ", (unsigned)madt[2]);
				b_log("ID %02X, ", (unsigned)madt[3]);

				if ((madt[4] & 1) != 0)
					b_log("Enabled)\n");
				else
					b_log("Disabled)\n");

				information.num_cpu_core += 1;
				if (information.max_apic_id < madt[3])
					information.max_apic_id = madt[3];
				break;
			case 1:
				if (len < 12)
					break;
				b_log("\tI/O APIC (");
				b_log("ID %02X, ", (unsigned)madt[2]);
				b_log("Addr %08lX, ", LE32(&madt[4]));
				b_log("Base %08lX)\n", LE32(&madt[8]));

				information.num_io_apic += 1;
				if (information.max_io_apic_id < madt[2])
					information.max_io_apic_id = madt[2];
				break;
			case 2:
				if (len < 10)
					break;
				b_log("\tInterrupt Override (");
				b_log("Source %02X, ", (unsigned)madt[3]);
				b_log("Global %08lX, ", LE32(&madt[4]));
				b_log("Flags %04lX)\n", LE16(&madt[8]));
				break;
			case 3:
				if (len < 8)
					break;
				b_log("\tNMI Source (");
				b_log("Flags %04lX, ", LE16(&madt[2]));
				b_log("Global %08lX)\n", LE32(&madt[4]));
				break;
			case 4:
				if (len < 6)
					break;
				b_log("\tLocal APIC NMI (");
				b_log("UID %02X, ", (unsigned)madt[2]);
				b_log("Flags %04lX, ", LE16(&madt[3]));
				b_log("LINT# %02X)\n", (unsigned)madt[5]);
				break;
			case 5:
				if (len < 12)
					break;
				b_log("\tLocal APIC Override (");
				b_log("Address %08lX", LE32(&madt[8]));
				b_log("%08lX)\n", LE32(&madt[4]));
#ifdef DANCY_64
				{
					phys_addr_t a = LE32(&madt[4]);
					phys_addr_t b = LE32(&madt[8]);
					phys_addr_t c = (a + (b << 32));

					information.local_apic_base = c;
				}
#endif
				break;
			default:
				b_log("\tType %02X, Length %u\n", type, len);
				break;
			}

			madt += len;
			length -= len;
		}
	}

	/*
	 * Find values from MCFG table.
	 */
	if (information.mcfg_addr) {
		const uint8_t *mcfg = (const uint8_t *)information.mcfg_addr;
		length = (uint32_t)LE32(mcfg + 4);

		b_log("\n\t---- MCFG ----\n");

		if (length >= 44) {
			mcfg += 44;
			length -= 44;
		} else {
			length = 0;
		}

		while (length >= 16) {
			b_log("\tBase %08lX", LE32(&mcfg[4]));
			b_log("%08lX ", LE32(&mcfg[0]));

			b_log("Group %04X ", LE16(&mcfg[8]));
			b_log("Start %02X ", (unsigned)mcfg[10]);
			b_log("End %02X\n", (unsigned)mcfg[11]);

			mcfg += 16;
			length -= 16;
		}
	}

	/*
	 * Find values from HPET table.
	 */
	if (information.hpet_addr) {
		const uint8_t *hpet = (const uint8_t *)information.hpet_addr;
		length = (uint32_t)LE32(hpet + 4);

		b_log("\n\t---- HPET ----\n");

		if (length >= 56) {
			uint32_t a, b;
			phys_addr_t c;

			a = (unsigned)LE32(&hpet[36]);
			b_log("\tEvent Timer Block ID %08X\n", a);
			information.hpet_block_id = a;

			a = (unsigned)LE16(&hpet[53]);
			b_log("\tMinimum Clock Tick %04X\n", a);
			information.hpet_min_tick = a;

			a = (unsigned)LE32(&hpet[44]);
			b = (unsigned)LE32(&hpet[48]);
			b_log("\tBase %08X%08X\n", b, a);

			c = (((phys_addr_t)b << 16) << 16) | (phys_addr_t)a;
			information.hpet_base = c;
#ifdef DANCY_32
			if (b) {
				information.hpet_addr = 0;
				information.hpet_base = 0;
				information.hpet_block_id = 0;
				information.hpet_min_tick = 0;
			}
#endif
		}
	}

	b_log("\n");
	return (init = 1), &information;
}

int acpi_get_apic(unsigned idx, struct acpi_apic *apic)
{
	const uint8_t *madt = (const uint8_t *)information.madt_addr;
	uint32_t length;
	unsigned i;

	memset(apic, 0, sizeof(*apic));

	if (!madt)
		return 1;

	length = (uint32_t)LE32(madt + 4);
	if (length < 44)
		return 1;

	madt += 44;
	length -= 44;

	for (i = 0; length >= 2; /* void */) {
		unsigned type = (unsigned)madt[0];
		unsigned len = (unsigned)madt[1];

		if (len > length)
			break;

		/*
		 * Processor Local APIC Structure.
		 */
		if (type == 0 && len >= 8) {
			if (i == idx) {
				apic->id = (uint32_t)madt[3];
				apic->enabled = (int)(madt[4] & 1);
				return 0;
			}
			i += 1;
		}

		madt += len;
		length -= len;
	}

	return 1;
}

int acpi_get_io_apic(unsigned idx, struct acpi_io_apic *io_apic)
{
	const uint8_t *madt = (const uint8_t *)information.madt_addr;
	uint32_t length;
	unsigned i;
	int r = 1;

	memset(io_apic, 0, sizeof(*io_apic));

	if (!madt)
		return 1;

	length = (uint32_t)LE32(madt + 4);
	if (length < 44)
		return 1;

	madt += 44;
	length -= 44;

	/*
	 * Identity-mapped IRQs unless overridden.
	 */
	for (i = 0; i < 16; i++) {
		io_apic->irq[i].global_int = i;
		io_apic->irq[i].flags = 0;
	}

	for (i = 0; length >= 2; /* void */) {
		unsigned type = (unsigned)madt[0];
		unsigned len = (unsigned)madt[1];

		if (len > length)
			break;

		/*
		 * I/O APIC Structure.
		 */
		if (type == 1 && len >= 12) {
			if (i == idx) {
				io_apic->id = (uint32_t)madt[2];
				io_apic->base_int = (uint32_t)LE32(&madt[8]);
				io_apic->addr = (phys_addr_t)LE32(&madt[4]);
				r = 0;
			}
			i += 1;
		}

		/*
		 * Interrupt Source Override Structure.
		 */
		if (type == 2 && len >= 10) {
			unsigned src = (unsigned)madt[3];
			struct acpi_irq_override *irq;

			if (src < 16) {
				irq = &io_apic->irq[src];
				irq->global_int = (uint32_t)LE32(&madt[4]);
				irq->flags = (uint32_t)LE16(&madt[8]);
			}
		}

		madt += len;
		length -= len;
	}

	return r;
}
