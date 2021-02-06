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
 * base/pg.c
 *      Page translation and protection
 */

#include <dancy.h>

cpu_native_t pg_kernel;

static mtx_t pg_mtx;
static int pg_ready;

static addr_t pg_apic_base_vaddr;

static phys_addr_t pg_alloc_page(void)
{
	const size_t size = 0x1000;
	void *page;

	page = aligned_alloc(size, size);
	memset(page, 0, size);

	return (phys_addr_t)page;
}

#ifdef DANCY_32

static int pg_map_identity(phys_addr_t addr, int type, int large_page)
{
	uint32_t page_bits = 0x23;
	int offset = (int)(addr >> 22);
	uint32_t page, *ptr;

	/*
	 * Page-directory table.
	 */
	ptr = (uint32_t *)pg_kernel;

	if (large_page) {
		if ((ptr[offset] & 1) != 0 && (ptr[offset] & 0x80) == 0)
			return 1;

		page = (uint32_t)(addr & 0xFFC00000);

		if ((type & pg_uncached) != 0)
			page_bits |= 0x18;

		if (page < 0x10000000)
			page_bits |= 0x100;

		page_bits |= 0x80;
		ptr[offset] = page | page_bits;

		return 0;
	}

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint32_t)pg_alloc_page()) == 0)
			return 1;
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint32_t)(addr & 0xFFFFF000);
	ptr = (uint32_t *)(ptr[offset] & 0xFFFFF000);

	if ((type & pg_uncached) != 0)
		page_bits |= 0x18;

	if (page < 0x10000000)
		page_bits |= 0x100;

	offset = (int)((addr >> 12) & 0x3FF);

	if (page == pg_apic_base_vaddr)
		ptr[offset] = (uint32_t)kernel->apic_base_addr | page_bits;
	else
		ptr[offset] = page | page_bits | (ptr[offset] & 0x18);

	return 0;
}

static const phys_addr_t pg_unit_size = 0x400000;

#endif

#ifdef DANCY_64

static int pg_map_identity(phys_addr_t addr, int type, int large_page)
{
	uint64_t page_bits = 0x23;
	int pml4e_offset = (int)(addr >> 39);
	int pdpe_offset = (int)((addr >> 30) & 0x1FF);
	int offset = (int)((addr >> 21) & 0x1FF);
	uint64_t page, *ptr;

	if (pml4e_offset > 0x1FF)
		return 1;

	/*
	 * Page-map-level-4 table.
	 */
	ptr = (uint64_t *)pg_kernel;

	if ((ptr[pml4e_offset] & 1) == 0) {
		if ((page = (uint64_t)pg_alloc_page()) == 0)
			return 1;
		ptr[pml4e_offset] = page | page_bits;
	}

	/*
	 * Page-directory-pointer table.
	 */
	ptr = (uint64_t *)(ptr[pml4e_offset] & 0xFFFFF000);

	if ((ptr[pdpe_offset] & 1) == 0) {
		if ((page = (uint64_t)pg_alloc_page()) == 0)
			return 1;
		ptr[pdpe_offset] = page | page_bits;
	}

	/*
	 * Page-directory table.
	 */
	ptr = (uint64_t *)(ptr[pdpe_offset] & 0xFFFFF000);

	if (large_page) {
		if ((ptr[offset] & 1) != 0 && (ptr[offset] & 0x80) == 0)
			return 1;

		page = (uint64_t)(addr & 0xFFFFFFFFFFE00000ull);

		if ((type & pg_uncached) != 0)
			page_bits |= 0x18;

		if (page < 0x10000000)
			page_bits |= 0x100;

		page_bits |= 0x80;
		ptr[offset] = page | page_bits;

		return 0;
	}

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint64_t)pg_alloc_page()) == 0)
			return 1;
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint64_t)(addr & 0xFFFFFFFFFFFFF000ull);
	ptr = (uint64_t *)(ptr[offset] & 0xFFFFF000);

	if ((type & pg_uncached) != 0)
		page_bits |= 0x18;

	if (page < 0x10000000)
		page_bits |= 0x100;

	offset = (int)((addr >> 12) & 0x1FF);

	if (page == pg_apic_base_vaddr)
		ptr[offset] = (uint64_t)kernel->apic_base_addr | page_bits;
	else
		ptr[offset] = page | page_bits | (ptr[offset] & 0x18);

	return 0;
}

