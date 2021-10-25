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
 * base/mm.c
 *      Physical memory manager
 */

#include <dancy.h>

static mtx_t mm_mtx;
static int mm_ready;

static size_t mm_bitmap_size;
static size_t mm_bitmap_used;
static uint8_t *mm_bitmap;

static void mm_bitmap_clear(size_t page_frame, size_t page_count)
{
	unsigned int b = (unsigned int)(page_frame & 7u);
	size_t i = (page_frame >> 3);

	while (page_count--) {
		unsigned int val = mm_bitmap[i];

		mm_bitmap[i] = (uint8_t)(val & (~(1u << b)));

		if ((b = (b + 1) & 7u) == 0)
			i += 1;
	}
}

static void mm_bitmap_set(size_t page_frame, size_t page_count)
{
	unsigned int b = (unsigned int)(page_frame & 7u);
	size_t i = (page_frame >> 3);

	while (page_count--) {
		unsigned int val = mm_bitmap[i];

		mm_bitmap[i] = (uint8_t)(val | (1u << b));

		if ((b = (b + 1) & 7u) == 0)
			i += 1;
	}
}

static int mm_bitmap_test(size_t page_frame, size_t page_count)
{
	unsigned int b = (unsigned int)(page_frame & 7u);
	size_t i = (page_frame >> 3);

	while (page_count--) {
		unsigned int val = mm_bitmap[i];

		if ((val & (1u << b)) != 0)
			return 1;

		if ((b = (b + 1) & 7u) == 0)
			i += 1;
	}

	return 0;
}

static int mm_get_count(int order)
{
	static const int table[11] = {
		0x0001, 0x0002, 0x0004, 0x0008,
		0x0010, 0x0020, 0x0040, 0x0080,
		0x0100, 0x0200, 0x0400
	};

	if (order >= 0 && order < 11)
		return table[order];

	return 0;
}

