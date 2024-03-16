/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/stdlib/alloc.c
 *      The memory management functions
 */

#include <__dancy/spin.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <threads.h>

static mtx_t alloc_mtx;

static size_t *alloc_map;
static size_t alloc_map_size;
static int alloc_map_count;

static void *alloc_zero;
static void *alloc_high;

static size_t alloc_state[2];
static int alloc_init_state[2];

static int internal_map_pages(void *addr, size_t size)
{
	void *r;

	r = mmap(addr, size, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);

	return (r != addr) ? 1 : 0;
}

static void internal_unmap_pages(void *addr, size_t size)
{
	(void)munmap(addr, size);
}

static void internal_alloc_init(void)
{
	static int run_once;
	size_t addr;
	void *m0, *m1;

	if (!__dancy_spin_trylock(&run_once))
		return;

	addr = 0x10;
	alloc_zero = (void *)addr;

	addr = 0x80001000;
	m0 = (void *)addr;

	if (internal_map_pages(m0, 0x1000)) {
		__dancy_spin_trylock(&alloc_init_state[1]);
		return;
	}

	addr = 0x7FFF0000;
	addr = (addr << 16);
	addr = (addr | 0xFFFFE000);
	m1 = (void *)addr;

	alloc_high = (void *)(addr + 0x1000);

	if (internal_map_pages(m1, 0x1000)) {
		__dancy_spin_trylock(&alloc_init_state[1]);
		return;
	}

	alloc_state[0] = (size_t)m0;
	alloc_state[1] = (size_t)m1;

	alloc_map = m0;
	alloc_map_size = 0x1000;

	alloc_map[0] = alloc_state[0] + 0x1000;
	alloc_map[1] = alloc_state[1] + 0x1000;
	alloc_map_count = 2;

	if (mtx_init(&alloc_mtx, mtx_plain) != thrd_success) {
		__dancy_spin_trylock(&alloc_init_state[1]);
		return;
	}

	__dancy_spin_trylock(&alloc_init_state[0]);
}

static void *internal_aligned_alloc(size_t alignment, size_t size)
{
	const size_t used_bit = 1;
	size_t addr = 0, map_used;
	int map_insert = 0;
	int i;

	if (alignment == 0 || alignment > 0x400000 || size == 0)
		return NULL;

	if ((alignment & (alignment - 1)) != 0)
		return NULL;

	if (alignment < 0x10)
		alignment = 0x10;

	/*
	 * The value of i is used after this for loop.
	 */
	for (i = alloc_map_count - 1; i > 0; i--) {
		size_t b = alloc_map[i - 1];

		if ((b & used_bit) == 0) {
			size_t e = alloc_map[i] & (~used_bit);
			size_t remain;

			if (b >= e)
				return NULL;

			if (size > e - b)
				continue;

			remain = (e - size) % alignment;

			if (size + remain <= e - b) {
				addr = e - size - remain;
				if (addr != b)
					map_insert = 1;
				break;
			}
		}
	}

	if (addr == 0 || alloc_map_count > 0x08000000)
		return NULL;

	if (addr < alloc_state[1]) {
		const size_t m = 0x0FFF;
		size_t aligned_addr = addr & (~m);
		size_t aligned_size = alloc_state[1] - aligned_addr;

		if (internal_map_pages((void *)aligned_addr, aligned_size))
			return NULL;

		alloc_state[1] = aligned_addr;
	}

	map_used = sizeof(size_t) * (size_t)alloc_map_count;

	if (map_used >= alloc_map_size) {
		size_t alloc_map_addr = (size_t)alloc_map;

		if (map_used > alloc_map_size)
			return NULL;

		if ((alloc_map[0] & used_bit) != 0)
			return NULL;

		if ((alloc_map[1] - alloc_map[0]) < 0x2000)
			return NULL;

		if (internal_map_pages(alloc_map + alloc_map_count, 0x1000))
			return NULL;

		alloc_map_size += 0x1000;
		alloc_map[0] = alloc_map_addr + alloc_map_size;
	}

	if (map_insert) {
		size_t s = sizeof(size_t) * (size_t)(alloc_map_count - i);

		memmove(&alloc_map[i + 1], &alloc_map[i], s);
		alloc_map_count += 1;
	} else {
		i -= 1;
	}

	alloc_map[i] = addr | used_bit;

	return (void *)addr;
}

