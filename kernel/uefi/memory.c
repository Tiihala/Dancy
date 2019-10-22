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
 * uefi/memory.c
 *      Memory allocation and GetMemoryMap
 */

#include <uefi.h>

uint64_t gMapkey;

/*
 * memory_db_all[0]   database #0        65536 bytes
 * memory_db_all[1]   database #1        65536 bytes
 *   ...                ...
 * memory_db_all[999] database #99       65536 bytes
 *
 *
 * memory_in_x64[0]   IN_X64.AT          65536 bytes
 * memory_in_x64[1]   native memory map  65536 bytes
 *
 * Notes:
 *         1. All slots are 65536-byte aligned
 *         2. &memory_in_x64[0] + 0x10000 == &memory_in_x64[1]
 *         2. &memory_in_x64[1] + 0x10000 == &memory_db_all[0]
 */
void *memory_db_all[1000];
void *memory_in_x64[2];

extern void *MemoryMap;

static uint64_t MemoryMapSize;
static uint64_t MemoryMapEntries;
static uint64_t DescriptorSize;
static uint32_t DescriptorVersion;

#define NUMBER_OF_ALLOCATIONS 2

static struct {
	uint64_t Memory;
	uint64_t Pages;
	uint64_t Attribute;
} Allocations[NUMBER_OF_ALLOCATIONS];

static const uint64_t StaticPages = 0x4000;
static const uint64_t MaxAddress = 0xEFFFFFFFull;

static const char *memory_types[] = {
	"EfiReservedMemoryType",
	"EfiLoaderCode",
	"EfiLoaderData",
	"EfiBootServicesCode",
	"EfiBootServicesData",
	"EfiRuntimeServicesCode",
	"EfiRuntimeServicesData",
	"EfiConventionalMemory",
	"EfiUnusableMemory",
	"EfiACPIReclaimMemory",
	"EfiACPIMemoryNVS",
	"EfiMemoryMappedIO",
	"EfiMemoryMappedIOPortSpace",
	"EfiPalCode",
	"EfiPersistentMemory"
};

static int qsort_native_map(const void *a, const void *b)
{
	uint64_t addr1 = ((const struct b_mem *)a)->base;
	uint64_t addr2 = ((const struct b_mem *)b)->base;
	uint32_t type1 = ((const struct b_mem *)a)->type + 1;
	uint32_t type2 = ((const struct b_mem *)b)->type + 1;

	if (addr1 < addr2)
		return -1;
	if (addr1 > addr2)
		return 1;

	/*
	 * Bigger type value is listed first and this detail is very
	 * important. Only the bigger type value is preserved and
	 * other entries will be deleted (see memory_export_map).
	 *
	 * The special type 0xFFFFFFFF is for "not reported" areas
	 * and that is why 1 was added to the types before comparing
	 * the values. 0xFFFFFFFF + 1 == 0 (uint32_t).
	 */
	if (type1 > type2)
		return -1;
	if (type1 < type2)
		return 1;
	return 0;
}

