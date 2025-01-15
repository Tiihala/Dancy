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

#include <boot/init.h>

/*
 * The Unicode ranges to be exported to the kernel console.
 */
static struct {
	int start; int end;
} unicode_ranges[] = {
	{ 0x000020, 0x00007F },
	{ 0x0000A0, 0x0002FF },
	{ 0x002000, 0x00200A },
	{ 0x002010, 0x002015 },
	{ 0x0020AC, 0x0020AC },
	{ 0x002122, 0x002122 },
	{ 0x002500, 0x00257F },
	{ -1, -1 }
};

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

static void *table_alloc(size_t size)
{
	size_t aligned_size = (size + 0x0Fu) & 0xFFFFFFF0u;
	addr_t addr;

	heap_used = (heap_used + 0x0Fu) & 0xFFFFFFF0u;

	if (size > 0x100000 || (heap_used + aligned_size) > heap_size)
		panic("table_alloc: out of memory");

	addr = kernel->heap_addr + (addr_t)heap_used;
	heap_used += aligned_size;

	memset((void *)addr, 0, size);

	return (void *)addr;
}

static void detach_init_module(volatile uint32_t *ticks)
{
	static void (*null_func)(volatile uint32_t *);

	kernel->detach_init_module = null_func;

	if (kernel->panic == panic)
		kernel->panic("kernel->panic() not overridden");

	if (kernel->print == gui_print)
		kernel->panic("kernel->print() not overridden");

	while (*ticks < 2000)
		cpu_halt(1);

	if (gui_detach())
		kernel->panic("detach_init_module: unexpected behavior");
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
	kernel->detach_init_module = detach_init_module;
	kernel->panic = panic;
	kernel->print = gui_print;

	/*
	 * Write the supported processor features.
	 */
	kernel->cpu_feature.osfxr = cpu_osfxr_support;
	kernel->cpu_feature.nxbit = cpu_nxbit_support;
	kernel->cpu_feature.gpage = cpu_gpage_support;
	kernel->cpu_feature.rdtscp = cpu_rdtscp_support;

	/*
	 * Write the TSC frequency variable.
	 */
	kernel->delay_tsc_hz = delay_tsc_hz;
	if (kernel->delay_tsc_hz < 1000)
		panic("TSC: unexpected frequency value");

	/*
	 * Allocate the virtual address slot for Local APIC registers.
	 */
	kernel->apic_base_vaddr = (addr_t)table_alloc_pages(1);

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
	 * Write the kernel console bitmap glyphs (monospace).
	 */
	if (gui_video_info != NULL) {
		const struct b_video_info *vi = gui_video_info;
		static unsigned char ttf_bitmap[1024];
		extern void *ttf_array[3];

		void *ttf_kernel = ttf_array[2];
		unsigned code_point, width = 0;
		int glyph_em = 16;
		unsigned char *data;

		if (vi->width >= 1920 && vi->height >= 1080)
			glyph_em = 28;
		else if (vi->width >= 1280 && vi->height >= 1024)
			glyph_em = 22;
		else if (vi->width >= 1024 && vi->height >= 768)
			glyph_em = 20;

		size = (size_t)(glyph_em * glyph_em);

		if (ttf_set_bitmap(ttf_kernel, size, &ttf_bitmap[0]))
			panic("Glyph: could not set the bitmap");

		if (ttf_set_shades(ttf_kernel, 256))
			panic("Glyph: unexpected behavior");

		if (ttf_render(ttf_kernel, 0x20, &width))
			panic("Glyph: rendering error");

		if (width < 8 || width > (unsigned)glyph_em)
			panic("Glyph: unexpected glyph width");

		kernel->glyph_count = 0;

		for (i = 0; unicode_ranges[i].start > 0; i++)
			kernel->glyph_count += 1;

		kernel->glyph_width = (int)width;
		kernel->glyph_height = glyph_em;

		if (kernel->glyph_height < kernel->glyph_width)
			panic("Glyph: unexpected glyph height");

		size = (size_t)kernel->glyph_count * sizeof(kernel->glyph[0]);
		kernel->glyph = table_alloc(size);

		for (i = 0; i < (size_t)kernel->glyph_count; i++) {
			int s = unicode_ranges[i].start;
			int e = unicode_ranges[i].end;
			int unicode_count;

			if (s <= 0 || s > e)
				panic("Glyph: unexpected unicode range");

			unicode_count = (e - s) + 1;

			kernel->glyph[i].unicode_count = unicode_count;
			kernel->glyph[i].unicode = (uint32_t)s;

			size = (size_t)unicode_count;
			size *= ((size_t)width * (size_t)glyph_em);

			kernel->glyph[i].data = table_alloc(size);

			code_point = (unsigned)kernel->glyph[i].unicode;
			data = kernel->glyph[i].data;

			for (j = 0; j < (size_t)unicode_count; j++) {
				unsigned char *ptr = &ttf_bitmap[0];
				unsigned w;
				int x, y;

				if (ttf_render(ttf_kernel, code_point++, &w))
					panic("Glyph: rendering error");

				for (y = 0; y < glyph_em; y++) {
					for (x = 0; x < (int)width; x++)
						*data++ = ptr[x];
					ptr += glyph_em;
				}
			}
		}
	}

	/*
	 * Write the SMP variables. Allocate the zero-length memory area
	 * even if there were no application processors.
	 */
	{
		uint32_t state_count = 0;

		kernel->smp_ap_count = (int)smp_ap_count;

		size = (size_t)(smp_ap_count * sizeof(uint32_t));
		kernel->smp_ap_id = table_alloc(size);

		for (i = 0; i < smp_ap_count; i++)
			kernel->smp_ap_id[i] = smp_ap_id[i];

		for (i = 0; i < smp_ap_count; i++) {
			if (state_count < kernel->smp_ap_id[i] + 1)
				state_count = kernel->smp_ap_id[i] + 1;
		}

		size = (size_t)(state_count * sizeof(*kernel->smp_ap_state));
		kernel->smp_ap_state = table_alloc(size);
	}

	/*
	 * Write ACPI variables.
	 */
	{
		const struct acpi_information *acpi = acpi_get_information();

		if (acpi) {
			kernel->acpi_enabled = 1;

			size = sizeof((kernel->acpi[0]));
			kernel->acpi = table_alloc(size);

			kernel->acpi->rsdp_addr = acpi->rsdp_addr;
			kernel->acpi->rsdt_addr = acpi->rsdt_addr;
			kernel->acpi->xsdt_addr = acpi->xsdt_addr;
			kernel->acpi->fadt_addr = acpi->fadt_addr;
			kernel->acpi->madt_addr = acpi->madt_addr;
			kernel->acpi->mcfg_addr = acpi->mcfg_addr;
			kernel->acpi->hpet_addr = acpi->hpet_addr;

			kernel->acpi->rtc_century_idx = acpi->rtc_century_idx;
			kernel->acpi->iapc_boot_arch = acpi->iapc_boot_arch;
		}
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

		/*
		 * Use the virtual slot if the APIC base address is not set.
		 */
		if (!kernel->apic_base_addr)
			kernel->apic_base_addr = kernel->apic_base_vaddr;

		kernel->io_apic_enabled = apic_mode;

		if (acpi)
			kernel->io_apic_count = (int)acpi->num_io_apic;
		else
			kernel->io_apic_count = 0;

		size = sizeof((kernel->io_apic[0]));
		size *= (size_t)kernel->io_apic_count;

		kernel->io_apic = table_alloc(size);

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
		kernel->memory_map = table_alloc(size);
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
	 * Write the file system image for native binaries.
	 */
	{
		size = (arctic_bin_size + 0x0FFFu) & 0xFFFFF000u;
		addr = (addr_t)table_alloc_pages(size / 0x1000);

		memcpy((void *)addr, arctic_bin_data, arctic_bin_size);
		memset(arctic_bin_data, 0, arctic_bin_size);

		kernel->arctic_bin_addr = addr;
		kernel->arctic_bin_size = size;
	}

	/*
	 * Write the root file system image.
	 */
	{
		size = (arctic_root_size + 0x0FFFu) & 0xFFFFF000u;
		addr = (addr_t)table_alloc_pages(size / 0x1000);

		memcpy((void *)addr, arctic_root_data, arctic_root_size);
		memset(arctic_root_data, 0, arctic_root_size);

		kernel->arctic_root_addr = addr;
		kernel->arctic_root_size = size;
	}

	/*
	 * Write the PCI device structures.
	 */
	{
		size_t entries = (size_t)pci_device_count;

		kernel->pci_device_count = (int)entries;

		size = entries * sizeof(kernel->pci_device[0]);
		kernel->pci_device = table_alloc(size);

		for (i = 0; i < entries; i++) {
			int group  = pci_devices[i].group;
			int bus    = pci_devices[i].bus;
			int device = pci_devices[i].device;
			int func   = pci_devices[i].func;

			phys_addr_t ecam    = pci_devices[i].ecam;
			uint32_t vendor_id  = pci_devices[i].vendor_id;
			uint32_t device_id  = pci_devices[i].device_id;
			uint32_t class_code = pci_devices[i].class_code;

			int cap_msi   = pci_devices[i].cap_msi;
			int cap_msi_x = pci_devices[i].cap_msi_x;

			kernel->pci_device[i].group  = group;
			kernel->pci_device[i].bus    = bus;
			kernel->pci_device[i].device = device;
			kernel->pci_device[i].func   = func;
			kernel->pci_device[i].ecam   = ecam;

			kernel->pci_device[i].vendor_id  = vendor_id;
			kernel->pci_device[i].device_id  = device_id;
			kernel->pci_device[i].class_code = class_code;

			kernel->pci_device[i].cap_msi   = cap_msi;
			kernel->pci_device[i].cap_msi_x = cap_msi_x;
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
