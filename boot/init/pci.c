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
 * init/pci.c
 *      Peripheral Component Interconnect
 */

#include <init.h>

uint32_t pci_device_count;
struct pci_device *pci_devices;

static uint32_t pci_device_count_maximum;

static int pci_device_found(phys_addr_t ecam, int gbdf[4], uint32_t id_pair)
{
	static int zero_vendor_count;
	uint32_t vendor_id = (id_pair & 0xFFFFu);
	uint32_t device_id = (id_pair >> 16);
	int i = (int)pci_device_count;

	if (!pci_device_count)
		zero_vendor_count = 0;

	if (pci_device_count >= pci_device_count_maximum)
		return 1;

	/*
	 * It is very likely that vendor 0x0000 is not valid. If
	 * there is more than one of them, return 1. It might be
	 * that the "ecam" method reads zero-initialized memory.
	 */
	if (!vendor_id) {
		if (zero_vendor_count++)
			return 1;
	}

	pci_devices[i].ecam   = ecam;
	pci_devices[i].group  = gbdf[0];
	pci_devices[i].bus    = gbdf[1];
	pci_devices[i].device = gbdf[2];
	pci_devices[i].func   = gbdf[3];

	pci_devices[i].vendor_id = vendor_id;
	pci_devices[i].device_id = device_id;

	{
		uint32_t class_code = pci_read(&pci_devices[i], 8) >> 8;
		uint32_t type = (pci_read(&pci_devices[i], 12) >> 16) & 0x7Fu;

		pci_devices[i].class_code = class_code;
		pci_devices[i].header_type = (int)type;
	}

	pci_device_count += 1;

	return 0;
}

static uint32_t pci_read32(int b, int d, int f, int off)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000u | ((uint32_t)off & 0xFCu);

	addr |= (((uint32_t)f & 0x07u) << 8);
	addr |= (((uint32_t)d & 0x1Fu) << 11);
	addr |= (((uint32_t)b & 0xFFu) << 16);

	cpu_out32(pci_config_addr, addr);
	return cpu_in32(pci_config_data);
}

static void pci_write32(int b, int d, int f, int off, uint32_t val)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000u | ((uint32_t)off & 0xFCu);

	addr |= (((uint32_t)f & 0x07u) << 8);
	addr |= (((uint32_t)d & 0x1Fu) << 11);
	addr |= (((uint32_t)b & 0xFFu) << 16);

	cpu_out32(pci_config_addr, addr);
	cpu_out32(pci_config_data, val);
}

static int pci_enumerate_bus(int b)
{
	int gbdf[4];
	uint32_t id_pair;
	int d, f;

	gbdf[0] = 0, gbdf[1] = b;

	for (d = 0; d < 32; d++) {
		id_pair = pci_read32(b, d, 0, 0);

		if ((id_pair & 0xFFFFu) == 0xFFFFu)
			continue;

		gbdf[2] = d, gbdf[3] = 0;

		if (pci_device_found(0, gbdf, id_pair))
			return 1;

		if ((pci_read32(b, d, 0, 12) & 0x00800000u) == 0)
			continue;

		for (f = 1; f < 8; f++) {
			id_pair = pci_read32(b, d, f, 0);

			if ((id_pair & 0xFFFFu) == 0xFFFFu)
				continue;

			gbdf[3] = f;

			if (pci_device_found(0, gbdf, id_pair))
				return 1;
		}
	}

	return 0;
}

static int pci_enumerate_ecam(phys_addr_t base, int g, int b_beg, int b_end)
{
	int gbdf[4];
	uint32_t id_pair;
	int b, d, f;

	gbdf[0] = g;

	for (b = b_beg; b <= b_end; b++) {
		gbdf[1] = b;

		for (d = 0; d < 32; d++) {
			phys_addr_t ecam = base;
			const uint32_t *e;

			ecam |= (phys_addr_t)(((unsigned)b & 0xFFu) << 20);
			ecam |= (phys_addr_t)(((unsigned)d & 0x1Fu) << 15);
			e = (const uint32_t *)ecam;

			id_pair = cpu_read32(&e[0]);

			if ((id_pair & 0xFFFFu) == 0xFFFFu)
				continue;

			gbdf[2] = d, gbdf[3] = 0;

			if (pci_device_found(ecam, gbdf, id_pair))
				return 1;

			if ((cpu_read32(&e[3]) & 0x00800000u) == 0)
				continue;

			for (f = 1; f < 8; f++) {
				ecam += 0x1000;
				e = (const uint32_t *)ecam;

				id_pair = cpu_read32(&e[0]);

				if ((id_pair & 0xFFFFu) == 0xFFFFu)
					continue;

				gbdf[3] = f;

				if (pci_device_found(ecam, gbdf, id_pair))
					return 1;
			}
		}
	}

	return 0;
}

