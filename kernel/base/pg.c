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
 * base/pg.c
 *      Page translation and protection
 */

#include <dancy.h>

cpu_native_t pg_kernel;
static void *pg_kernel_pde;

static mtx_t pg_mtx;
static int pg_ready;

static addr_t pg_apic_base_vaddr;

static int pg_rw_lock;
static addr_t pg_rw_entry;
static addr_t pg_rw_vaddr;

#ifdef DANCY_32

static void *pg_create_cr3(void)
{
	uint32_t *pde = (uint32_t *)mm_alloc_pages(mm_addr32, 0);

	if (pde) {
		memset(pde, 0, 0x1000);
		memcpy(pde, pg_kernel_pde, 256);
	}

	return pde;
}

static void pg_free_pte(uint32_t *pte)
{
	int i;

	for (i = 0; i < 1024; i++) {
		if ((pte[i] & 0x01) == 0)
			continue;
		mm_free_page((phys_addr_t)pte[i]);
	}
}

static void pg_delete_cr3(cpu_native_t cr3)
{
	const uint32_t entry_mask = 0xFFFFF000;
	uint32_t *pde, *pte;
	int i;

	/*
	 * Page-directory table.
	 */
	pde = (uint32_t *)cr3;

	for (i = 64; i < 1024; i++) {
		if ((pde[i] & 0x01) == 0)
			continue;
		if ((pde[i] & 0x80) != 0)
			continue;
		/*
		 * Page table.
		 */
		pte = (uint32_t *)(pde[i] & entry_mask);
		pg_free_pte(pte);

		mm_free_page((phys_addr_t)pte);
	}

	mm_free_page((phys_addr_t)pde);
}

static int pg_map_identity(phys_addr_t addr, int type, int large_page)
{
	uint32_t page_bits = 0x23;
	int offset = (int)(addr >> 22);
	uint32_t page, *ptr;

	/*
	 * Page-directory table.
	 */
	ptr = (uint32_t *)pg_kernel;

	if (!pg_kernel_pde)
		pg_kernel_pde = ptr;

	if (large_page) {
		if ((ptr[offset] & 1) != 0 && (ptr[offset] & 0x80) == 0)
			return 1;

		page = (uint32_t)(addr & 0xFFC00000);

		if ((type & pg_uncached) != 0)
			page_bits |= 0x18;

		if (page < 0x10000000)
			page_bits |= 0x100;

		page_bits |= 0x80;
		ptr[offset] = page | page_bits;

		return 0;
	}

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint32_t)heap_alloc_static_page()) == 0)
			return 1;
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint32_t)(addr & 0xFFFFF000);
	ptr = (uint32_t *)(ptr[offset] & 0xFFFFF000);

	if ((type & pg_uncached) != 0)
		page_bits |= 0x18;

	if (page < 0x10000000 && page != pg_rw_vaddr)
		page_bits |= 0x100;

	offset = (int)((addr >> 12) & 0x3FF);

	if (page == pg_apic_base_vaddr)
		ptr[offset] = (uint32_t)kernel->apic_base_addr | page_bits;
	else
		ptr[offset] = page | page_bits | (ptr[offset] & 0x18);

	return 0;
}

static int pg_map_virtual(cpu_native_t cr3, addr_t vaddr, phys_addr_t addr)
{
	uint32_t page_bits = 0x27;
	int offset = (int)(vaddr >> 22);
	uint32_t page, *ptr;

	/*
	 * Page-directory table.
	 */
	ptr = (uint32_t *)cr3;

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint32_t)mm_alloc_page()) == 0)
			return 1;
		memset((void *)page, 0, 0x1000);
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint32_t)(addr & 0xFFFFF000);
	ptr = (uint32_t *)(ptr[offset] & 0xFFFFF000);

	offset = (int)((vaddr >> 12) & 0x3FF);

	if ((ptr[offset] & 1) == 0)
		ptr[offset] = page | page_bits;
	else
		mm_free_page(addr);

	return 0;
}

void *pg_get_entry(cpu_native_t cr3, const void *pte)
{
	addr_t addr = (addr_t)pte;
	int offset = (int)(addr >> 22);
	uint32_t *ptr;

	/*
	 * Page-directory table.
	 */
	ptr = (uint32_t *)(cr3 & 0xFFFFF000);

	if ((ptr[offset] & 1) == 0 || (ptr[offset] & 0x80) != 0)
		return NULL;

	/*
	 * Page table.
	 */
	ptr = (uint32_t *)(ptr[offset] & 0xFFFFF000);
	offset = (int)((addr >> 12) & 0x3FF);

	return &ptr[offset];
}

