/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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

#include <init.h>

size_t memory_entries;
void *memory_map;

mtx_t memory_mtx;
int (*memory_mtx_lock)(mtx_t *);
int (*memory_mtx_unlock)(mtx_t *);

#define TYPE_BOOT       (0x01u)
#define TYPE_INIT       (0x02u)
#define TYPE_LOW_END    (0x04u)
#define TYPE_ALL        (0x07u)

static int memory_mtx_nop(mtx_t *mtx)
{
	return (*mtx == NULL) ? thrd_success : thrd_error;
}

int memory_init(void *map)
{
	const char *err = "Error: boot loader gave a corrupted memory map";
	const struct b_mem *memory = map;
	const size_t max = (2048 - 512);
	unsigned memory_types = 0;
	size_t i;

	/*
	 * This memory_init function is the only one in this module that
	 * uses boot loader services.
	 *
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

		if (t == B_MEM_BOOT_LOADER) {
			memory_types |= TYPE_BOOT;

		} else if (t == B_MEM_UEFI_SYSCALLS) {
			memory_types |= TYPE_BOOT;

		} else if (t == B_MEM_INIT_EXECUTABLE) {
			phys_addr_t init_b = (phys_addr_t)memory - 0x10000ul;
			phys_addr_t size = e - b;

			if (init_b != b || b & 0xFFFFul || size != 0x20000ul)
				return b_print("%s\n", err), 1;
			memory_types |= TYPE_INIT;
		}

		if (e - 1 == 0xFFFFFFFFul)
			memory_types |= TYPE_LOW_END;
	}

	if (memory_types != TYPE_ALL)
		return b_print("%s\n", err), 1;

	for (/* void */; memory[i].flags; i++) {
		if (i == max)
			return b_print("%s\n", err), 1;
	}

	if (memory[i].base)
		return b_print("%s\n", err), 1;

	memory_entries = i;

	memory_mtx_lock = memory_mtx_nop;
	memory_mtx_unlock = memory_mtx_nop;

	return (memory_map = map), 0;
}

void memory_print_map(void (*print)(const char *, ...))
{
	static const struct { uint32_t type; const char *desc; } names[] = {
		{ B_MEM_EFI_RESERVED,      "EFI Reserved" },
		{ B_MEM_EFI_LOADER_CODE,   "EFI Loader Code" },
		{ B_MEM_EFI_LOADER_DATA,   "EFI Loader Data" },
		{ B_MEM_EFI_BOOT_CODE,     "EFI Boot Code" },
		{ B_MEM_EFI_BOOT_DATA,     "EFI Boot Data" },
		{ B_MEM_EFI_RUNTIME_CODE,  "EFI Runtime Code" },
		{ B_MEM_EFI_RUNTIME_DATA,  "EFI Runtime Data" },
		{ B_MEM_NORMAL,            "Free" },
		{ B_MEM_RESERVED,          "Reserved" },
		{ B_MEM_ACPI_RECLAIMABLE,  "Acpi Reclaimable" },
		{ B_MEM_ACPI_NVS,          "Acpi NVS" },
		{ B_MEM_MAP_IO,            "Map IO" },
		{ B_MEM_MAP_IO_PORT_SPACE, "Map IO Port Space" },
		{ B_MEM_PAL_CODE,          "Pal Code" },
		{ B_MEM_PERSISTENT,        "Persistent" },
		{ B_MEM_BOOT_LOADER,       "Loader Runtime" },
		{ B_MEM_UEFI_SYSCALLS,     "Loader Syscalls" },
		{ B_MEM_INIT_EXECUTABLE,   "Executable (Init)" },
		{ B_MEM_NOT_REPORTED,      "Not Reported" }
	};
	const struct b_mem *memory = memory_map;
	phys_addr_t total = 0;
	size_t i, j;

	if (memory_mtx_lock(&memory_mtx) != thrd_success)
		return;

	(*print)("Memory Map\n");

	for (i = 0; i == 0 || memory[i].base; i++) {
		uint32_t t = memory[i].type;
		phys_addr_t b = memory[i].base;
		phys_addr_t e = memory[i + 1].base - 1;
		phys_addr_t size = (e + 1) - b;
		const char *desc = "(Unknown)";

		for (j = 0; j < sizeof(names) / sizeof(names[0]); j++) {
			if (names[j].type == t) {
				desc = names[j].desc;
				break;
			}
		}

		if (t == B_MEM_NORMAL)
			total += memory[i + 1].base - b;

		(*print)("    %p %p  ", b, e);

		if (size < 1024 || (size < 4096 && (size % 1024) != 0))
			(*print)("%4d B  ", (int)size);
		else if ((size /= 1024) < 4096)
			(*print)("%4d KiB", (int)size);
		else if ((size /= 1024) <= 9999)
			(*print)("%4d MiB", (int)size);
		else if ((size /= 1024) <= 9999)
			(*print)("%4d GiB", (int)size);
		else
			(*print)("%8s", " ");

		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			unsigned a = (unsigned)t & 0x0000FFFFu;
			(*print)("  Allocated (#%u)\n", a);
			continue;
		}

		if (t >= B_MEM_DATABASE_MIN && t <= B_MEM_DATABASE_MAX) {
			unsigned db = (unsigned)t & 0x0000FFFFu;
			(*print)("  Database (#%u)\n", db);
		} else {
			(*print)("  %s\n", desc);
		}
	}

	(*print)("\n    Total free: ");

	if ((total /= 1024) < 65536)
		(*print)("%zd KiB\n\n", total);
	else
		(*print)("%zd MiB\n\n", (total / 1024));

	memory_mtx_unlock(&memory_mtx);
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
				memory_entries -= 1;
				continue;
			}
		}
		i += 1;
	}
}