int mm_init(void)
{
	static int run_once;

	struct {
		size_t page_frame;
		size_t page_count;
	} *mm_array;
	int mm_count = 0;

	size_t map_size = kernel->memory_map_size;
	int map_entries = (int)(map_size / sizeof(kernel->memory_map[0]));
	void *heap_reserved, *ptr;
	int i;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&mm_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	mm_array = calloc((size_t)map_entries, sizeof(mm_array[0]));

	if (!mm_array)
		return DE_MEMORY;

	for (i = 0; i < map_entries - 1; i++) {
		const uint32_t type_kernel_reserved = 0xE000FFFF;
		const uint64_t page_mask = 0x0FFF;
		uint64_t base, next, size;

		if (kernel->memory_map[i].type != type_kernel_reserved)
			continue;

		base = kernel->memory_map[i + 0].base;
		next = kernel->memory_map[i + 1].base;

		if (next != 0 && base > next)
			return DE_UNEXPECTED;

		if (base < 0x10000)
			base = 0x10000;

		base = (base + page_mask) & (~page_mask);
		next = (next & (~page_mask));

#ifdef DANCY_32
		if (next > 0x100000000ull)
			next = 0x100000000ull;
#else
		if (next > 0x800000000000ull)
			next = 0x800000000000ull;
#endif
		if (base >= next)
			continue;

		if ((size = next - base) < 0x2000)
			continue;

		/*
		 * Both base and size have 4096-byte alignment.
		 */
		mm_array[mm_count].page_frame = (size_t)(base / 0x1000);
		mm_array[mm_count].page_count = (size_t)(size / 0x1000);

		mm_count += 1;
	}

	/*
	 * The bitmap will cover at least memory from 0 to 0x0FFFFFFF.
	 */
	mm_bitmap_size = 0x2000;

	for (i = 0; i < mm_count; i++) {
		const size_t page_mask = 0x0FFF;
		const size_t seven = 7, eight = 8;
		size_t size = mm_array[i].page_frame + mm_array[i].page_count;

		size = ((size + seven) & (~seven)) / eight;
		size = ((size + page_mask) & (~page_mask));

		if (mm_bitmap_size < size)
			mm_bitmap_size = size;
	}

#ifdef DANCY_32
	mm_bitmap = aligned_alloc(0x1000, mm_bitmap_size);
#else
	for (i = mm_count - 1; i >= 0; i--) {
		size_t bitmap_page_count = mm_bitmap_size / 0x1000;

		size_t page_frame = mm_array[i].page_frame;
		size_t page_count = mm_array[i].page_count;
		phys_addr_t addr;

		/*
		 * Check the size of the block and avoid having
		 * equal sizes (an "untested" corner case).
		 */
		if (page_count <= bitmap_page_count)
			continue;

		page_frame += bitmap_page_count;
		page_count -= bitmap_page_count;

		addr = (phys_addr_t)(page_frame * 0x1000);

		/*
		 * Make sure that extended mapping is possible. The limit
		 * is arbitrary, but clearly below the hard limit. This
		 * also avoids an extremely unlikely scenario: mapping the
		 * highest possible virtual addresses.
		 */
		if (addr > 0x7800000000ull)
			continue;

		/*
		 * On 64-bit systems, the bitmap size can be quite large.
		 */
		mm_bitmap = pg_map_kernel(addr, mm_bitmap_size, pg_extended);

		mm_array[i].page_frame = page_frame;
		mm_array[i].page_count = page_count;
		break;
	}
#endif
	if (!mm_bitmap)
		return free(mm_array), DE_MEMORY;

	memset(mm_bitmap, 0xFF, mm_bitmap_size);

	/*
	 * Verify that the bitmap really has all pages "allocated"
	 * initially. The memory manager would behave in a really
	 * bad way if some bits were hardwired to value zero.
	 */
	{
		size_t t0 = 0, t1 = mm_bitmap_size - 1;

		/*
		 * Prevent reading any cached results. The compiler
		 * must assume that cpu_read8 could change the whole
		 * bitmap content, although they do nothing.
		 */
		(void)cpu_read8(mm_bitmap + t0), cpu_wbinvd();
		(void)cpu_read8(mm_bitmap + t1), cpu_wbinvd();

		while (t0 <= t1) {
			if (mm_bitmap[t0] != 0xFF) {
				char msg[64];

				snprintf(&msg[0], sizeof(msg),
					"mm_init: address %p unavailable",
					&mm_bitmap[t0]);

				kernel->panic(&msg[0]);
			}
			t0 += 1;
		}
	}

	/*
	 * Clear all bitmap bits that refer to free pages.
	 */
	for (i = 0; i < mm_count; i++) {
		size_t page_frame = mm_array[i].page_frame;
		size_t page_count = mm_array[i].page_count;

		mm_bitmap_clear(page_frame, page_count);
	}

	free(mm_array);

	/*
	 * Give almost all heap memory to the physical memory manager.
	 */
	if ((heap_reserved = aligned_alloc(0x1000, 0x400000)) != NULL) {
		while ((ptr = aligned_alloc(0x1000, 0x400000)) != NULL) {
			phys_addr_t addr = (phys_addr_t)ptr;

			size_t page_frame = (size_t)(addr / 0x1000);
			size_t page_count = 0x400;

			mm_bitmap_clear(page_frame, page_count);
		}

		free(heap_reserved);
	}

	cpu_write32((uint32_t *)&mm_ready, 1);

	return 0;
}

size_t mm_available_pages(int type)
{
	size_t page_frame = mm_bitmap_size * 8;
	size_t pages = 0;
	size_t i;

	if (!mm_ready)
		return 0;

	if (type != mm_normal) {
		if (type == mm_addr32) {
			if (page_frame > 0x100000)
				page_frame = 0x100000;

		} else if (type == mm_kernel) {
			if (page_frame > 0x10000)
				page_frame = 0x10000;

		} else if (type == mm_legacy) {
			if (page_frame > 0x1000)
				page_frame = 0x1000;

		} else /* unknown memory type */ {
			return 0;
		}
	}

	if (mtx_lock(&mm_mtx) != thrd_success)
		return 0;

	for (i = 0; i < (page_frame >> 3); i++) {
		unsigned int val = mm_bitmap[i];
		unsigned int b;

		for (b = 0; b < 8; b++) {
			if ((val & (1u << b)) == 0)
				pages += 1;
		}
	}

	mtx_unlock(&mm_mtx);

	return pages;
}

