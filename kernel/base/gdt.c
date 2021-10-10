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
 * base/gdt.c
 *      Global Descriptor Table
 */

#include <dancy.h>

static int gdt_lock = 1;
static int gdt_ready = 0;

struct gdt_block {
	uint16_t pad1[3];
	uint16_t table_limit;
	uint32_t table_addr;
	uint32_t table_addr_high;

	uint32_t id;
	uint32_t tss_addr;
	uint32_t pad2[2];

	uint8_t table[80];
	uint8_t tss[144];
};

static struct gdt_block *gdt_array = NULL;
static int gdt_count = 0;
static int gdt_used = 0;

static uint32_t gdt_early_apic_id(void)
{
	const void *id;

	if (!kernel->apic_enabled)
		return kernel->apic_bsp_id;

	/*
	 * Use the physical APIC base address because the
	 * kernel paging has not been set up at this point.
	 */
	id = (const void *)(kernel->apic_base_addr + 0x20);

	return (cpu_read32(id) >> 24);
}

#ifdef DANCY_32

static void gdt_build_block(struct gdt_block *gb)
{
	uint8_t *p;

	gb->table_limit = (uint16_t)(sizeof(gb->table) - 1);
	gb->table_addr = (uint32_t)((addr_t)&gb->table[0]);
	gb->id = gdt_early_apic_id();

	p = &gb->table[gdt_kernel_code];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0x9B, p[6] = 0xCF, p[7] = 0x00;

	p = &gb->table[gdt_kernel_data];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0x93, p[6] = 0xCF, p[7] = 0x00;

	p = &gb->table[gdt_user_code];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0xFB, p[6] = 0xCF, p[7] = 0x00;

	p = &gb->table[gdt_user_data];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0xF3, p[6] = 0xCF, p[7] = 0x00;

	{
		addr_t tss_addr = (uint32_t)((addr_t)&gb->tss[0]);

		gb->tss_addr = (uint32_t)tss_addr;

		p = &gb->table[gdt_task_state];
		p[0] = (uint8_t)(sizeof(gb->tss) - 1);
		p[1] = 0x00;
		p[2] = (uint8_t)(tss_addr);
		p[3] = (uint8_t)(tss_addr >> 8);
		p[4] = (uint8_t)(tss_addr >> 16);
		p[5] = 0x89;
		p[6] = 0x00;
		p[7] = (uint8_t)(tss_addr >> 24);

		p = (uint8_t *)tss_addr;
		p[8] = (uint8_t)gdt_kernel_data;
		p[9] = 0x00;
		p[102] = (uint8_t)(sizeof(gb->tss));
	}

	{
		addr_t gdt_block_addr = (uint32_t)((addr_t)gb);

		p = &gb->table[gdt_block_data];
		p[0] = 0xFF;
		p[1] = 0xFF;
		p[2] = (uint8_t)(gdt_block_addr);
		p[3] = (uint8_t)(gdt_block_addr >> 8);
		p[4] = (uint8_t)(gdt_block_addr >> 16);
		p[5] = 0x93;
		p[6] = 0x40;
		p[7] = (uint8_t)(gdt_block_addr >> 24);
	}
}

#endif

#ifdef DANCY_64