static void *memory_aligned_alloc(size_t alignment, size_t size)
{
	struct b_mem_raw *memory = memory_map;
	phys_addr_t memory_map_addr = (phys_addr_t)memory_map;
	phys_addr_t addr = 0;
	size_t b, e;
	size_t i;

	if (alignment >= 0x00400000ul || size == 0ul || size >= 0x7FFFFFFFul)
		return NULL;

	for (i = memory_entries - 1; i > 1; i--) {
		size_t remainder = 0;

		if (memory[i].base_high)
			continue;
		if (memory[i].base_low > memory_map_addr)
			continue;

		b = (size_t)memory[i - 1].base_low;
		e = (size_t)memory[i].base_low;

		if (memory[i - 1].type != B_MEM_NORMAL || size > e - b)
			continue;
		if (memory[i - 1].flags & B_FLAG_NO_INIT_ALLOC)
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

	/*
	 * The allocated memory must not be too far away from the init
	 * module. This guarantees that dynamically linked modules work.
	 */
	{
		size_t addr_diff;

		if (addr < memory_map_addr)
			addr_diff = (size_t)(memory_map_addr - addr);
		else
			return NULL;

		if (addr_diff > 0x7F000000ul)
			return NULL;
	}

	size = sizeof(struct b_mem) * (memory_entries - i);
	memmove(&memory[i + 1], &memory[i], size);
	memmove(&memory[i], &memory[i - 1], sizeof(struct b_mem));
	memory_entries += 1;

	memory[i].type = B_MEM_INIT_ALLOC_MIN;
	memory[i].base_low = (uint32_t)addr;
	memory[i].base_high = 0;

	fix_memory_map();
	return (void *)addr;
}

void *aligned_alloc(size_t alignment, size_t size)
{
	void *r;

	if (memory_mtx_lock(&memory_mtx) != thrd_success)
		return NULL;

	r = memory_aligned_alloc(alignment, size);
	memory_mtx_unlock(&memory_mtx);

	return r;
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

void free(void *ptr)
{
	struct b_mem *memory = memory_map;
	phys_addr_t addr = (phys_addr_t)ptr;
	uint32_t t;
	size_t i;

	if (ptr == NULL)
		return;

	if (memory_mtx_lock(&memory_mtx) != thrd_success)
		return;

	for (i = 1; (memory[i].flags & B_FLAG_VALID_ENTRY); i++) {
		if (memory[i].base != addr)
			continue;
		t = memory[i].type;

		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			memory[i].type = B_MEM_NORMAL;
			break;
		}
	}
	fix_memory_map();
	memory_mtx_unlock(&memory_mtx);
}
