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
 * init/pg.c
 *      Page translation and protection
 */

#include <init.h>

volatile uint32_t pg_fault_counter;
volatile uint32_t pg_tables_size;

static phys_addr_t pg_addr;

static phys_addr_t alloc_table_page(void)
{
	static const size_t buffer_size = 0x40000;
	static unsigned char *buffer = NULL;
	static size_t size = 0;
	void *page;

	if (size == 0) {
		buffer = aligned_alloc(buffer_size, buffer_size);
		if (!buffer)
			return 0;
		memset(buffer, 0, buffer_size);
		pg_tables_size += buffer_size;

		size = buffer_size;
	}

	page = &buffer[buffer_size - size];
	size -= 0x1000;

	return (phys_addr_t)page;
}

#ifdef DANCY_32

static int identity_map(phys_addr_t addr, int cached, int single_page)
{
	uint32_t page_bits = 0x23;
	uint32_t page_dir_offset = (uint32_t)(addr >> 22);
	uint32_t i = 0, page, *ptr;

	/*
	 * Page-directory table.
	 */
	ptr = (uint32_t *)pg_addr;

	if ((ptr[page_dir_offset] & 1) == 0) {
		if ((page = (uint32_t)alloc_table_page()) == 0)
			return 1;
		ptr[page_dir_offset] = page | page_bits;
	}

	/*
	 * Page table.
	 */
	page = (uint32_t)(addr & 0xFFC00000);
	ptr = (uint32_t *)(ptr[page_dir_offset] & 0xFFFFF000);

	if (page_dir_offset == 0)
		i = 1, page += 0x1000;

	if (!cached)
		page_bits |= 0x18;

	if (!single_page) {
		while (i < 1024) {
			ptr[i++] = page | page_bits;
			page += 0x1000;
		}

	} else {
		int offset = (int)((addr >> 12) & 0x3FF);

		page = (uint32_t)(addr & 0xFFFFF000);
		ptr[offset] = page | page_bits;
	}

	return 0;
}

static phys_addr_t map_unit = 0x400000;

#endif

#ifdef DANCY_64

static int identity_map(phys_addr_t addr, int cached, int single_page)
{
	uint64_t page_bits = 0x23;
	uint64_t pml4e_offset = (uint64_t)(addr >> 39);
	uint64_t pdpe_offset = (uint64_t)((addr >> 30) & 0x1FF);
	uint64_t pde_offset = (uint64_t)((addr >> 21) & 0x1FF);
	uint64_t i = 0, page, *ptr;

	if (pml4e_offset > 0x1FF)
		return 1;

	/*
	 * Page-map-level-4 table.
	 */
	ptr = (uint64_t *)pg_addr;

	if ((ptr[pml4e_offset] & 1) == 0) {
		if ((page = (uint64_t)alloc_table_page()) == 0)
			return 1;
		ptr[pml4e_offset] = page | page_bits;
	}

	/*
	 * Page-directory-pointer table.
	 */
	ptr = (uint64_t *)(ptr[pml4e_offset] & 0xFFFFF000);

	if ((ptr[pdpe_offset] & 1) == 0) {
		if ((page = (uint64_t)alloc_table_page()) == 0)
			return 1;
		ptr[pdpe_offset] = page | page_bits;
	}

	/*
	 * Page-directory table.
	 */
	ptr = (uint64_t *)(ptr[pdpe_offset] & 0xFFFFF000);

	if ((ptr[pde_offset] & 1) == 0) {
		if ((page = (uint64_t)alloc_table_page()) == 0)
			return 1;
		ptr[pde_offset] = page | page_bits;
	}

	/*
	 * Page table.
	 */
	page = (uint64_t)(addr & 0xFFFFFFFFFFE00000ull);
	ptr = (uint64_t *)(ptr[pde_offset] & 0xFFFFF000);

	if (pde_offset == 0 && pdpe_offset == 0 && pml4e_offset == 0)
		i = 1, page += 0x1000;

	if (!cached)
		page_bits |= 0x18;

	if (!single_page) {
		while (i < 512) {
			ptr[i++] = page | page_bits;
			page += 0x1000;
		}

	} else {
		int offset = (int)((addr >> 12) & 0x1FF);

		page = (uint64_t)(addr & 0xFFFFFFFFFFFFF000ull);
		ptr[offset] = page | page_bits;
	}

	return 0;
}

static phys_addr_t map_unit = 0x200000;

#endif

int pg_init(void)
{
	phys_addr_t high_addr = (phys_addr_t)memory_map + 0x10000;
	phys_addr_t addr = 0;

	high_addr &= (~(map_unit - 1));

	if ((pg_addr = alloc_table_page()) == 0)
		return 1;

	while (addr < high_addr) {
		if (identity_map(addr, 1, 0))
			return 1;
		addr += map_unit;
	}

	if (identity_map(high_addr, 1, 0))
		return 1;

	pg_switch(pg_addr);

	return 0;
}

int pg_handler(void)
{
	phys_addr_t addr;

	pg_fault_counter += 1;
	pg_get_fault(&addr);

	if (addr < 0x1000 || identity_map(addr, 1, 0))
		return 1;

	pg_switch(pg_addr);

	return 0;
}

void pg_map_uncached(void *addr)
{
	if ((phys_addr_t)addr < 0x1000)
		return;

	identity_map((phys_addr_t)addr, 0, 1);

	pg_switch(pg_addr);
}