static const phys_addr_t pg_unit_size = 0x400000;

#endif

#ifdef DANCY_64

static void *pg_create_cr3(void)
{
	uint64_t page_bits = 0x27;
	uint64_t *pml4e, *pdpe, *pde;

	if ((pml4e = (uint64_t *)mm_alloc_pages(mm_addr32, 0)) == NULL)
		return NULL;

	if ((pdpe = (uint64_t *)mm_alloc_page()) == NULL) {
		mm_free_page((phys_addr_t)pml4e);
		return NULL;
	}

	if ((pde = (uint64_t *)mm_alloc_page()) == NULL) {
		mm_free_page((phys_addr_t)pdpe);
		mm_free_page((phys_addr_t)pml4e);
		return NULL;
	}

	memset(pml4e, 0, 0x1000);
	memset(pdpe, 0, 0x1000);
	memset(pde, 0, 0x1000);

	memcpy(pde, pg_kernel_pde, 1024);

	pdpe[0] = (uint64_t)pde | page_bits;
	pml4e[0] = (uint64_t)pdpe | page_bits;

	return pml4e;
}

static void pg_free_pte(uint64_t *pte)
{
	int i;

	for (i = 0; i < 512; i++) {
		if ((pte[i] & 0x01) == 0)
			continue;
		mm_free_page((phys_addr_t)pte[i]);
	}
}

static void pg_delete_cr3(cpu_native_t cr3)
{
	const uint64_t entry_mask = 0x000FFFFFFFFFF000ull;
	uint64_t *pml4e, *pdpe, *pde, *pte;
	int i, j, k;

	/*
	 * Page-map-level-4 table.
	 */
	pml4e = (uint64_t *)cr3;

	for (i = 0; i < 256; i++) {
		if ((pml4e[i] & 1) == 0)
			continue;

		/*
		 * Page-directory-pointer table.
		 */
		pdpe = (uint64_t *)(pml4e[i] & entry_mask);

		for (j = 0; j < 512; j++) {
			if ((pdpe[j] & 1) == 0)
				continue;

			/*
			 * Page-directory table.
			 */
			pde = (uint64_t *)(pdpe[j] & entry_mask);

			for (k = (j > 0 ? 0 : 128); k < 512; k++) {
				if ((pde[k] & 0x01) == 0)
					continue;
				if ((pde[k] & 0x80) != 0)
					continue;
				/*
				 * Page table.
				 */
				pte = (uint64_t *)(pde[k] & entry_mask);
				pg_free_pte(pte);

				mm_free_page((phys_addr_t)pte);
			}

			mm_free_page((phys_addr_t)pde);
		}

		mm_free_page((phys_addr_t)pdpe);
	}

	mm_free_page((phys_addr_t)pml4e);
}

static int pg_map_identity(phys_addr_t addr, int type, int large_page)
{
	uint64_t page_bits = 0x23;
	int pml4e_offset = (int)(addr >> 39);
	int pdpe_offset = (int)((addr >> 30) & 0x1FF);
	int offset = (int)((addr >> 21) & 0x1FF);
	uint64_t page, *ptr;

	if (pml4e_offset > 0xFF)
		return 1;

	/*
	 * Page-map-level-4 table.
	 */
	ptr = (uint64_t *)pg_kernel;

	if ((ptr[pml4e_offset] & 1) == 0) {
		if ((page = (uint64_t)heap_alloc_static_page()) == 0)
			return 1;
		ptr[pml4e_offset] = page | page_bits;
	}

	/*
	 * Page-directory-pointer table.
	 */
	ptr = (uint64_t *)(ptr[pml4e_offset] & 0xFFFFFFFFFFFFF000ull);

	if ((ptr[pdpe_offset] & 1) == 0) {
		if ((page = (uint64_t)heap_alloc_static_page()) == 0)
			return 1;
		ptr[pdpe_offset] = page | page_bits;
	}

	/*
	 * Page-directory table.
	 */
	ptr = (uint64_t *)(ptr[pdpe_offset] & 0xFFFFFFFFFFFFF000ull);

	if (!pg_kernel_pde)
		pg_kernel_pde = ptr;

	if (large_page) {
		if ((ptr[offset] & 1) != 0 && (ptr[offset] & 0x80) == 0)
			return 1;

		page = (uint64_t)(addr & 0xFFFFFFFFFFE00000ull);

		if ((type & pg_uncached) != 0)
			page_bits |= 0x18;

		if (page < 0x10000000)
			page_bits |= 0x100;

		page_bits |= 0x80;
		ptr[offset] = page | page_bits;

		return 0;
	}

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint64_t)heap_alloc_static_page()) == 0)
			return 1;
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint64_t)(addr & 0xFFFFFFFFFFFFF000ull);
	ptr = (uint64_t *)(ptr[offset] & 0xFFFFFFFFFFFFF000ull);

	if ((type & pg_uncached) != 0)
		page_bits |= 0x18;

	if (page < 0x10000000 && page != pg_rw_vaddr)
		page_bits |= 0x100;

	offset = (int)((addr >> 12) & 0x1FF);

	if (page == pg_apic_base_vaddr)
		ptr[offset] = (uint64_t)kernel->apic_base_addr | page_bits;
	else
		ptr[offset] = page | page_bits | (ptr[offset] & 0x18);

	return 0;
}