static const phys_addr_t pg_unit_size = 0x200000;

#endif

int pg_init(void)
{
	static int run_once;

	size_t map_size = kernel->memory_map_size;
	int map_entries = (int)(map_size / sizeof(kernel->memory_map[0]));
	int i, r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&pg_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if ((pg_kernel = (cpu_native_t)pg_alloc_page()) == 0)
		return DE_MEMORY;

	pg_apic_base_vaddr = kernel->apic_base_vaddr;
	if (pg_apic_base_vaddr == 0 || (pg_apic_base_vaddr & 0x0FFF) != 0)
		return DE_UNEXPECTED;

	/*
	 * Create a special mapping for Local APIC registers.
	 */
	pg_map_kernel((phys_addr_t)pg_apic_base_vaddr, 0x1000, pg_uncached);

	/*
	 * Map the framebuffer (identity mapping).
	 */
	if (kernel->fb_height) {
		size_t size = (size_t)(kernel->fb_height * kernel->fb_stride);

		pg_map_kernel(kernel->fb_addr, size, pg_normal);
	}

	for (i = 0; i < map_entries - 1; i++) {
		const uint32_t type_kernel = 0xE0000000;
		const uint32_t type_kernel_reserved = 0xE000FFFF;

		uint32_t type = kernel->memory_map[i].type;
		uint64_t base, next, size;

		if (type != type_kernel && type != type_kernel_reserved)
			continue;

		base = kernel->memory_map[i + 0].base;
		next = kernel->memory_map[i + 1].base;

		if (next != 0 && base > next)
			return DE_UNEXPECTED;
#ifdef DANCY_32
		if (next > 0x100000000ull)
			break;
#endif
		size = next - base;
		pg_map_kernel((phys_addr_t)base, (size_t)size, pg_normal);
	}

	cpu_write32((uint32_t *)&pg_ready, 1);

	r = cpu_ints(0);
	cpu_write_cr4(cpu_read_cr4() | (1u << 7) | (1u << 4));
	cpu_write_cr3(pg_kernel);

	/*
	 * Update the timer interrupt handler (I/O APIC)
	 * and restore the interrupt flag.
	 */
	{
		uint32_t a, *p;

		a = (uint32_t)kernel->apic_base_vaddr;
		p = (uint32_t *)((addr_t)&timer_apic_base[0]);
		cpu_write32(p, a);
#ifdef DANCY_64
		a = (uint32_t)(kernel->apic_base_vaddr >> 32);
		p = (uint32_t *)((addr_t)&timer_apic_base[4]);
		cpu_write32(p, a);
#endif
		cpu_ints(r);
	}

	return 0;
}

int pg_init_ap(void)
{
	while (cpu_read32((const uint32_t *)&pg_ready) == 0)
		delay(1000000);

	if (pg_kernel == 0 || (pg_kernel & 0x0FFF) != 0)
		return DE_UNEXPECTED;

	cpu_write_cr4(cpu_read_cr4() | (1u << 7) | (1u << 4));
	cpu_write_cr3(pg_kernel);

	return 0;
}

void *pg_map_kernel(phys_addr_t addr, size_t size, int type)
{
	const phys_addr_t page_mask = 0x0FFF;
	const phys_addr_t unit_mask = pg_unit_size - 1;
	phys_addr_t addr_beg, addr_end, addr_sub;

	if (size == 0 || addr > (SIZE_MAX - size) + 1)
		return NULL;

	addr_beg = addr & (~page_mask);
	addr_end = ((phys_addr_t)(addr + size) + page_mask) & (~page_mask);

	if (addr_beg == 0)
		addr_beg += 0x1000;

	if (mtx_lock(&pg_mtx) != thrd_success)
		return NULL;

	while ((addr_sub = addr_end - addr_beg) != 0) {
		if ((addr_beg & unit_mask) == 0 && addr_sub >= pg_unit_size) {
			if (!pg_map_identity(addr_beg, type, 1)) {
				addr_beg += pg_unit_size;
				continue;
			}
		}

		if (pg_map_identity(addr_beg, type, 0)) {
			mtx_unlock(&pg_mtx);
			return NULL;
		}

		addr_beg += 0x1000;
	}

	mtx_unlock(&pg_mtx);

	cpu_write_cr3(cpu_read_cr3());

	return (void *)addr;
}
