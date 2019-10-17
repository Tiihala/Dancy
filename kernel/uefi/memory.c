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

void *memory_db_all[1000];
void *memory_in_x64[2];

static void *MemoryMap;
static uint64_t MemoryMapSize;
static uint64_t MemoryMapEntries;
static uint64_t DescriptorSize;
static uint32_t DescriptorVersion;

#define NUMBER_OF_ALLOCATIONS 3

static struct {
	uint64_t Memory;
	uint64_t Pages;
} Allocations[NUMBER_OF_ALLOCATIONS];

static const uint64_t StaticCodePages = 0x0040;
static const uint64_t StaticDataPages = 0x4000;

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

int memory_export_map(void)
{
	return 1;
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
	}

	for (i = 0; i < 2; i++) {
		memory_in_x64[i] = NULL;
	}

	for (i = 0; i < 1000; i++) {
		memory_db_all[i] = NULL;
	}

	MemoryMap = NULL;
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
		u_print("AllocatePages: unknown error (%016llX))\n", s);
		return 1;
	}
	return 0;
}

int memory_init(void)
{
	uint64_t Memory = MaxAddress;
	EFI_STATUS s;

	/*
	 * Allocate static data pages that are mainly used for databases.
	 */
	s = gSystemTable->BootServices->AllocatePages(
		AllocateMaxAddress, EfiLoaderData, StaticDataPages, &Memory);

	if (check_allocate_pages_errors(s))
		return 1;

	Allocations[0].Memory = Memory;
	Allocations[0].Pages = StaticDataPages;

	/*
	 * It should be very unlikely that a succesfully returned
	 * memory area does not meet the MaxAddress requirement.
	 */
	if (Memory >= MaxAddress) {
		u_print("AllocatePages: invalid MaxAddress\n");
		return memory_free(), 1;
	}

	memset((void *)Memory, 0, (size_t)(StaticDataPages * 4096));

	/*
	 * Set the data segment slots. All of them are 65536-byte aligned.
	 * One segment slot is for the MemoryMap.
	 */
	{
		const uint64_t segment_size = 0x10000;
		int i;

		Memory += 0x0000FFFFull;
		Memory &= 0xFFFF0000ull;

		for (i = 0; i < 1000; i++) {
			memory_db_all[i] = (void *)Memory;
			Memory += segment_size;
		}

		MemoryMap = (void *)Memory;
	}

	/*
	 * Allocate static code pages (IN_X64.AT + native memory map).
	 */
	Memory = MaxAddress;

	s = gSystemTable->BootServices->AllocatePages(
		AllocateMaxAddress, EfiLoaderCode, StaticCodePages, &Memory);

	if (check_allocate_pages_errors(s))
		return memory_free(), 1;

	Allocations[1].Memory = Memory;
	Allocations[1].Pages = StaticCodePages;

	/*
	 * It should be very unlikely that a succesfully returned
	 * memory area does not meet the MaxAddress requirement.
	 */
	if (Memory >= MaxAddress) {
		u_print("AllocatePages: invalid MaxAddress\n");
		return memory_free(), 1;
	}

	memset((void *)Memory, 0, (size_t)(StaticCodePages * 4096));

	/*
	 * Set the code segment slots. Both of them are 65536-byte aligned.
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
	}

	if (memory_update_map())
		return memory_free(), 1;

	/*
	 * Allocate the third memory slot that will be used for memory
	 * allocations (the init executable, IN_X64.AT). The type of
	 * the memory slot is EfiLoaderData.
	 */
	{
		uint64_t Pages = 16;
		unsigned char *map = MemoryMap;
		EFI_MEMORY_DESCRIPTOR *entry;
		uint64_t i, b, e;

		/*
		 * Find the biggest contiguous area (EfiConventionalMemory).
		 */
		for (i = 0; i < MemoryMapEntries; i++) {
			entry = (void *)(map + i * DescriptorSize);

			if (entry->Type != EfiConventionalMemory)
				continue;

			b = entry->PhysicalAddress;

			if (b >= (e = b + entry->NumberOfPages * 4096 - 1))
				continue;
			if (e > MaxAddress)
				continue;

			if (Pages < entry->NumberOfPages)
				Pages = entry->NumberOfPages;
		}

		/*
		 * The "Pages" is the amount of contiguous memory that
		 * is potentially available for the next allocation. Use
		 * only 3/4 of it because the UEFI firmware might need
		 * free pages for boot services. For performance reasons,
		 * the allocated memory is not zeroed.
		 */
		Memory = MaxAddress;
		Pages = Pages - (Pages / 4);

		s = gSystemTable->BootServices->AllocatePages(
			AllocateMaxAddress, EfiLoaderData, Pages, &Memory);

		if (check_allocate_pages_errors(s))
			return memory_free(), 1;

		Allocations[2].Memory = Memory;
		Allocations[2].Pages = Pages;

		/*
		 * It should be very unlikely that a succesfully returned
		 * memory area does not meet the MaxAddress requirement.
		 */
		if (Memory >= MaxAddress) {
			u_print("AllocatePages: invalid MaxAddress\n");
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
	unsigned char *map = MemoryMap;
	EFI_MEMORY_DESCRIPTOR *entry;
	uint64_t i, j, b, e;

	(*print)("Memory Map\n");

	for (i = 0; i < MemoryMapEntries; i++) {
		const char *desc1 = "", *desc2 = "";

		entry = (void *)(map + i * DescriptorSize);
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

static int qsort_compare(const void *a, const void *b)
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
		(size_t)DescriptorSize, qsort_compare);

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

			e1 = (void *)(map + i * DescriptorSize);

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

			e1 = (void *)(map + (i - 1) * DescriptorSize);
			e2 = (void *)(map + (i - 0) * DescriptorSize);

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