static void internal_free(const void *ptr)
{
	const size_t used_bit = 1;
	size_t a = (size_t)ptr | used_bit;
	int i;

	for (i = 0; i < alloc_map_count - 1; i++) {
		size_t b = alloc_map[i];

		if (a == b) {
			alloc_map[i] = b & (~used_bit);
			break;
		}
	}

	i = 1;

	while (i < alloc_map_count - 1) {
		size_t b = alloc_map[i - 1];
		size_t e = alloc_map[i];
		size_t s;

		if ((b & used_bit) != 0 || (e & used_bit) != 0) {
			i += 1;
			continue;
		}

		s = sizeof(size_t) * (size_t)(alloc_map_count - i);

		memmove(&alloc_map[i], &alloc_map[i + 1], s);
		alloc_map[--alloc_map_count] = 0;
	}

	if ((alloc_map[0] & used_bit) == 0 && alloc_map[1] > alloc_state[1]) {
		const size_t m = 0x0FFF;
		size_t d = (alloc_map[1] - alloc_state[1]) & (~m);

		if (d > 0x4000) {
			d -= 0x2000;
			internal_unmap_pages((void *)alloc_state[1], d);
			alloc_state[1] += d;
		}
	}
}

void *aligned_alloc(size_t alignment, size_t size)
{
	void *ptr;

	while (alloc_init_state[0] == 0) {
		if (alloc_init_state[1] != 0)
			return NULL;
		internal_alloc_init();
	}

	if (size == 0)
		return NULL;

	if (mtx_lock(&alloc_mtx) != thrd_success)
		return NULL;

	ptr = internal_aligned_alloc(alignment, size);

	mtx_unlock(&alloc_mtx);

	if (ptr == NULL)
		errno = ENOMEM;

	return ptr;
}

void *calloc(size_t nmemb, size_t size)
{
	size_t total_size = nmemb * size;
	void *ptr;

	while (alloc_init_state[0] == 0) {
		if (alloc_init_state[1] != 0)
			return NULL;
		internal_alloc_init();
	}

	if (nmemb == 0 || size == 0)
		return alloc_zero;

	if (total_size == 0 || (total_size / nmemb) != size)
		return NULL;

	if (mtx_lock(&alloc_mtx) != thrd_success)
		return NULL;

	ptr = internal_aligned_alloc(16, total_size);

	mtx_unlock(&alloc_mtx);

	if (ptr == NULL)
		errno = ENOMEM;
	else
		memset(ptr, 0, total_size);

	return ptr;
}

void *malloc(size_t size)
{
	void *ptr;

	while (alloc_init_state[0] == 0) {
		if (alloc_init_state[1] != 0)
			return NULL;
		internal_alloc_init();
	}

	if (size == 0)
		return alloc_zero;

	if (mtx_lock(&alloc_mtx) != thrd_success)
		return NULL;

	ptr = internal_aligned_alloc(16, size);

	mtx_unlock(&alloc_mtx);

	if (ptr == NULL)
		errno = ENOMEM;

	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	size_t addr = (size_t)ptr;
	size_t copy_size = 0;
	void *new_ptr;

	while (alloc_init_state[0] == 0) {
		if (alloc_init_state[1] != 0)
			return NULL;
		internal_alloc_init();
	}

	if (ptr != NULL) {
		if (addr >= (size_t)alloc_high) {
			errno = EINVAL;
			return NULL;
		}

		copy_size = (size_t)alloc_high - addr;

		if (copy_size > size)
			copy_size = size;
	}

	if (mtx_lock(&alloc_mtx) != thrd_success)
		return NULL;

	if (size != 0) {
		new_ptr = internal_aligned_alloc(16, size);
		if (new_ptr != NULL && copy_size != 0)
			memcpy(new_ptr, ptr, copy_size);
	} else {
		new_ptr = alloc_zero;
	}

	if (new_ptr != NULL && ptr != NULL)
		internal_free(ptr);

	mtx_unlock(&alloc_mtx);

	if (new_ptr == NULL)
		errno = ENOMEM;

	return new_ptr;
}

void free(void *ptr)
{
	while (alloc_init_state[0] == 0) {
		if (alloc_init_state[1] != 0)
			return;
		internal_alloc_init();
	}

	if (ptr == NULL || ptr == alloc_zero)
		return;

	if (mtx_lock(&alloc_mtx) != thrd_success)
		return;

	internal_free(ptr);

	mtx_unlock(&alloc_mtx);
}
