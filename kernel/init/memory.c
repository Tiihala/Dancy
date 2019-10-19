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
 * init/memory.c
 *      Simple memory manager for pre-kernel environment
 */

#include <dancy.h>

static size_t memory_free_end;
static size_t memory_entries;
static void *memory_map;

#define TYPE_BOOT       (0x01u)
#define TYPE_INIT       (0x02u)
#define TYPE_LOW_END    (0x04u)
#define TYPE_ALL        (0x07u)

int memory_init(void *map)
{
	const char *err = "Error: boot loader gave a corrupted memory map";
	const struct b_mem *memory = map;
	const size_t max = (2048 - 512);
	uint32_t continuous_normal = 0;
	unsigned memory_types = 0;
	int check_run = 0;
	size_t i;

	/*
	 * This function should be called with "map == NULL" when the init
	 * procedures have been finished. It will be an extra check to make
	 * sure that any memory map modifications (see malloc and others)
	 * do not make the map corrupted. In theory, this could detect some
	 * physical memory errors (but it is very unlikely).
	 *
	 * This can also be used for checking that there are still enough
	 * continuous free memory areas available for the kernel.
	 */
	if (map == NULL) {
		err = "Error: the memory map is corrupted";
		memory = map = memory_map;
		check_run = 1;
	} else {
		memory_free_end = 0, memory_entries = 0;
		memory_map = NULL;
	}

	/*
	 * The boot loader is responsible for providing a valid memory map.
	 * There are no error recovery strategies because the whole computer
	 * state is unknown if we cannot trust the memory map.
	 *
	 * The firmware (BIOS or UEFI) gave the original memory map but the
	 * boot loader has processed it, e.g. sorting, merging, etc.
	 */
	if (memory[0].base)
		return b_print("%s\n", err), 1;

	for (i = 0; (i == 0 || memory[i].base); i++) {
		phys_addr_t b = memory[i].base;
		phys_addr_t e = memory[i + 1].base;
		uint32_t t = memory[i].type;

		if (b > e - 1 || i == max)
			return b_print("%s\n", err), 1;

		if (t == B_MEM_NORMAL && e <= 0xFFFFFFFFul) {
			uint32_t size = (uint32_t)(e - b);
			if (size > continuous_normal) {
				continuous_normal = size;
				if (!check_run)
					memory_free_end = i + 1;
			}

		} else if (t == B_MEM_BOOT_LOADER) {
			memory_types |= TYPE_BOOT;

		} else if (t == B_MEM_INIT_EXECUTABLE) {
			phys_addr_t init_b = (phys_addr_t)memory - 0x10000ul;
			phys_addr_t size = e - b;

			if (init_b != b || b & 0xFFFFul || size != 0x20000ul)
				return b_print("%s\n", err), 1;
			memory_types |= TYPE_INIT;
		}

		if (e - 1 == 0xFFFFFFFFul) {
			memory_types |= TYPE_LOW_END;
			if ((memory[i + 1].flags & B_FLAG_VALID_LEGACY) == 0)
				return b_print("%s\n", err), 1;
			if ((memory[i + 2].flags & B_FLAG_VALID_LEGACY) != 0)
				return b_print("%s\n", err), 1;
		}
	}

	if (memory_types != TYPE_ALL)
		return b_print("%s\n", err), 1;

	for (/* void */; memory[i].flags; i++) {
		if (i == max)
			return b_print("%s\n", err), 1;
	}

	if (memory[i].base)
		return b_print("%s\n", err), 1;

	if (check_run && memory_entries != i)
		return b_print("%s\n", err), 1;

	memory_entries = i;
	return (memory_map = map), 0;
}

void memory_print_map(void (*print)(const char *, ...))
{
	const struct b_mem *memory = memory_map;
	phys_addr_t total = 0;
	size_t i;

	(*print)("Memory Map\n");

	for (i = 0; i == 0 || memory[i].base; i++) {
		uint32_t t = memory[i].type;
		phys_addr_t b = memory[i].base;
		phys_addr_t e = memory[i + 1].base - 1;
		const char *desc = "(Unknown)";

		if (t == B_MEM_NORMAL)
			desc = "Free", total += memory[i + 1].base - b;
		else if (t == B_MEM_RESERVED)
			desc = "Reserved";
		else if (t == B_MEM_ACPI_RECLAIMABLE)
			desc = "Acpi Reclaimable";
		else if (t == B_MEM_ACPI_NVS)
			desc = "Acpi NVS";
		else if (t == B_MEM_BOOT_LOADER)
			desc = "Loader Runtime (Reclaimable)";
		else if (t == B_MEM_INIT_EXECUTABLE)
			desc = "Executable (Init)";
		else if (t == B_MEM_NOT_REPORTED)
			desc = "Not Reported";

		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			unsigned a = (unsigned)t & 0x0000FFFFu;
			(*print)("    %p %p  Allocated (#%u)\n", b, e, a);
			continue;
		}

		if (t >= B_MEM_DATABASE_MIN && t <= B_MEM_DATABASE_MAX) {
			unsigned db = (unsigned)t & 0x0000FFFFu;
			(*print)("    %p %p  Database (#%u)\n", b, e, db);
		} else {
			(*print)("    %p %p  %s\n", b, e, desc);
		}
	}
	(*print)("\n    Total free: %zd KiB\n\n", total / 1024);
}

