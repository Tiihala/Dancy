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
 * base/heap.c
 *      Heap-based kernel memory manager
 */

#include <dancy.h>

static mtx_t heap_mtx;

static uint32_t *heap_map;
static size_t heap_map_size;

static int heap_map_entries;

int heap_init(void)
{
	static int run_once;
	size_t heap_size;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	heap_size = (size_t)(kernel->stack_array_addr - kernel->heap_addr);

	if (heap_size < 0x100000)
		return DE_MEMORY;

	if (heap_size > 0x10000000)
		return DE_UNEXPECTED;

	if ((heap_size & 0xFFF) != 0 || (kernel->heap_addr & 0xFFF) != 0)
		return DE_UNEXPECTED;

	heap_map = (uint32_t *)kernel->heap_addr;
	heap_map_size = 0x1000;

	memset(heap_map, 0, heap_map_size);

	heap_map[0] = (uint32_t)(kernel->heap_addr + heap_map_size);
	heap_map[1] = (uint32_t)(kernel->stack_array_addr);
	heap_map_entries = 2;

	if (mtx_init(&heap_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	return 0;
}

static void *heap_aligned_alloc(size_t alignment, size_t size)
{
	const uint32_t used_bit = 1;
	uint32_t map_used;
	addr_t addr = 0;
	int map_insert = 0;
	int i;

	if (alignment == 0 || alignment > 0x1000)
		return NULL;

	if ((alignment & (alignment - 1)) != 0)
		return NULL;

	if (alignment < 0x10)
		alignment = 0x10;

	if (size == 0 || size > 0x10000000)
		return NULL;

	/*
	 * The value of i is used after this for loop.
	 */
	for (i = heap_map_entries - 1; i > 0; i--) {
		uint32_t b = heap_map[i - 1];

		if ((b & used_bit) == 0) {
			uint32_t e = heap_map[i] & (~used_bit);
			uint32_t remainder;

			if (b >= e)
				return NULL;

			if (size > e - b)
				continue;

			remainder = (uint32_t)((e - size) % alignment);

			if (size + remainder <= e - b) {
				addr = (addr_t)(e - size - remainder);
				if (addr != b)
					map_insert = 1;
				break;
			}
		}
	}

	if (addr == 0)
		return NULL;

	map_used = (uint32_t)((int)(sizeof(uint32_t)) * heap_map_entries);

	if (map_used >= heap_map_size) {
		addr_t heap_map_addr = (addr_t)heap_map;

		if (map_used > heap_map_size)
			return NULL;

		if ((heap_map[0] & used_bit) != 0)
			return NULL;

		if ((heap_map[1] - heap_map[0]) < 0x2000)
			return NULL;

		heap_map_size += 0x1000;
		heap_map[0] = (uint32_t)(heap_map_addr + heap_map_size);
	}

	if (map_insert) {
		size_t s = sizeof(uint32_t) * (size_t)(heap_map_entries - i);

		memmove(&heap_map[i + 1], &heap_map[i], s);
		heap_map_entries += 1;
	} else {
		i -= 1;
	}

	heap_map[i] = (uint32_t)addr | used_bit;

	return (void *)addr;
}

static void heap_free(const void *ptr)
{
	const uint32_t used_bit = 1;
	uint32_t a = (uint32_t)((addr_t)ptr) | used_bit;
	int i;

	for (i = 0; i < heap_map_entries - 1; i++) {
		uint32_t b = heap_map[i];

		if (a == b) {
			heap_map[i] = b & (~used_bit);
			break;
		}
	}

	i = 1;

	while (i < heap_map_entries - 1) {
		uint32_t b = heap_map[i - 1];
		uint32_t e = heap_map[i];
		size_t s;

		if ((b & used_bit) != 0 || (e & used_bit) != 0) {
			i += 1;
			continue;
		}

		s = sizeof(uint32_t) * (size_t)(heap_map_entries - i);

		memmove(&heap_map[i], &heap_map[i + 1], s);
		heap_map[--heap_map_entries] = 0;
	}
}

void *heap_alloc_static_page(void)
{
	const uint32_t used_bit = 1;
	const size_t size = 0x1000;
	addr_t heap_map_addr = (addr_t)heap_map;
	void *ptr;

	if (mtx_lock(&heap_mtx) != thrd_success)
		return NULL;

	if ((heap_map[0] & used_bit) != 0) {
		mtx_unlock(&heap_mtx);
		return NULL;
	}

	if ((heap_map[1] - heap_map[0]) < 0x2000) {
		mtx_unlock(&heap_mtx);
		return NULL;
	}

	ptr = heap_map;
	heap_map_addr += (addr_t)size;

	memmove((void *)heap_map_addr, ptr, heap_map_size);

	heap_map = (uint32_t *)heap_map_addr;
	heap_map[0] = (uint32_t)(heap_map_addr + heap_map_size);

	mtx_unlock(&heap_mtx);

	return memset(ptr, 0, size);
}

void *aligned_alloc(size_t alignment, size_t size)
{
	void *ptr;

	if (mtx_lock(&heap_mtx) != thrd_success)
		return NULL;

	ptr = heap_aligned_alloc(alignment, size);

	mtx_unlock(&heap_mtx);

	return ptr;
}

void *calloc(size_t nmemb, size_t size)
{
	size_t mem_size = nmemb * size;
	void *ptr;

	if (mem_size == 0 || (mem_size / nmemb) != size)
		return NULL;

	if (mtx_lock(&heap_mtx) != thrd_success)
		return NULL;

	ptr = heap_aligned_alloc(16, mem_size);

	mtx_unlock(&heap_mtx);

	if (ptr)
		memset(ptr, 0, mem_size);

	return ptr;
}

void *malloc(size_t size)
{
	void *ptr;

	if (mtx_lock(&heap_mtx) != thrd_success)
		return NULL;

	ptr = heap_aligned_alloc(16, size);

	mtx_unlock(&heap_mtx);

	return ptr;
}

void free(void *ptr)
{
	if (!ptr)
		return;

	if (mtx_lock(&heap_mtx) != thrd_success)
		return;

	heap_free(ptr);

	mtx_unlock(&heap_mtx);
}
