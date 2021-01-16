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
 * init/table.c
 *      Kernel table initialization
 */

#include <init.h>

static size_t heap_size = 0;
static size_t heap_used = 0;

static void *table_alloc_pages(size_t count)
{
	size_t size = count * 0x1000;
	addr_t addr;

	heap_used = (heap_used + 0x0FFFu) & 0xFFFFF000u;

	if (count > 0x8000 || (heap_used + size) > heap_size)
		panic("table_alloc_pages: out of memory");

	addr = kernel->heap_addr + (addr_t)heap_used;
	heap_used += size;

	memset((void *)addr, 0, size);

	return (void *)addr;
}

static void *table_malloc(size_t size)
{
	size_t aligned_size = (size + 0x0Fu) & 0xFFFFFFF0u;
	addr_t addr;

	heap_used = (heap_used + 0x0Fu) & 0xFFFFFFF0u;

	if (size > 0x10000 || (heap_used + aligned_size) > heap_size)
		panic("table_malloc: out of memory");

	addr = kernel->heap_addr + (addr_t)heap_used;
	heap_used += aligned_size;

	memset((void *)addr, 0, size);

	return (void *)addr;
}

void table_init(void)
{
	addr_t addr;
	size_t size, i, j;

	/*
	 * Initialize the local memory manager.
	 */
	heap_size = (size_t)(kernel->stack_array_addr - kernel->heap_addr);

	if (heap_size == 0 || (heap_size & 0x0FFFu) != 0)
		panic("table_init: unexpected behavior");

	/*
	 * Write the function pointers.
	 */
	kernel->panic = panic;
	kernel->print = gui_print;

	/*
	 * Write the TSC frequency variable.
	 */
	kernel->delay_tsc_hz = delay_tsc_hz;
	if (kernel->delay_tsc_hz < 1000)
		panic("TSC: unexpected frequency value");

	/*
	 * Write the framebuffer variables.
	 */
	if (gui_video_info != NULL) {
		const struct b_video_info *vi = gui_video_info;

		kernel->fb_mode   = vi->mode;
		kernel->fb_width  = vi->width;
		kernel->fb_height = vi->height;
		kernel->fb_stride = vi->stride;
		kernel->fb_addr   = vi->framebuffer;

		/*
		 * The standard framebuffer has 4-byte pixels.
		 */
		size = (size_t)(vi->width * vi->height * sizeof(uint32_t));
		size = (size + 0x0FFFu) & 0xFFFFF000u;
		addr = (addr_t)table_alloc_pages(size / 0x1000);

		kernel->fb_standard_addr = addr;
		kernel->fb_standard_size = size;
	}

	/*
	 * Write the SMP variables. Allocate the zero-length memory area
	 * even if there were no application processors.
	 */
	{
		kernel->smp_ap_count = (int)smp_ap_count;

		size = (size_t)(smp_ap_count * sizeof(uint32_t));
		kernel->smp_ap_id = table_malloc(size);

		for (i = 0; i < smp_ap_count; i++)
			kernel->smp_ap_id[i] = smp_ap_id[i];
	}

	/*
	 * Write the APIC and I/O APIC variables.
	 */
	{
		const struct acpi_information *acpi = acpi_get_information();
		struct acpi_io_apic io_apic;

		kernel->apic_enabled = apic_mode;
		kernel->apic_base_addr = apic_base_addr;
		kernel->apic_bsp_id = apic_bsp_id;

		kernel->io_apic_enabled = apic_mode;
		kernel->io_apic_count = (int)acpi->num_io_apic;

		size = sizeof((kernel->io_apic[0]));
		size *= (size_t)kernel->io_apic_count;

		kernel->io_apic = table_malloc(size);

		for (i = 0; i < (size_t)kernel->io_apic_count; i++) {
			if (acpi_get_io_apic((unsigned)i, &io_apic))
				panic("I/O APIC: unexpected behavior");

			kernel->io_apic[i].id = io_apic.id;
			kernel->io_apic[i].base_int = io_apic.base_int;
			kernel->io_apic[i].addr = io_apic.addr;

			/*
			 * Override table is the same for each I/O APIC.
			 */
			for (j = 0; j < 16; j++) {
				uint32_t g = io_apic.irq[j].global_int;
				uint32_t f = io_apic.irq[j].flags;

				kernel->io_apic_override[j].global_int = g;
				kernel->io_apic_override[j].flags = f;
			}
		}
	}

	/*
	 * Write the system memory map.
	 */
	{
		const struct b_mem_raw *memory = memory_map;
		size_t entries = 1;

		for (;;) {
			uint32_t base_low = memory[entries].base_low;
			uint32_t base_high = memory[entries].base_high;

			entries += 1;

			if (base_low == 0 && base_high == 0)
				break;
		}

		if (entries != memory_entries + 1)
			panic("table_init: inconsistent memory map");

		size = entries * sizeof((kernel->memory_map[0]));
		kernel->memory_map = table_malloc(size);
		kernel->memory_map_size = size;

		for (i = 0; i < (entries - 1); i++) {
			uint64_t base_low = (uint64_t)memory[i].base_low;
			uint64_t base_high = (uint64_t)memory[i].base_high;
			uint64_t base = ((base_high << 16) << 16) | base_low;

			const void *other = &memory[i].other[0];
			uint64_t efi_attributes = *((const uint64_t *)other);

			kernel->memory_map[i].type = memory[i].type;
			kernel->memory_map[i].base = base;
			kernel->memory_map[i].efi_attributes = efi_attributes;
		}

		if (kernel->memory_map[0].base != 0)
			panic("table_init: inconsistent memory map");

		for (i = 1; i < (entries - 1); i++) {
			uint64_t b1 = kernel->memory_map[i - 1].base;
			uint64_t b2 = kernel->memory_map[i].base;

			if (b1 >= b2)
				panic("table_init: inconsistent memory map");
		}
	}

	/*
	 * Write the PCI device structures.
	 */
	{
		size_t entries = (size_t)pci_device_count;

		kernel->pci_device_count = (int)entries;

		size = entries * sizeof(kernel->pci_device[0]);
		kernel->pci_device = table_malloc(size);

		for (i = 0; i < entries; i++) {
			int group  = pci_devices[i].group;
			int bus    = pci_devices[i].bus;
			int device = pci_devices[i].device;
			int func   = pci_devices[i].func;

			phys_addr_t ecam    = pci_devices[i].ecam;
			uint32_t vendor_id  = pci_devices[i].vendor_id;
			uint32_t device_id  = pci_devices[i].device_id;
			uint32_t class_code = pci_devices[i].class_code;

			kernel->pci_device[i].group  = group;
			kernel->pci_device[i].bus    = bus;
			kernel->pci_device[i].device = device;
			kernel->pci_device[i].func   = func;
			kernel->pci_device[i].ecam   = ecam;

			kernel->pci_device[i].vendor_id  = vendor_id;
			kernel->pci_device[i].device_id  = device_id;
			kernel->pci_device[i].class_code = class_code;
		}
	}

	/*
	 * Update the heap_addr variable.
	 */
	heap_used = (heap_used + 0x0FFFu) & 0xFFFFF000u;
	kernel->heap_addr += (addr_t)heap_used;

	if (kernel->heap_addr > kernel->stack_array_addr)
		panic("table_init: unexpected behavior");
}