static int pg_map_virtual(cpu_native_t cr3, addr_t vaddr, phys_addr_t addr)
{
	uint64_t page_bits = 0x27;
	int pml4e_offset = (int)(vaddr >> 39);
	int pdpe_offset = (int)((vaddr >> 30) & 0x1FF);
	int offset = (int)((vaddr >> 21) & 0x1FF);
	uint64_t page, *ptr;

	if (pml4e_offset > 0xFF)
		return 1;

	/*
	 * Page-map-level-4 table.
	 */
	ptr = (uint64_t *)cr3;

	if ((ptr[pml4e_offset] & 1) == 0) {
		if ((page = (uint64_t)mm_alloc_page()) == 0)
			return 1;
		memset((void *)page, 0, 0x1000);
		ptr[pml4e_offset] = page | page_bits;
	}

	/*
	 * Page-directory-pointer table.
	 */
	ptr = (uint64_t *)(ptr[pml4e_offset] & 0xFFFFFFFFFFFFF000ull);

	if ((ptr[pdpe_offset] & 1) == 0) {
		if ((page = (uint64_t)mm_alloc_page()) == 0)
			return 1;
		memset((void *)page, 0, 0x1000);
		ptr[pdpe_offset] = page | page_bits;
	}

	/*
	 * Page-directory table.
	 */
	ptr = (uint64_t *)(ptr[pdpe_offset] & 0xFFFFFFFFFFFFF000ull);

	if ((ptr[offset] & 1) == 0) {
		if ((page = (uint64_t)mm_alloc_page()) == 0)
			return 1;
		memset((void *)page, 0, 0x1000);
		ptr[offset] = page | page_bits;
	} else if ((ptr[offset] & 0x80) != 0) {
		return 1;
	}

	/*
	 * Page table.
	 */
	page = (uint64_t)(addr & 0xFFFFFFFFFFFFF000ull);
	ptr = (uint64_t *)(ptr[offset] & 0xFFFFFFFFFFFFF000ull);

	offset = (int)((vaddr >> 12) & 0x1FF);

	if ((ptr[offset] & 1) == 0)
		ptr[offset] = page | page_bits;
	else
		mm_free_page(addr);

	return 0;
}

void *pg_get_entry(cpu_native_t cr3, const void *pte)
{
	addr_t addr = (addr_t)pte;
	int pml4e_offset = (int)(addr >> 39);
	int pdpe_offset = (int)((addr >> 30) & 0x1FF);
	int offset = (int)((addr >> 21) & 0x1FF);
	uint64_t *ptr;

	if (pml4e_offset > 0xFF)
		return NULL;

	/*
	 * Page-map-level-4 table.
	 */
	ptr = (uint64_t *)(cr3 & 0xFFFFF000);

	if ((ptr[pml4e_offset] & 1) == 0)
		return NULL;

	/*
	 * Page-directory-pointer table.
	 */
	ptr = (uint64_t *)(ptr[pml4e_offset] & 0xFFFFF000);

	if ((ptr[pdpe_offset] & 1) == 0)
		return NULL;

	/*
	 * Page-directory table.
	 */
	ptr = (uint64_t *)(ptr[pdpe_offset] & 0xFFFFF000);

	if ((ptr[offset] & 1) == 0 || (ptr[offset] & 0x80) != 0)
		return NULL;

	/*
	 * Page table.
	 */
	ptr = (uint64_t *)(ptr[offset] & 0xFFFFF000);
	offset = (int)((addr >> 12) & 0x1FF);

	return &ptr[offset];
}