int memory_export_map(void)
{
	const size_t max_entries = 2019;
	const unsigned char *map = MemoryMap;
	struct b_mem *native_map = memory_in_x64[1];
	const EFI_MEMORY_DESCRIPTOR *efi_entry;
	size_t entries;
	size_t i, j;

	/*
	 * The "max_entries" variable is not a strict limit for the
	 * updated native map. The limit is 2047 + empty entry.
	 */
	for (entries = 0; entries < max_entries; entries++) {
		if ((native_map[entries].flags & B_FLAG_VALID_ENTRY) == 0)
			break;
	}

	if (entries >= max_entries) {
		u_print("Error: the native memory map overflows\n");
		return memset(native_map, 0, 0x10000), 1;
	}

	/*
	 * Types B_MEM_INIT_ALLOC_MIN to MAX from the current native
	 * map must be preserved. Other entries will be rewritten,
	 * including the one that ends the memory allocation slot.
	 */
	for (i = 0; i < entries; /* void */) {
		struct b_mem *m = &native_map[i];
		size_t t, size;

		t = (size_t)m->type;
		if (t >= B_MEM_INIT_ALLOC_MIN && t <= B_MEM_INIT_ALLOC_MAX) {
			i += 1;
			continue;
		}

		size = sizeof(struct b_mem) * (entries - i);
		memmove(&native_map[i], &native_map[i + 1], size);
		entries -= 1;
	}

	/*
	 * Write the entries that must be available on the native map.
	 */
	native_map[entries].type = 0xFFFFFFFFul;
	native_map[entries].base = 0x000000000ull;
	entries += 1;

	native_map[entries].type = 0xFFFFFFFFul;
	native_map[entries].base = 0x100000000ull;
	entries += 1;

	/*
	 * Write all the memory entries from MemoryMap that are not
	 * in the Allocations array.
	 */
	for (i = 0; i < MemoryMapEntries; i++) {
		uint64_t addr, next;

		efi_entry = (const void *)(map + i * DescriptorSize);

		addr = efi_entry->PhysicalAddress;
		next = addr + efi_entry->NumberOfPages * 4096;

		for (j = 0; j < NUMBER_OF_ALLOCATIONS; j++) {
			uint64_t t1 = Allocations[j].Memory;
			uint64_t t2 = t1 + Allocations[j].Pages * 4096;

			if (addr >= t1 && addr < t2)
				efi_entry = NULL;
			if ((next - 1) >= t1 && (next - 1) < t2)
				efi_entry = NULL;
		}
		if (efi_entry == NULL)
			continue;

		native_map[entries].type = efi_entry->Type;
		native_map[entries].base = (phys_addr_t)addr;
		native_map[entries].efi_attributes = efi_entry->Attribute;
		entries += 1;

		native_map[entries].type = 0xFFFFFFFFul;
		native_map[entries].base = (phys_addr_t)next;
		native_map[entries].efi_attributes = 0;
		entries += 1;

		if (entries >= max_entries) {
			u_print("Error: the native memory map overflows\n");
			return memset(native_map, 0, 0x10000), 1;
		}
	}

	/*
	 * Write all the memory entries from the Allocations array.
	 */
	for (i = 0; i < NUMBER_OF_ALLOCATIONS; i++) {
		uint64_t addr, next;

		addr = Allocations[i].Memory;
		next = addr + Allocations[i].Pages * 4096;

		native_map[entries].type = B_MEM_NORMAL;
		native_map[entries].base = (phys_addr_t)addr;
		native_map[entries].efi_attributes = Allocations[i].Attribute;
		entries += 1;

		native_map[entries].type = 0xFFFFFFFFul;
		native_map[entries].base = (phys_addr_t)next;
		native_map[entries].efi_attributes = 0;
		entries += 1;
	}

	/*
	 * Write the IN_X64.AT + native memory map.
	 */
	{
		uint64_t addr = (uint64_t)memory_in_x64[0];

		native_map[entries].type = (uint32_t)(B_MEM_INIT_EXECUTABLE);
		native_map[entries].base = (phys_addr_t)addr;
		native_map[entries].efi_attributes = Allocations[0].Attribute;
		entries += 1;
	}

	/*
	 * Write all the database entries. If there is no database, change
	 * the memory type to B_MEM_NORMAL.
	 */
	for (i = 0; i < 1000; i++) {
		unsigned int db_test = *((unsigned int *)memory_db_all[i]);
		uint64_t addr = (uint64_t)memory_db_all[i];;

		if (db_test) {
			uint32_t type = (uint32_t)(B_MEM_DATABASE_MIN + i);
			native_map[entries].type = type;
		} else {
			native_map[entries].type = B_MEM_NORMAL;
			native_map[entries].flags = B_FLAG_VALID_ENTRY;
		}
		native_map[entries].base = (phys_addr_t)addr;
		native_map[entries].efi_attributes = Allocations[0].Attribute;
		entries += 1;

		if (entries >= max_entries) {
			u_print("Error: the native memory map overflows\n");
			return memset(native_map, 0, 0x10000), 1;
		}
	}

	/*
	 * Add an entry that starts after the last database.
	 */
	native_map[entries].type = B_MEM_NORMAL;
	native_map[entries].base = (phys_addr_t)memory_db_all[999] + 0x10000;
	native_map[entries].efi_attributes = Allocations[0].Attribute;
	entries += 1;

	/*
	 * Set the proper memory map entry flags.
	 */
	for (i = 0; i < entries; i++) {
		native_map[i].flags = (B_FLAG_VALID_ENTRY | B_FLAG_UEFI);

		if (native_map[i].base <= 0x100000000ull)
			native_map[i].flags |= B_FLAG_VALID_LEGACY;

		if (native_map[i].type == B_MEM_NORMAL) {
			if (native_map[i].base != Allocations[1].Memory)
				native_map[i].flags |= B_FLAG_NO_INIT_ALLOC;
		}
	}

	qsort(native_map, entries, sizeof(native_map[0]), qsort_native_map);

	/*
	 * Delete unnecessary map entries. The base address must be unique.
	 */
	for (i = 1; i < entries; /* void */) {
		struct b_mem *m1 = &native_map[i - 1];
		struct b_mem *m2 = &native_map[i];
		size_t size;

		if (m1->base != m2->base) {
			i += 1;
			continue;
		}
		size = sizeof(struct b_mem) * (entries - i);
		memmove(&native_map[i], &native_map[i + 1], size);
		entries -= 1;
	}

	/*
	 * Merge contiguous memory areas. The entry for base 0x100000000 must
	 * always be on the native map.
	 */
	for (i = 1; i < entries; /* void */) {
		struct b_mem *m1 = &native_map[i - 1];
		struct b_mem *m2 = &native_map[i];
		size_t size;

		if (m1->base <= 0xFFFFFFFFull && m2->base == 0x100000000ull) {
			i += 1;
			continue;
		}

		if (m1->type != m2->type || m1->flags != m2->flags) {
			i += 1;
			continue;
		}
		if (m1->efi_attributes != m2->efi_attributes) {
			i += 1;
			continue;
		}

		size = sizeof(struct b_mem) * (entries - i);
		memmove(&native_map[i], &native_map[i + 1], size);
		entries -= 1;
	}

	/*
	 * Check the map so that there are no major structural errors.
	 */
	for (i = 1; i < entries + 4; i++) {
		struct b_mem *m1 = &native_map[i - 1];
		struct b_mem *m2 = &native_map[i];
		int err = 0;

		if (i < entries) {
			if (m1->base >= m2->base)
				err = 1;
		} else {
			if (m2->type != 0 || m2->flags != 0 || m2->base != 0)
				err = 1;
			if (m2->efi_attributes != 0)
				err = 1;
		}

		if (err != 0) {
			u_print("Error: exporting the memory map failed\n");
			return memset(native_map, 0, 0x10000), 1;
		}
	}
	return 0;
}

