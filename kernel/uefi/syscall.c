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
 * uefi/syscall.c
 *      Boot loader system calls (UEFI)
 */

#include <uefi.h>

static unsigned long not_implemented(const char *name)
{
	u_print("%s\n", name);
	while (gSystemTable->BootServices->Stall(1000000) == EFI_SUCCESS) { }
	return 0;
}

unsigned long b_output_string(const char *arg1, unsigned int arg2)
{
	size_t size = (arg2 != 0) ? arg2 : (unsigned long)strlen(arg1);

	u_print("%.*s", size, arg1);
	return (unsigned long)size;
}

unsigned long b_output_string_hl(const char *arg1, unsigned int arg2)
{
	return not_implemented("b_output_string_hl");
}

unsigned long b_output_control(unsigned int arg1, unsigned int arg2)
{
	return not_implemented("b_output_control");
}

unsigned long b_get_keycode(void)
{
	return not_implemented("b_get_keycode");
}

unsigned long b_get_byte_com1(void)
{
	return serial_get_byte(1);
}

unsigned long b_put_byte_com1(unsigned char b)
{
	return serial_put_byte(1, b);
}

unsigned long b_get_byte_com2(void)
{
	return serial_get_byte(2);
}

unsigned long b_put_byte_com2(unsigned char b)
{
	return serial_put_byte(2, b);
}

unsigned long b_get_parameter(unsigned int num)
{
	unsigned long r = 0;

	if (num == B_ACPI_POINTER) {
		EFI_GUID guid1 = EFI_ACPI_TABLE_GUID;
		EFI_GUID guid2 = ACPI_TABLE_GUID;;
		EFI_CONFIGURATION_TABLE *t;
		uint64_t i, j, vt;

		for (i = 0; i < gSystemTable->NumberOfTableEntries; i++) {
			t = &gSystemTable->ConfigurationTable[i];

			if (t->VendorGuid.Data1 != guid1.Data1)
				continue;
			if (t->VendorGuid.Data2 != guid1.Data2)
				continue;
			if (t->VendorGuid.Data3 != guid1.Data3)
				continue;
			for (j = 0; j < 8; j++) {
				if (t->VendorGuid.Data4[j] != guid1.Data4[j])
					continue;
			}
			vt = (uint64_t)t->VendorTable;
			if (vt < 0x100000000ull)
				return (unsigned long)vt;
		}

		for (i = 0; i < gSystemTable->NumberOfTableEntries; i++) {
			t = &gSystemTable->ConfigurationTable[i];

			if (t->VendorGuid.Data1 != guid2.Data1)
				continue;
			if (t->VendorGuid.Data2 != guid2.Data2)
				continue;
			if (t->VendorGuid.Data3 != guid2.Data3)
				continue;
			for (j = 0; j < 8; j++) {
				if (t->VendorGuid.Data4[j] != guid2.Data4[j])
					continue;
			}
			vt = (uint64_t)t->VendorTable;
			if (vt < 0x100000000ull)
				return (unsigned long)vt;
		}
		return r;
	}

	switch (num) {
	case B_BYTES_PER_BLOCK:
		r = b_bytes_per_block;
		break;
	case B_TOTAL_BLOCKS:
		r = b_total_blocks;
		break;
	case B_HIDDEN_BLOCKS:
		r = b_hidden_blocks;
		break;
	case B_DRIVE_NUMBER:
		r = b_drive_number;
		break;
	case B_MEDIA_CHANGED:
		r = b_media_changed;
		break;
	case B_A20_STATE:
		r = B_A20_AUTOMATIC;
		break;
	default:
		break;
	}
	return r;
}

unsigned long b_get_structure(void *arg1, unsigned int arg2)
{
	return not_implemented("b_get_structure");
}

unsigned long b_set_read_buffer(void *addr, unsigned int size)
{
	return block_set_read_buffer(addr, size);
}

unsigned long b_read_blocks(unsigned int lba, unsigned int blocks)
{
	return block_read_blocks(lba, blocks);
}

unsigned long b_set_write_buffer(void *addr, unsigned int size)
{
	return block_set_write_buffer(addr, size);
}

unsigned long b_write_blocks(unsigned int lba, unsigned int blocks)
{
	return block_write_blocks(lba, blocks);
}

unsigned long b_pause(void)
{
	return not_implemented("b_pause");
}

unsigned long b_exit(void)
{
	return not_implemented("b_exit");
}

unsigned long b_get_time(void *arg1)
{
	return not_implemented("b_get_time");
}