static const phys_addr_t pg_unit_size = 0x200000;

#endif

int pg_init(void)
{
	static int run_once;

	size_t map_size = kernel->memory_map_size;
	int map_entries = (int)(map_size / sizeof(kernel->memory_map[0]));
	int i, j, r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&pg_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if ((pg_kernel = (cpu_native_t)heap_alloc_static_page()) == 0)
		return DE_MEMORY;

	if ((pg_rw_vaddr = (addr_t)heap_alloc_static_page()) == 0)
		return DE_MEMORY;

	pg_apic_base_vaddr = kernel->apic_base_vaddr;
	if (pg_apic_base_vaddr == 0 || (pg_apic_base_vaddr & 0x0FFF) != 0)
		return DE_UNEXPECTED;

	/*
	 * Map the modules (4 KiB pages).
	 */
	for (i = 0; i < kernel->module_count; i++) {
		size_t size;
		int map_count;
		phys_addr_t addr;

		/*
		 * The .text section.
		 */
		size = kernel->module[i].text_size;
		map_count = (int)((size + 0x0FFF) / 0x1000);
		addr = (phys_addr_t)kernel->module[i].text_addr;

		for (j = 0; j < map_count; j++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}

		/*
		 * The .rdata section.
		 */
		size = kernel->module[i].rdata_size;
		map_count = (int)((size + 0x0FFF) / 0x1000);
		addr = (phys_addr_t)kernel->module[i].rdata_addr;

		for (j = 0; j < map_count; j++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}

		/*
		 * The .data section.
		 */
		size = kernel->module[i].data_size;
		map_count = (int)((size + 0x0FFF) / 0x1000);
		addr = (phys_addr_t)kernel->module[i].data_addr;

		for (j = 0; j < map_count; j++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}

		/*
		 * The .bss section.
		 */
		size = kernel->module[i].bss_size;
		map_count = (int)((size + 0x0FFF) / 0x1000);
		addr = (phys_addr_t)kernel->module[i].bss_addr;

		for (j = 0; j < map_count; j++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}
	}

	/*
	 * Map the memory slot for read and write functions.
	 */
	pg_map_kernel((phys_addr_t)pg_rw_vaddr, 0x1000, pg_uncached);
	pg_rw_entry = (addr_t)pg_get_entry(pg_kernel, (void *)pg_rw_vaddr);

	if (pg_rw_entry == 0)
		return DE_UNEXPECTED;

	/*
	 * Create a special mapping for Local APIC registers.
	 */
	pg_map_kernel((phys_addr_t)pg_apic_base_vaddr, 0x1000, pg_uncached);

	/*
	 * Map the I/O APIC memory registers (IOREGSEL and IOWIN).
	 */
	if (kernel->io_apic_enabled) {
		for (i = 0; i < kernel->io_apic_count; i++) {
			phys_addr_t addr = kernel->io_apic[i].addr;

			pg_map_kernel(addr, 0x20, pg_uncached);
		}
	}

	/*
	 * Map the framebuffer (4 KiB pages).
	 */
	{
		size_t size = (size_t)(kernel->fb_height * kernel->fb_stride);
		int map_count = (int)((size + 0x0FFF) / 0x1000);
		phys_addr_t addr = kernel->fb_addr;

		for (i = 0; i < map_count; i++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}
	}

	/*
	 * Map the standard framebuffer (4 KiB pages).
	 */
	{
		size_t size = kernel->fb_standard_size;
		int map_count = (int)((size + 0x0FFF) / 0x1000);
		phys_addr_t addr = kernel->fb_standard_addr;

		for (i = 0; i < map_count; i++) {
			pg_map_kernel(addr, 0x1000, pg_normal);
			addr += 0x1000;
		}
	}

	/*
	 * Map the PCI memory mapped configuration areas (4 KiB pages).
	 */
	for (i = 0; i < kernel->pci_device_count; i++) {
		phys_addr_t addr = kernel->pci_device[i].ecam;

		if (addr)
			pg_map_kernel(addr, 0x1000, pg_uncached);
	}

	/*
	 * Map other kernel memory areas. Use large pages if possible.
	 * It is safe to run the pg_map_kernel function for areas that
	 * have been mapped already.
	 */
	for (i = 0; i < map_entries - 1; i++) {
		const uint32_t type_kernel = 0xE0000000;
		const uint32_t type_kernel_reserved = 0xE000FFFF;

		uint32_t type = kernel->memory_map[i].type;
		uint64_t base, next, size;

		if (type != type_kernel && type != type_kernel_reserved)
			continue;

		base = kernel->memory_map[i + 0].base;
		next = kernel->memory_map[i + 1].base;

		if (next != 0 && base > next)
			return DE_UNEXPECTED;
#ifdef DANCY_32
		if (next > 0x100000000ull)
			break;
#endif
		size = next - base;
		pg_map_kernel((phys_addr_t)base, (size_t)size, pg_normal);
	}

	cpu_write32((uint32_t *)&pg_ready, 1);

	r = cpu_ints(0);
	cpu_write_cr4(cpu_read_cr4() | (1u << 7) | (1u << 4));
	cpu_write_cr3(pg_kernel);

	/*
	 * Update the timer interrupt handler (I/O APIC)
	 * and restore the interrupt flag.
	 */
	{
		uint32_t a, *p;

		a = (uint32_t)kernel->apic_base_vaddr;
		p = (uint32_t *)((addr_t)&timer_apic_base[0]);
		cpu_write32(p, a);
#ifdef DANCY_64
		a = (uint32_t)(kernel->apic_base_vaddr >> 32);
		p = (uint32_t *)((addr_t)&timer_apic_base[4]);
		cpu_write32(p, a);
#endif
		cpu_ints(r);
	}

	return 0;
}