void memory_free(void)
{
	size_t i;

	for (i = 0; i < NUMBER_OF_ALLOCATIONS; i++) {
		uint64_t m = Allocations[i].Memory;
		uint64_t p = Allocations[i].Pages;

		if (p != 0)
			gSystemTable->BootServices->FreePages(m, p);

		Allocations[i].Memory = 0;
		Allocations[i].Pages = 0;
		Allocations[i].Attribute = 0;
	}

	for (i = 0; i < 2; i++) {
		memory_in_x64[i] = NULL;
	}

	for (i = 0; i < 1000; i++) {
		memory_db_all[i] = NULL;
	}

	MemoryMapSize = 0;
	MemoryMapEntries = 0;
	DescriptorSize = 0;
	DescriptorVersion = 0;
}

static int check_allocate_pages_errors(EFI_STATUS s)
{
	if (s == EFI_OUT_OF_RESOURCES) {
		u_print("AllocatePages: out of resources\n");
		return 1;
	}
	if (s == EFI_NOT_FOUND) {
		u_print("AllocatePages: pages could not be found\n");
		return 1;
	}
	if (s != EFI_SUCCESS) {
		u_print("AllocatePages: unknown error %016llX\n", s);
		return 1;
	}
	return 0;
}

static uint64_t find_free_memory(uint64_t pages, uint64_t *mem, uint64_t *att)
{
	const unsigned char *map = MemoryMap;
	const EFI_MEMORY_DESCRIPTOR *entry;
	uint64_t max_pages = (pages != 0) ? pages : 16;
	uint64_t i, b, e;

	*mem = 0xFFFFFFFFFFFFFFFFull, *att = 0;

	if (memory_update_map())
		return 1;

	for (i = 0; i < MemoryMapEntries; i++) {
		entry = (const void *)(map + i * DescriptorSize);

		if (entry->Type != EfiConventionalMemory)
			continue;

		b = entry->PhysicalAddress;

		if (b >= (e = b + entry->NumberOfPages * 4096 - 1))
			continue;
		if (e > MaxAddress)
			continue;

		if (pages == 0) {
			if (entry->NumberOfPages >= max_pages) {
				*mem = b;
				*att = entry->Attribute;
				max_pages = entry->NumberOfPages;
			}
		} else if (entry->NumberOfPages >= pages) {
			*mem = b + ((entry->NumberOfPages - pages) * 4096);
			*att = entry->Attribute;
		}
	}

	if (*mem == 0xFFFFFFFFFFFFFFFFull) {
		(void)check_allocate_pages_errors(EFI_NOT_FOUND);
		return (*att = 0);
	}
	return max_pages;
}

