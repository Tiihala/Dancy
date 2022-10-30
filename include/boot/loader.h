/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
 * boot/loader.h
 *      Header of Dancy Operating System
 */

#ifndef BOOT_LOADER_H
#define BOOT_LOADER_H

#include <common/types.h>

#if defined(DANCY_32) || defined(DANCY_64)

#define BOOT_LOADER_TYPE_BIOS   (0x00000000)
#define BOOT_LOADER_TYPE_UEFI   (0x00000001)

extern uint32_t boot_loader_type;

unsigned long b_output_string(const char *, unsigned int);
unsigned long b_output_string_hl(const char *, unsigned int);
unsigned long b_output_control(unsigned int, unsigned int);
unsigned long b_get_keycode(void);
unsigned long b_get_byte_com1(void);
unsigned long b_put_byte_com1(unsigned char);
unsigned long b_get_byte_com2(void);
unsigned long b_put_byte_com2(unsigned char);
unsigned long b_get_parameter(unsigned int);
unsigned long b_get_structure(void *, unsigned int);
unsigned long b_set_read_buffer(void *, unsigned int);
unsigned long b_read_blocks(unsigned int, unsigned int);
unsigned long b_set_write_buffer(void *, unsigned int);
unsigned long b_write_blocks(unsigned int, unsigned int);
unsigned long b_pause(void);
unsigned long b_exit(void);

enum b_ctl {
	B_CLEAR_CONSOLE,
	B_GET_CURSOR,
	B_SET_CURSOR
};

#define B_CURSOR(col,row) \
(((unsigned)(col) & 0xFFu) | ((unsigned)(row) << 8 & 0xFF00u))

enum b_parameter {
	B_BYTES_PER_BLOCK,
	B_TOTAL_BLOCKS,
	B_HIDDEN_BLOCKS,
	B_DRIVE_NUMBER,
	B_MEDIA_CHANGED,
	B_A20_STATE,
	B_DROPPED_BYTES_COM1,
	B_DROPPED_BYTES_COM2,
	B_DRIVE_PARAMETERS_80H,
	B_DRIVE_PARAMETERS_81H,
	B_DRIVE_PARAMETERS_82H,
	B_DRIVE_PARAMETERS_83H,
	B_ACPI_POINTER
};

enum b_structure {
	B_VIDEO_INFO,
	B_VIDEO_EDID,
	B_TIME,
	B_UEFI_INFO
};

struct b_video_info {
	uint32_t mode;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	phys_addr_t framebuffer;
#if defined(DANCY_64)
	uint8_t reserved[8];
#else
	uint8_t reserved[12];
#endif
};

struct b_video_edid {
	uint8_t edid[128];
};

enum b_mode {
	B_MODE_VGA,
	B_MODE_PALETTE,
	B_MODE_15_BIT,
	B_MODE_16_BIT,
	B_MODE_RED_GREEN_BLUE,
	B_MODE_BLUE_GREEN_RED,
	B_MODE_RED_GREEN_BLUE_RESERVED,
	B_MODE_BLUE_GREEN_RED_RESERVED
};

struct b_mem {
	uint32_t type;
	uint32_t flags;
	phys_addr_t base;
#if defined(DANCY_64)
	uint64_t efi_attributes;
	uint8_t reserved[8];
#else
	uint8_t reserved[20];
#endif
};

struct b_mem_raw {
	uint32_t type;
	uint32_t flags;
	uint32_t base_low;
	uint32_t base_high;
	uint8_t other[16];
};

struct b_uefi_info {
	const void *runtime_services;
	const char *log;
	size_t log_size;
};

#define B_MEM_EFI_RESERVED      (0x00000000)
#define B_MEM_EFI_LOADER_CODE   (0x00000001)
#define B_MEM_EFI_LOADER_DATA   (0x00000002)
#define B_MEM_EFI_BOOT_CODE     (0x00000003)
#define B_MEM_EFI_BOOT_DATA     (0x00000004)
#define B_MEM_EFI_RUNTIME_CODE  (0x00000005)
#define B_MEM_EFI_RUNTIME_DATA  (0x00000006)
#define B_MEM_NORMAL            (0x00000007)
#define B_MEM_RESERVED          (0x00000008)
#define B_MEM_ACPI_RECLAIMABLE  (0x00000009)
#define B_MEM_ACPI_NVS          (0x0000000A)
#define B_MEM_MAP_IO            (0x0000000B)
#define B_MEM_MAP_IO_PORT_SPACE (0x0000000C)
#define B_MEM_PAL_CODE          (0x0000000D)
#define B_MEM_PERSISTENT        (0x0000000E)

#define B_MEM_BOOT_LOADER       (0x80000004)
#define B_MEM_UEFI_SYSCALLS     (0x80000005)
#define B_MEM_INIT_EXECUTABLE   (0x80860000)
#define B_MEM_INIT_ALLOC_MIN    (0x80860001)
#define B_MEM_INIT_ALLOC_MAX    (0x8086FFFF)
#define B_MEM_DATABASE_MIN      (0xDB000000)
#define B_MEM_DATABASE_MAX      (0xDB00FFFF)
#define B_MEM_KERNEL            (0xE0000000)
#define B_MEM_KERNEL_RESERVED   (0xE000FFFF)
#define B_MEM_NOT_REPORTED      (0xFFFFFFFF)

#define B_FLAG_VALID_ENTRY      (1u << 0)
#define B_FLAG_BIOS_LOADER      (1u << 1)
#define B_FLAG_NO_INIT_ALLOC    (1u << 16)

#define B_A20_INT15H            (0x01)
#define B_A20_KEYBOARD          (0x02)
#define B_A20_FAST              (0x03)
#define B_A20_AUTOMATIC         (0x80)

struct b_time {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t reserved[9];
};

#endif
#endif