static void fix_memory_map(void)
{
	struct b_mem_raw *memory = memory_map;
	uint32_t a = 0;
	size_t i;

	for (i = 1; i < memory_entries; i++) {
		uint32_t t = memory[i].type;
		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			memory[i].type = B_MEM_INIT_ALLOC_MIN + a;
			a += 1;
		}
	}

	for (i = 1; i < memory_entries; /* void */) {
		struct b_mem_raw *m1 = &memory[i - 1];
		struct b_mem_raw *m2 = &memory[i];

		if (m2->base_low == 0 && m2->base_high == 1) {
			i += 1;
			continue;
		}

		if (m1->type == m2->type && m1->flags == m2->flags) {
			size_t size, other;

			size = sizeof(struct b_mem) * (memory_entries - i);
			other = sizeof(m1->other);

			if (size && !memcmp(&m1->other, &m2->other, other)) {
				memmove(&memory[i], &memory[i + 1], size);
				if (i < memory_free_end)
					memory_free_end -= 1;
				memory_entries -= 1;
				continue;
			}
		}
		i += 1;
	}
}

void *aligned_alloc(size_t alignment, size_t size)
{
	struct b_mem *memory = memory_map;
	phys_addr_t addr = 0;
	size_t b, e;
	size_t i;

	if (alignment >= 0x00400000ul || size == 0ul || size >= 0x7FFFFFFFul)
		return NULL;

	for (i = memory_free_end; i > 1; i--) {
		size_t remainder = 0;

		b = (size_t)memory[i - 1].base;
		e = (size_t)memory[i].base;

		if (memory[i - 1].type != B_MEM_NORMAL || size > e - b)
			continue;
		if (memory[i - 1].flags & B_FLAG_NO_INIT_ALLOC)
			continue;
		if (!(memory[i - 1].flags & B_FLAG_VALID_LEGACY))
			continue;

		if (alignment)
			remainder = (e - size) % alignment;
		if (size + remainder <= e - b) {
			addr = (phys_addr_t)(e - size - remainder);
			break;
		}
	}

	if (addr == 0 || memory_entries <= i || memory_entries >= 2040)
		return NULL;

	size = sizeof(struct b_mem) * (memory_entries - i);
	memmove(&memory[i + 1], &memory[i], size);
	memmove(&memory[i], &memory[i - 1], sizeof(struct b_mem));
	memory_free_end += 1, memory_entries += 1;

	memory[i].type = B_MEM_INIT_ALLOC_MIN;
	memory[i].base = addr;

	fix_memory_map();
	return (void *)addr;
}

void *calloc(size_t nmemb, size_t size)
{
	size_t total = nmemb * size;
	void *ptr;

	if (!total || total / nmemb != size)
		return NULL;
	if ((ptr = malloc(total)) != NULL)
		memset(ptr, 0, total);
	return ptr;
}

void *malloc(size_t size)
{
	if (size >= 0x7FFFFFFFul)
		return NULL;
	return aligned_alloc(16, (size + 15ul) & 0xFFFFFFF0ul);
}

void *realloc(void *ptr, size_t size)
{
	struct b_mem *memory = memory_map;
	phys_addr_t addr = (phys_addr_t)ptr;
	int found = 0;
	size_t aligned_size;
	size_t old_size, i;

	if (ptr == NULL)
		return malloc(size);

	for (i = 1; i < memory_free_end; i++) {
		uint32_t t = memory[i].type;
		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			if (memory[i].base == addr) {
				found = 1;
				break;
			}
		}
	}

	if (!found || size >= 0x7FFFFFFFul)
		return NULL;
	if (!size)
		size = 1;

	aligned_size = (size + 15ul) & 0xFFFFFFF0ul;
	old_size = (size_t)(memory[i + 1].base - memory[i].base);

	if (old_size < size) {
		void *new_ptr = malloc(size);
		if (new_ptr == NULL)
			return NULL;
		memcpy(new_ptr, ptr, old_size);
		return free(ptr), new_ptr;
	}

	if (old_size - size < 64)
		return ptr;
	if (memory_entries <= i || memory_entries >= 2040)
		return ptr;

	size = sizeof(struct b_mem) * (memory_entries - i);
	memmove(&memory[i + 1], &memory[i], size);
	memory_free_end += 1, memory_entries += 1;

	memory[i + 1].type = B_MEM_NORMAL;
	memory[i + 1].base += (phys_addr_t)aligned_size;

	fix_memory_map();
	return ptr;
}

void free(void *ptr)
{
	struct b_mem *memory = memory_map;
	phys_addr_t addr = (phys_addr_t)ptr;
	uint32_t t;
	size_t i;

	if (ptr == NULL)
		return;

	for (i = 1; (memory[i].flags & B_FLAG_VALID_LEGACY); i++) {
		if (memory[i].base != addr)
			continue;
		t = memory[i].type;

		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			memory[i].type = B_MEM_NORMAL;
			break;
		}
	}
	fix_memory_map();
}