static void gdt_build_block(struct gdt_block *gb)
{
	uint8_t *p;

	gb->table_limit = (uint16_t)(sizeof(gb->table) - 1);
	gb->table_addr = (uint32_t)((addr_t)&gb->table[0]);
	gb->id = gdt_early_apic_id();

	p = &gb->table[gdt_kernel_code];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0x9B, p[6] = 0xAF, p[7] = 0x00;

	p = &gb->table[gdt_kernel_data];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0x93, p[6] = 0xCF, p[7] = 0x00;

	p = &gb->table[gdt_user_code];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0xFB, p[6] = 0xAF, p[7] = 0x00;

	p = &gb->table[gdt_user_data];
	p[0] = 0xFF, p[1] = 0xFF, p[2] = 0x00, p[3] = 0x00;
	p[4] = 0x00, p[5] = 0xF3, p[6] = 0xCF, p[7] = 0x00;

	{
		addr_t tss_addr = (uint32_t)((addr_t)&gb->tss[0]);

		gb->tss_addr = (uint32_t)tss_addr;

		p = &gb->table[gdt_task_state];
		p[0] = (uint8_t)(sizeof(gb->tss) - 1);
		p[1] = 0x00;
		p[2] = (uint8_t)(tss_addr);
		p[3] = (uint8_t)(tss_addr >> 8);
		p[4] = (uint8_t)(tss_addr >> 16);
		p[5] = 0x89;
		p[6] = 0x00;
		p[7] = (uint8_t)(tss_addr >> 24);

		p = (uint8_t *)tss_addr;
		p[102] = (uint8_t)(sizeof(gb->tss));
	}

	{
		addr_t gdt_block_addr = (uint32_t)((addr_t)gb);

		p = &gb->table[gdt_block_data];
		p[0] = 0xFF;
		p[1] = 0xFF;
		p[2] = (uint8_t)(gdt_block_addr);
		p[3] = (uint8_t)(gdt_block_addr >> 8);
		p[4] = (uint8_t)(gdt_block_addr >> 16);
		p[5] = 0x93;
		p[6] = 0x40;
		p[7] = (uint8_t)(gdt_block_addr >> 24);
	}
}

#endif

int gdt_init(void)
{
	static int run_once;
	struct gdt_block *gb;
	size_t size;
	int i, r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (kernel->apic_bsp_id != gdt_early_apic_id())
		return DE_UNEXPECTED;

	gdt_count = kernel->smp_ap_count + 1;

	size = (size_t)gdt_count * sizeof(gdt_array[0]);
	if ((size & 255) != 0)
		return DE_UNEXPECTED;

	gdt_array = aligned_alloc(256, size);
	if (!gdt_array)
		return DE_MEMORY;

	memset(gdt_array, 0, size);

	gb = &gdt_array[gdt_used++];

	spin_unlock(&gdt_lock);

	gdt_build_block(gb);

	r = cpu_ints(0);
	gdt_load(&gb->table_limit);

	gdt_load_cs(gdt_kernel_code);
	gdt_load_es(gdt_kernel_data);
	gdt_load_ss(gdt_kernel_data);
	gdt_load_ds(gdt_kernel_data);
	gdt_load_fs(0);
	gdt_load_gs(0);
	gdt_load_tss(gdt_task_state);

	cpu_ints(r);

	/*
	 * Wait until all APs have initialized the global descriptor table.
	 */
	for (i = 0; i < 10000; i++) {
		r = (int)cpu_read32((const uint32_t *)&gdt_used);

		if (r == gdt_count) {
			cpu_write32((uint32_t *)&gdt_ready, 1);
			return 0;
		}

		delay(100000);
	}

	return DE_UNEXPECTED;;
}

int gdt_init_ap(void)
{
	struct gdt_block *gb = NULL;
	int r;

	if (kernel->apic_bsp_id == gdt_early_apic_id())
		return DE_UNEXPECTED;

	spin_lock(&gdt_lock);

	if (gdt_used < gdt_count)
		gb = &gdt_array[gdt_used++];

	spin_unlock(&gdt_lock);

	if (!gb)
		return DE_UNEXPECTED;

	gdt_build_block(gb);

	r = cpu_ints(0);
	gdt_load(&gb->table_limit);

	gdt_load_cs(gdt_kernel_code);
	gdt_load_es(gdt_kernel_data);
	gdt_load_ss(gdt_kernel_data);
	gdt_load_ds(gdt_kernel_data);
	gdt_load_fs(0);
	gdt_load_gs(0);
	gdt_load_tss(gdt_task_state);

	cpu_ints(r);

	return 0;
}

void *gdt_get_tss(void)
{
	uint32_t id = apic_id();
	int i;

	if (!gdt_ready)
		return NULL;

	for (i = 0; i < gdt_count; i++) {
		if (gdt_array[i].id == id)
			return (void *)(&gdt_array[i].tss[0]);
	}

	return NULL;
}