int pg_init_ap(void)
{
	while (cpu_read32((const uint32_t *)&pg_ready) == 0)
		delay(1000000);

	if (pg_kernel == 0 || (pg_kernel & 0x0FFF) != 0)
		return DE_UNEXPECTED;

	cpu_write_cr4(cpu_read_cr4() | (1u << 7) | (1u << 4));
	cpu_write_cr3(pg_kernel);

	return 0;
}

int pg_create(void)
{
	struct task *current = task_current();
	cpu_native_t cr3;

	if (current->pg_cr3)
		return DE_UNEXPECTED;

	if ((cr3 = (cpu_native_t)pg_create_cr3()) == 0)
		return DE_MEMORY;

	current->pg_cr3 = (uint32_t)cr3;
	current->cr3 = (uint32_t)cr3;
	cpu_write_cr3(cr3);

	return 0;
}

void pg_delete(void)
{
	struct task *current = task_current();
	cpu_native_t cr3;

	if ((cr3 = (cpu_native_t)current->pg_cr3) == 0)
		return;

	current->pg_cr3 = (uint32_t)pg_kernel;
	current->cr3 = (uint32_t)pg_kernel;
	cpu_write_cr3(pg_kernel);

	pg_delete_cr3(cr3);
}

void pg_enter_kernel(void)
{
	struct task *current = task_current();

	current->cr3 = (uint32_t)pg_kernel;
	cpu_add32(&current->pg_state, 1);

	if (cpu_read_cr3() == pg_kernel)
		return;

	cpu_write_cr3(pg_kernel);
}

void pg_leave_kernel(void)
{
	struct task *current = task_current();
	cpu_native_t cr3;

	if (cpu_sub32(&current->pg_state, 1) != 0)
		return;

	if ((cr3 = (cpu_native_t)current->pg_cr3) == 0)
		return;

	current->cr3 = (uint32_t)cr3;
	cpu_write_cr3(cr3);
}

void *pg_map_kernel(phys_addr_t addr, size_t size, int type)
{
	const phys_addr_t page_mask = 0x0FFF;
	const phys_addr_t unit_mask = pg_unit_size - 1;
	phys_addr_t addr_beg, addr_end, addr_sub;

	if (size == 0 || addr > (SIZE_MAX - size) + 1)
		return NULL;

	addr_beg = addr & (~page_mask);
	addr_end = ((phys_addr_t)(addr + size) + page_mask) & (~page_mask);

	if (addr_beg == 0)
		addr_beg += 0x1000;

	if (mtx_lock(&pg_mtx) != thrd_success)
		return NULL;

	while ((addr_sub = addr_end - addr_beg) != 0) {
		if ((addr_beg & unit_mask) == 0 && addr_sub >= pg_unit_size) {
			if (!pg_map_identity(addr_beg, type, 1)) {
				addr_beg += pg_unit_size;
				continue;
			}
		}

		if (pg_map_identity(addr_beg, type, 0)) {
			mtx_unlock(&pg_mtx);
			return NULL;
		}

		addr_beg += 0x1000;
	}

	mtx_unlock(&pg_mtx);

	cpu_write_cr3(cpu_read_cr3());

	return (void *)addr;
}

