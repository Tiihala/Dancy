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

static uint64_t DataBaseAddress;
static const uint64_t Pages = 0x4000ul;
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
	int i;

	gSystemTable->BootServices->FreePages(DataBaseAddress, Pages);
	DataBaseAddress = 0;

	for (i = 0; i < 2; i++) {
		memory_in_x64[i] = NULL;
	}

	for (i = 0; i < 1000; i++) {
		memory_db_all[i] = NULL;
	}

	MemoryMapSize = 0;
	MemoryMap = NULL;
}

int memory_init(void)
{
	uint64_t Memory = MaxAddress;
	EFI_STATUS s;

	s = gSystemTable->BootServices->AllocatePages(
		AllocateMaxAddress, EfiLoaderData, Pages, &Memory);

	if (s == EFI_OUT_OF_RESOURCES) {
		u_print("AllocatePages: out of resources\n");
		return 1;
	}

	if (s == EFI_NOT_FOUND) {
		u_print("AllocatePages: pages could not be found\n");
		return 1;
	}

	if (s != EFI_SUCCESS) {
		u_print("AllocatePages: unknown error\n");
		return 1;
	}

	DataBaseAddress = Memory;

	/*
	 * It should be very unlikely that a succesfully returned
	 * memory area does not meet the MaxAddress requirement.
	 */
	if (Memory >= MaxAddress) {
		u_print("AllocatePages: invalid MaxAddress\n");
		return memory_free(), 1;
	}

	memset((void *)DataBaseAddress, 0, (size_t)(Pages * 4096));

	/*
	 * Set the segment slots. All of them are 65536-byte aligned.
	 */
	{
		const uint64_t segment_size = 0x10000;
		int i;

		Memory += 0x0000FFFFul;
		Memory &= 0xFFFF0000ul;

		for (i = 0; i < 2; i++) {
			memory_in_x64[i] = (void *)Memory;
			Memory += segment_size;
		}

		for (i = 0; i < 1000; i++) {
			memory_db_all[i] = (void *)Memory;
			Memory += segment_size;
		}

		MemoryMap = (void *)Memory;
	}

	if (memory_update_map())
		return memory_free(), 1;
	return 0;
}

void memory_print_map(void (*print)(const char *, ...))
{
	size_t types_limit = sizeof(memory_types) / sizeof(memory_types[0]);
	EFI_MEMORY_DESCRIPTOR *entry;
	uint64_t i, b, e;

	(*print)("Memory Map\n");

	for (i = 0; i < MemoryMapEntries; i++) {
		void *entry_address = (char *)MemoryMap + i * DescriptorSize;
		const char *desc1 = "", *desc2 = "";

		entry = entry_address;
		b = entry->PhysicalAddress;

		if (b >= (e = b + entry->NumberOfPages * 4096 - 1))
			continue;

		if (entry->Type < (uint32_t)types_limit)
			desc1 = memory_types[entry->Type];

		if (DataBaseAddress == b || (uint64_t)gBaseAddress == b)
			desc2 = " (*)";

		(*print)("    %016llX %016llX  %s\n", b, e, desc1, desc2);
	}
	(*print)("\n");
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
	if (DescriptorSize < 32) {
		u_print("GetMemoryMap: unknown descriptor size\n");
		return 1;
	}
	if ((MemoryMapEntries = MemoryMapSize / DescriptorSize) < 4) {
		u_print("GetMemoryMap: suspicious map size\n");
		return 1;
	}
	return 0;
}