int memory_init(void)
{
	uint64_t Memory, Attribute;
	EFI_STATUS s;

	/*
	 * Allocate static memory pages that are used for the init executable
	 * (IN_X64.AT), the native memory map, and the databases. The type of
	 * the allocated memory is EfiLoaderCode.
	 */
	if (!find_free_memory(StaticPages, &Memory, &Attribute))
		return 1;

	s = gSystemTable->BootServices->AllocatePages(
		AllocateAddress, EfiLoaderCode, StaticPages, &Memory);

	if (check_allocate_pages_errors(s))
		return 1;

	Allocations[0].Memory = Memory;
	Allocations[0].Pages = StaticPages;
	Allocations[0].Attribute = Attribute;

	if ((Memory & 4095ull) != 0ull || Memory >= MaxAddress) {
		u_print("AllocatePages: invalid memory address\n");
		return memory_free(), 1;
	}

	memset((void *)Memory, 0, (size_t)(StaticPages * 4096));

	/*
	 * Set the segment slots. All of them are 65536-byte aligned.
	 */
	{
		const uint64_t segment_size = 0x10000;
		int i;

		Memory += 0x0000FFFFull;
		Memory &= 0xFFFF0000ull;

		for (i = 0; i < 2; i++) {
			memory_in_x64[i] = (void *)Memory;
			Memory += segment_size;
		}

		for (i = 0; i < 1000; i++) {
			memory_db_all[i] = (void *)Memory;
			Memory += segment_size;
		}
	}

	/*
	 * Allocate another memory slot that will be used for memory
	 * allocations (the init executable, IN_X64.AT). The type of
	 * the memory slot is EfiLoaderCode.
	 */
	{
		uint64_t Pages;

		/*
		 * Find the biggest contiguous area (EfiConventionalMemory).
		 */
		if ((Pages = find_free_memory(0, &Memory, &Attribute)) == 0)
			return memory_free(), 1;

		/*
		 * The "Pages" is the amount of contiguous memory that
		 * is potentially available for the next allocation. Use
		 * only 3/4 of it because the UEFI firmware might need
		 * free pages for boot services. For performance reasons,
		 * the allocated memory is not zeroed.
		 */
		Memory = Memory + ((Pages / 4) * 4096);
		Pages = Pages - (Pages / 4);

		s = gSystemTable->BootServices->AllocatePages(
			AllocateAddress, EfiLoaderCode, Pages, &Memory);

		if (check_allocate_pages_errors(s))
			return memory_free(), 1;

		Allocations[1].Memory = Memory;
		Allocations[1].Pages = Pages;
		Allocations[1].Attribute = Attribute;

		if ((Memory & 4095ull) != 0ull || Memory >= MaxAddress) {
			u_print("AllocatePages: invalid memory address\n");
			return memory_free(), 1;
		}
	}

	if (memory_update_map())
		return memory_free(), 1;
	return 0;
}

