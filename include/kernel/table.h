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

	uint32_t smp_ap_count;
	uint32_t *smp_ap_id;

	uint64_t delay_tsc_hz;

	uint32_t fb_mode;
	uint32_t fb_width;
	uint32_t fb_height;
	uint32_t fb_stride;
	phys_addr_t fb_addr;

	addr_t fb_standard_addr;
	size_t fb_standard_size;
};

#endif