static int pci_enumerate(int early)
{
	const struct acpi_information *acpi = acpi_get_information();
	const unsigned char *mcfg;
	uint32_t length;
	int b;

	pci_device_count = 0;

	if (acpi != NULL && acpi->mcfg_addr != 0) {
		mcfg = (const unsigned char *)acpi->mcfg_addr;
		length = (uint32_t)LE32(mcfg + 4);

		if (length >= 44) {
			mcfg += 44;
			length -= 44;
		} else {
			length = 0;
		}

		while (length >= 16) {
			phys_addr_t base_lo = (phys_addr_t)LE32(&mcfg[0]);
			phys_addr_t base_hi = (phys_addr_t)LE32(&mcfg[4]);
			phys_addr_t base = ((base_hi << 16) << 16) | base_lo;

			int g = (int)LE16(&mcfg[8]);
			int b_beg = (int)mcfg[10];
			int b_end = (int)mcfg[11];
#ifdef DANCY_64
			if (early && base_hi) {
				pci_device_count = 0;
				return 0;
			}
#else
			if (base_hi) {
				pci_device_count = 0;
				break;
			}
#endif
			if (base == 0 || (base & 0xFFFFFu) != 0) {
				pci_device_count = 0;
				break;
			}

			if (pci_enumerate_ecam(base, g, b_beg, b_end)) {
				pci_device_count = 0;
				break;
			}

			mcfg += 16;
			length -= 16;
		}

		if (pci_device_count)
			return 0;
	}

	for (b = 0; b < 256; b++) {
		if (pci_enumerate_bus(b)) {
			pci_device_count = 0;
			return (early == 0);
		}
	}

	return 0;
}

static const char *pci_enumerate_error = "Error: pci_enumerate failure";

int pci_init_early(void)
{
	size_t size;
	uint32_t i;

	pci_device_count_maximum = 4096;

	size = (size_t)pci_device_count_maximum * sizeof(struct pci_device);

	if ((pci_devices = malloc(size)) == NULL) {
		b_print("Error: pci_init_early could not allocate memory\n");
		return 1;
	}

	memset(pci_devices, 0, size);

	if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
		return 0;

	if (pci_enumerate(1)) {
		b_print("%s\n", pci_enumerate_error);
		return 1;
	}

	if (!pci_device_count)
		return 0;

	b_log("Peripheral Component Interconnect (PCI)\n");

	for (i = 0; i < pci_device_count; i++) {
		b_log("\t%04X:%02X:%02X.%X - %04X %04X Class %06X Type %02X",
			(unsigned)pci_devices[i].group,
			(unsigned)pci_devices[i].bus,
			(unsigned)pci_devices[i].device,
			(unsigned)pci_devices[i].func,
			(unsigned)pci_devices[i].vendor_id,
			(unsigned)pci_devices[i].device_id,
			(unsigned)pci_devices[i].class_code,
			(unsigned)pci_devices[i].header_type);

		if (pci_devices[i].ecam)
			b_log(" ECAM %p", (const void *)pci_devices[i].ecam);
		b_log("\n");
	}

	b_log("\n");

	return 0;
}

void pci_init(void)
{
	uint32_t i;

	if (pci_device_count == 0 && pci_enumerate(0) != 0)
		panic(pci_enumerate_error);

	for (i = 0; i < pci_device_count; i++) {
		const struct pci_device *pci = &pci_devices[i];

		if (!pci->ecam)
			break;

		pg_map_uncached((void *)pci->ecam);
	}
}

uint32_t pci_read(const struct pci_device *pci, int off)
{
	uint32_t ret;

	if (off < 0 || (off & 3) != 0)
		return 0;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)off;
		ret = cpu_read32((const uint32_t *)a);
	} else {
		int b = pci->bus, d = pci->device, f = pci->func;
		ret = pci_read32(b, d, f, off);
	}

	return ret;
}

void pci_write(struct pci_device *pci, int off, uint32_t val)
{
	if (off < 0 || (off & 3) != 0)
		return;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)off;
		cpu_write32((uint32_t *)a, val);
	} else {
		int b = pci->bus, d = pci->device, f = pci->func;
		pci_write32(b, d, f, off, val);
	}
}