void memory_print_map(void (*print)(const char *, ...))
{
	size_t types_limit = sizeof(memory_types) / sizeof(memory_types[0]);
	const unsigned char *map = MemoryMap;
	const EFI_MEMORY_DESCRIPTOR *entry;
	uint64_t i, j, b, e;

	(*print)("Memory Map\n");

	for (i = 0; i < MemoryMapEntries; i++) {
		const char *desc1 = "", *desc2 = "";

		entry = (const void *)(map + i * DescriptorSize);
		b = entry->PhysicalAddress;

		if (b >= (e = b + entry->NumberOfPages * 4096 - 1))
			continue;

		if (entry->Type < (uint32_t)types_limit)
			desc1 = memory_types[entry->Type];

		for (j = 0; j < NUMBER_OF_ALLOCATIONS; j++) {
			if (Allocations[j].Memory == b)
				desc2 = " (*)";
		}
		(*print)("    %016llX %016llX  %s%s\n", b, e, desc1, desc2);
	}
	(*print)("\n");
}

static int qsort_map(const void *a, const void *b)
{
	uint64_t addr1 = ((const EFI_MEMORY_DESCRIPTOR *)a)->PhysicalAddress;
	uint64_t addr2 = ((const EFI_MEMORY_DESCRIPTOR *)b)->PhysicalAddress;

	if (addr1 < addr2)
		return -1;
	if (addr1 > addr2)
		return 1;
	return 0;
}

int memory_update_map(void)
{
	const uint64_t memory_map_size = 0x10000;
	EFI_STATUS s;

	gMapkey = 0;
	MemoryMapSize = memory_map_size - 256;
	memset(MemoryMap, 0, (size_t)memory_map_size);

	s = gSystemTable->BootServices->GetMemoryMap(&MemoryMapSize,
		MemoryMap, &gMapkey, &DescriptorSize, &DescriptorVersion);

	if (s != EFI_SUCCESS || MemoryMapSize >= memory_map_size) {
		u_print("GetMemoryMap: could not get the memory map\n");
		return 1;
	}
	if ((size_t)DescriptorSize < sizeof(EFI_MEMORY_DESCRIPTOR)) {
		u_print("GetMemoryMap: unknown descriptor size\n");
		return 1;
	}
	if ((MemoryMapEntries = MemoryMapSize / DescriptorSize) < 4) {
		u_print("GetMemoryMap: suspicious map size\n");
		return 1;
	}

	qsort(MemoryMap, (size_t)MemoryMapEntries,
		(size_t)DescriptorSize, qsort_map);

	/*
	 * Validate the memory map, e.g. overlapping memory areas are not
	 * accepted because the whole map might be corrupted. If there
	 * are many computers that fail this test, some memory map fixing
	 * procedures must be implemented here.
	 */
	{
		const unsigned char *map = MemoryMap;
		const EFI_MEMORY_DESCRIPTOR *e1, *e2;
		uint64_t i;

		for (i = 0; i < MemoryMapEntries; i++) {
			uint64_t addr, pages, next;
			int err = 0;

			e1 = (const void *)(map + i * DescriptorSize);

			addr = e1->PhysicalAddress;
			pages = e1->NumberOfPages;

			if ((addr & 4095ull) != 0ull)
				err = 1;
			if ((pages & 0xFFF0000000000000ull) != 0ull)
				err = 1;

			next = addr + pages * 4096;

			if (addr >= next || err != 0) {
				u_print("GetMemoryMap: corrupted map\n");
				return 1;
			}
		}

		for (i = 1; i < MemoryMapEntries; i++) {
			uint64_t addr, next;

			e1 = (const void *)(map + (i - 1) * DescriptorSize);
			e2 = (const void *)(map + (i - 0) * DescriptorSize);

			addr = e2->PhysicalAddress;
			next = e1->PhysicalAddress + e1->NumberOfPages * 4096;

			if (addr < next) {
				u_print("GetMemoryMap: overlapping map\n");
				return 1;
			}
		}
	}
	return 0;
}
