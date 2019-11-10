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

unsigned long b_output_string(const char *str, unsigned int len)
{
	size_t size = (len != 0) ? len : (size_t)strlen(str);

	if (video_active)
		video_output_string(str, len, 0, 0);
	else
		u_print("%.*s", size, str);
	return (unsigned long)size;
}

unsigned long b_output_string_hl(const char *str, unsigned int len)
{
	size_t size = (len != 0) ? len : (size_t)strlen(str);

	if (video_active) {
		video_output_string(str, len, 1, 0);
	} else {
		u_set_colors(0x70);
		u_print("%.*s", size, str);
		u_set_colors(0x07);
	}
	return (unsigned long)size;
}

unsigned long b_output_control(unsigned int cursor, unsigned int ctl)
{
	if (ctl == B_CLEAR_CONSOLE) {
		if (video_active)
			video_clear(1);
		else
			u_clear_screen();
		return 0;
	}
	if (ctl == B_GET_CURSOR) {
		uint64_t col, row;

		if (video_active) {
			col = video_column;
			row = video_row;
		} else {
			u_get_cursor(&col, &row);
		}
		return B_CURSOR(col, row);
	}
	if (ctl == B_SET_CURSOR) {
		uint64_t col = cursor & 0xFFull;
		uint64_t row = (cursor >> 8) & 0xFFull;

		if (video_active) {
			if (col >= video_columns)
				col = video_columns - 1;
			if (row >= video_rows)
				row = video_rows - 1;
			video_column = (uint32_t)col;
			video_row = (uint32_t)row;
		} else {
			u_set_cursor(col, row);
			u_get_cursor(&col, &row);
		}
		return B_CURSOR(col, row);
	}
	return 0;
}

unsigned long b_get_keycode(void)
{
	return key_get_code();
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

unsigned long b_get_structure(void *addr, unsigned int num)
{
	if (num == 0) {
		struct b_video_info *info = addr;
		return (unsigned long)video_get_info(info);
	}
	if (num == 1) {
		struct b_video_edid *edid = addr;
		return (unsigned long)video_get_edid(edid);
	}
	if (num == 2) {
		struct b_time *out = addr;
		size_t size = sizeof(struct b_time);
		EFI_TIME efi_time;
		EFI_STATUS s;

		memset(out, 0, size);
		memset(&efi_time, 0, sizeof(EFI_TIME));

		s = gSystemTable->RuntimeServices->GetTime(&efi_time, NULL);

		if (s != EFI_SUCCESS)
			return 0;

		out->year   = efi_time.Year;
		out->month  = efi_time.Month;
		out->day    = efi_time.Day;
		out->hour   = efi_time.Hour;
		out->minute = efi_time.Minute;
		out->second = efi_time.Second;

		return (unsigned long)size;
	}
	if (num == 3) {
		struct b_uefi_info *out = addr;
		size_t size = sizeof(struct b_uefi_info);

		memset(out, 0, size);
		out->runtime_services = (void *)gSystemTable->RuntimeServices;
		return (unsigned long)size;
	}
	return 0;
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
	u_stall(500);
	return 0;
}

unsigned long b_exit(void)
{
	EFI_STATUS s;

	if (memory_update_map() || memory_export_map(1))
		syscall_halt();

	s = gSystemTable->BootServices->ExitBootServices(
		gImageHandle, gMapkey);

	if (s != EFI_SUCCESS) {
		u_print("\nExitBootServices: error %016llX\n", s);
		syscall_halt();
	}

	return syscall_exit();
}
