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
 * kernel/table.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_TABLE_H
#define KERNEL_TABLE_H

#include <dancy/types.h>

struct kernel_table {
	size_t table_size;

	addr_t base_addr;
	addr_t heap_addr;

	addr_t stack_array_addr;
	size_t stack_array_size;

	void (*panic)(const char *message);
	void (*print)(const char *format, ...);

	/*
	 * Symmetric Multiprocessing (SMP).
	 */
	int smp_ap_count;
	uint32_t *smp_ap_id;

	/*
	 * Advanced Programmable Interrupt Controller (APIC)
	 */
	int apic_enabled;
	phys_addr_t apic_base_addr;

	uint32_t apic_bsp_id;

	/*
	 * I/O Advanced Programmable Interrupt Controller (I/O APIC).
	 */
	int io_apic_enabled;
	int io_apic_count;

	struct {
		uint32_t id;
		uint32_t base_int;
		phys_addr_t addr;
	} *io_apic;

	struct {
		uint32_t global_int;
		uint32_t flags;
	} io_apic_override[16];

	/*
	 * Time-Stamp Counter (TSC).
	 */
	uint64_t delay_tsc_hz;

	/*
	 * Framebuffer set by system firmware.
	 */
	uint32_t fb_mode;
	uint32_t fb_width;
	uint32_t fb_height;
	uint32_t fb_stride;
	phys_addr_t fb_addr;

	/*
	 * Standard framebuffer with 4-byte pixels.
	 */
	addr_t fb_standard_addr;
	size_t fb_standard_size;

	/*
	 * System memory map.
	 */
	size_t memory_map_size;

	struct {
		uint32_t type;
		uint32_t reserved;
		uint64_t base;
		uint64_t efi_attributes;
	} *memory_map;
};

#endif