void *pg_map_user(addr_t vaddr, size_t size)
{
	const addr_t page_mask = 0x0FFF;
	addr_t vaddr_beg, vaddr_end;
	phys_addr_t addr;

	struct task *current = task_current();
	cpu_native_t cr3 = cpu_read_cr3();

	if (cr3 == pg_kernel || cr3 != (cpu_native_t)current->cr3)
		return NULL;

	if (size == 0 || vaddr < 0x10000000)
		return NULL;

	if ((vaddr > (SIZE_MAX - size) + 1))
		return NULL;

	vaddr_beg = vaddr & (~page_mask);
	vaddr_end = ((addr_t)(vaddr + size) + page_mask) & (~page_mask);

	pg_enter_kernel();

	while ((vaddr_end - vaddr_beg) != 0) {
		if ((addr = mm_alloc_page()) == 0) {
			vaddr = 0;
			break;
		}

		memset((void *)addr, 0, 0x1000);

		if (pg_map_virtual(cr3, vaddr_beg, addr)) {
			mm_free_page(addr);
			vaddr = 0;
			break;
		}

		vaddr_beg += 0x1000;
	}

	pg_leave_kernel();

	return (void *)vaddr;
}

uint64_t pg_read_memory(phys_addr_t addr, size_t size)
{
	void *lock_local = &pg_rw_lock;
	uint64_t r = 0;

	const cpu_native_t page_mask = 0x0FFF;
	cpu_native_t page_addr = (cpu_native_t)addr & (~page_mask);
	cpu_native_t *entry = (cpu_native_t *)pg_rw_entry;

	int offset = (int)((cpu_native_t)addr & page_mask);
	addr_t access_vaddr = pg_rw_vaddr + (addr_t)offset;

#ifdef DANCY_64
	if ((page_addr & 0xFFF0000000000000ull) != 0)
		return r;
#endif
	spin_enter(&lock_local);

	*entry = (*entry & page_mask) | page_addr;
	cpu_invlpg((const void *)access_vaddr);

	if (size == 1)
		r = (uint64_t)cpu_read8((const void *)access_vaddr);

	else if (size == 2 && offset <= 0x0FFE)
		r = (uint64_t)cpu_read16((const void *)access_vaddr);

	else if (size == 4 && offset <= 0x0FFC)
		r = (uint64_t)cpu_read32((const void *)access_vaddr);

	else if (size == 8 && offset <= 0x0FF8)
		r = (uint64_t)cpu_read64((const void *)access_vaddr);

	spin_leave(&lock_local);

	return r;
}

void pg_write_memory(phys_addr_t addr, uint64_t val, size_t size)
{
	void *lock_local = &pg_rw_lock;

	const cpu_native_t page_mask = 0x0FFF;
	cpu_native_t page_addr = (cpu_native_t)addr & (~page_mask);
	cpu_native_t *entry = (cpu_native_t *)pg_rw_entry;

	int offset = (int)((cpu_native_t)addr & page_mask);
	addr_t access_vaddr = pg_rw_vaddr + (addr_t)offset;

#ifdef DANCY_64
	if ((page_addr & 0xFFF0000000000000ull) != 0)
		return;
#endif
	spin_enter(&lock_local);

	*entry = (*entry & page_mask) | page_addr;
	cpu_invlpg((const void *)access_vaddr);

	if (size == 1)
		cpu_write8((void *)access_vaddr, (uint8_t)val);

	else if (size == 2 && offset <= 0x0FFE)
		cpu_write16((void *)access_vaddr, (uint16_t)val);

	else if (size == 4 && offset <= 0x0FFC)
		cpu_write32((void *)access_vaddr, (uint32_t)val);

	else if (size == 8 && offset <= 0x0FF8)
		cpu_write64((void *)access_vaddr, (uint64_t)val);

	spin_leave(&lock_local);
}
