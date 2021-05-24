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
 * osdancy.c
 *      Dancy Operating System Services Layer for ACPICA
 */

#include <dancy.h>
#include "acpi.h"
#include "accommon.h"

int acpios_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	return 0;
}

ACPI_STATUS AcpiOsReadMemory(
	ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
	phys_addr_t addr = (phys_addr_t)Address;
	uint64_t val;

	if ((ACPI_PHYSICAL_ADDRESS)addr != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		val = pg_read_memory(addr, 1);
	else if (Width == 16)
		val = pg_read_memory(addr, 2);
	else if (Width == 32)
		val = pg_read_memory(addr, 4);
	else if (Width == 64)
		val = pg_read_memory(addr, 8);
	else
		return (AE_BAD_PARAMETER);

	if (Value)
		*Value = (UINT64)val;

	return (AE_OK);
}

ACPI_STATUS AcpiOsWriteMemory(
	ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
	phys_addr_t addr = (phys_addr_t)Address;
	uint64_t val = (uint64_t)Value;

	if ((ACPI_PHYSICAL_ADDRESS)addr != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		pg_write_memory(addr, val, 1);
	else if (Width == 16)
		pg_write_memory(addr, val, 2);
	else if (Width == 32)
		pg_write_memory(addr, val, 4);
	else if (Width == 64)
		pg_write_memory(addr, val, 8);
	else
		return (AE_BAD_PARAMETER);

	return (AE_OK);
}

ACPI_STATUS AcpiOsReadPort(
	ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width)
{
	uint16_t port = (uint16_t)Address;
	uint32_t val;

	if ((ACPI_IO_ADDRESS)port != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		val = cpu_in8(port);
	else if (Width == 16)
		val = cpu_in16(port);
	else if (Width == 32)
		val = cpu_in32(port);
	else
		return (AE_BAD_PARAMETER);

	if (Value)
		*Value = (UINT32)val;

	return (AE_OK);
}

ACPI_STATUS AcpiOsWritePort(
	ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	uint16_t port = (uint16_t)Address;

	if ((ACPI_IO_ADDRESS)port != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		cpu_out8(port, (uint8_t)Value);
	else if (Width == 16)
		cpu_out16(port, (uint16_t)Value);
	else if (Width == 32)
		cpu_out32(port, (uint32_t)Value);
	else
		return (AE_BAD_PARAMETER);

	return (AE_OK);
}