phys_addr_t mm_alloc_page(void)
{
	size_t page_frame, size;
	phys_addr_t page = 0;

	if (!mm_ready)
		return 0;

	if (mtx_lock(&mm_mtx) != thrd_success)
		return 0;

	size = mm_bitmap_size - mm_bitmap_used;
	page_frame = (size * 8);

	while (size != 0 && mm_bitmap[--size] == 0xFF) {
		mm_bitmap_used += 1;
		page_frame -= 8;
	}

	if (page_frame)
		page_frame = (page_frame - 1);

	while (page_frame) {
		unsigned int b = (unsigned int)(page_frame & 7u);
		unsigned int t = 1u << b;
		size_t i = (page_frame >> 3);
		unsigned int val = mm_bitmap[i];

		if ((val & t) == 0) {
			mm_bitmap[i] = (uint8_t)(val | t);
			page = page_frame * 0x1000;
			break;
		}

		page_frame -= 1;
	}

	mtx_unlock(&mm_mtx);

	return page;
}

phys_addr_t mm_alloc_pages(int type, int order)
{
	size_t page_count = (size_t)mm_get_count(order);
	size_t page_frame, size;
	phys_addr_t page = 0;

	if (!mm_ready || !page_count)
		return 0;

	if (mtx_lock(&mm_mtx) != thrd_success)
		return 0;

	size = mm_bitmap_size - mm_bitmap_used;
	page_frame = (size * 8);

	while (size != 0 && mm_bitmap[--size] == 0xFF) {
		mm_bitmap_used += 1;
		page_frame -= 8;
	}

	if (type != mm_normal) {
		if (type == mm_addr32) {
			if (page_frame > 0x100000)
				page_frame = 0x100000;

		} else if (type == mm_kernel) {
			if (page_frame > 0x10000)
				page_frame = 0x10000;

		} else if (type == mm_legacy) {
			if (page_frame > 0x1000)
				page_frame = 0x1000;

		} else /* unknown memory type */ {
			page_frame = 0;
		}
	}

	if (page_frame)
		page_frame = (page_frame - 1) & (~(page_count - 1));

	while (page_frame) {
		if (!mm_bitmap_test(page_frame, page_count)) {
			mm_bitmap_set(page_frame, page_count);
			page = page_frame * 0x1000;
			break;
		}

		page_frame -= page_count;
	}

	mtx_unlock(&mm_mtx);

	return page;
}

void mm_free_page(phys_addr_t addr)
{
	size_t page_frame = (size_t)(addr >> 12);
	unsigned int b;
	size_t i;

	if (!mm_ready || !page_frame)
		return;

	if ((i = page_frame / 8) >= mm_bitmap_size)
		return;

	if (mtx_lock(&mm_mtx) != thrd_success)
		return;

	b = (unsigned int)(page_frame & 7u);
	mm_bitmap[i] = (uint8_t)(mm_bitmap[i] & (~(1u << b)));

	if (mm_bitmap_used > (i = (mm_bitmap_size - i) - 1))
		mm_bitmap_used = i;

	mtx_unlock(&mm_mtx);
}

void mm_free_pages(phys_addr_t addr, int order)
{
	size_t page_frame = (size_t)(addr >> 12);
	size_t page_count = (size_t)mm_get_count(order);
	size_t i;

	if (!mm_ready || !page_count)
		return;

	if (!page_frame || (page_frame & (page_count - 1)) != 0)
		return;

	if ((i = (page_frame + (page_count - 1)) / 8) >= mm_bitmap_size)
		return;

	if (mtx_lock(&mm_mtx) != thrd_success)
		return;

	mm_bitmap_clear(page_frame, page_count);

	if (mm_bitmap_used > (i = (mm_bitmap_size - i) - 1))
		mm_bitmap_used = i;

	mtx_unlock(&mm_mtx);
}
