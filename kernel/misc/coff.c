/*
 * Copyright (c) 2022 Antti Tiihala
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
 * misc/coff.c
 *      User space executable loader
 */

#include <dancy.h>

#if DANCY_32
const unsigned int coff_native_signature = 0x014C;
static const char *coff_start = "___start";
#endif

#if DANCY_64
const unsigned int coff_native_signature = 0x8664;
static const char *coff_start = "__start";
#endif

struct coff {
	addr_t text_vaddr;
	addr_t rdata_vaddr;
	addr_t data_vaddr;
	addr_t bss_vaddr;

	addr_t stack_vaddr[4];

	uint32_t symbol_table;
	uint32_t symbol_count;

	size_t text_size;
	size_t rdata_size;
	size_t data_size;
	size_t bss_size;

	size_t text_aligned_size;
	size_t rdata_aligned_size;
	size_t data_aligned_size;
	size_t bss_aligned_size;
	size_t stack_aligned_size;

	uint32_t text_sec;
	uint32_t rdata_sec;
	uint32_t data_sec;
	uint32_t bss_sec;

	uint32_t text_rel;
	uint32_t rdata_rel;
	uint32_t data_rel;
	uint32_t bss_rel;

	uint32_t text_nrel;
	uint32_t rdata_nrel;
	uint32_t data_nrel;
	uint32_t bss_nrel;

	size_t text_rel_size;
	size_t rdata_rel_size;
	size_t data_rel_size;
};

static int coff_validate(const unsigned char *obj)
{
	if (LE16(&obj[0]) != coff_native_signature)
		return DE_COFF_SIGNATURE;
	if (LE16(&obj[2]) != 4)
		return DE_COFF_SECTION;
	if (obj[16] || obj[17])
		return DE_COFF_HEADER;

	if (memcmp(&obj[20], ".text", 6))
		return DE_COFF_SECTION;
	if (memcmp(&obj[60], ".rdata", 7))
		return DE_COFF_SECTION;
	if (memcmp(&obj[100], ".data", 6))
		return DE_COFF_SECTION;
	if (memcmp(&obj[140], ".bss", 5))
		return DE_COFF_SECTION;

	if ((LE32(&obj[20+36])  & 0xFF0FFFFF) != 0x60000020)
		return DE_COFF_FLAG;
	if ((LE32(&obj[60+36])  & 0xFF0FFFFF) != 0x40000040)
		return DE_COFF_FLAG;
	if ((LE32(&obj[100+36]) & 0xFF0FFFFF) != 0xC0000040)
		return DE_COFF_FLAG;
	if ((LE32(&obj[140+36]) & 0xFF0FFFFF) != 0xC0000080)
		return DE_COFF_FLAG;

	return 0;
}

static int coff_allocate(struct coff *coff)
{
	addr_t vaddr = 0x80000000;
	size_t size;

	vaddr -= (addr_t)coff->stack_aligned_size;

	if ((addr_t)pg_map_user(vaddr, coff->stack_aligned_size) != vaddr)
		return DE_MEMORY;

	if ((coff->stack_vaddr[0] = vaddr) < 0x78000000)
		return DE_UNEXPECTED;

	coff->stack_vaddr[1] = coff->stack_vaddr[0];
	coff->stack_vaddr[1] += (addr_t)(coff->text_rel_size);

	coff->stack_vaddr[2] = coff->stack_vaddr[1];
	coff->stack_vaddr[2] += (addr_t)(coff->rdata_rel_size);

	coff->stack_vaddr[3] = coff->stack_vaddr[2];
	coff->stack_vaddr[3] += (addr_t)(coff->data_rel_size);

	vaddr = 0x10000000;

	/*
	 * .text section mapping.
	 */
	size = coff->text_aligned_size;
	coff->text_vaddr = size ? vaddr : (addr_t)0;

	if (size && (addr_t)pg_map_user(vaddr, size) != vaddr)
		return DE_MEMORY;

	vaddr += (addr_t)size;

	/*
	 * .rdata section mapping.
	 */
	size = coff->rdata_aligned_size;
	coff->rdata_vaddr = size ? vaddr : (addr_t)0;

	if (size && (addr_t)pg_map_user(vaddr, size) != vaddr)
		return DE_MEMORY;

	vaddr += (addr_t)size;

	/*
	 * .data section mapping.
	 */
	size = coff->data_aligned_size;
	coff->data_vaddr = size ? vaddr : (addr_t)0;

	if (size && (addr_t)pg_map_user(vaddr, size) != vaddr)
		return DE_MEMORY;

	vaddr += (addr_t)size;

	/*
	 * .bss section mapping.
	 */
	size = coff->bss_aligned_size;
	coff->bss_vaddr = size ? vaddr : (addr_t)0;

	if (size && (addr_t)pg_map_user(vaddr, size) != vaddr)
		return DE_MEMORY;

	vaddr += (addr_t)size;

	if (vaddr >= 0x78000000)
		return DE_UNEXPECTED;

	return 0;
}

static int coff_copy(struct vfs_node *node,
	uint32_t offset, size_t size, addr_t vaddr)
{
	size_t s = size;
	void *section = (void *)vaddr;
	int r;

	if ((r = node->n_read(node, (uint64_t)offset, &s, section)) != 0)
		return r;

	if (s != size)
		return DE_COFF_FORMAT;

	return 0;
}

int coff_load_executable(struct vfs_node *node, addr_t *start_addr)
{
	unsigned char *obj;
	struct coff *coff;
	size_t size;
	int r;

	*start_addr = 0;

	if (!task_current()->pg_cr3)
		return DE_UNINITIALIZED;

	if ((obj = pg_map_user(0x7FFFF000, 0x1000)) == NULL)
		return DE_MEMORY;

	if ((size = 180, (r = node->n_read(node, 0, &size, obj))) != 0)
		return r;

	if (size != 180)
		return DE_COFF_SIZE;

	if ((r = coff_validate(obj)) != 0)
		return r;

	coff = (struct coff *)((void *)(obj + 256));

	coff->symbol_table = (uint32_t)LE32(&obj[8]);
	coff->symbol_count = (uint32_t)LE32(&obj[12]);

	if (coff->symbol_count < 4)
		return DE_COFF_SYMBOL;

	coff->text_size  = (size_t)LE32(&obj[20 + (0 * 40) + 16]);
	coff->rdata_size = (size_t)LE32(&obj[20 + (1 * 40) + 16]);
	coff->data_size  = (size_t)LE32(&obj[20 + (2 * 40) + 16]);
	coff->bss_size   = (size_t)LE32(&obj[20 + (3 * 40) + 16]);

	coff->text_sec  = (uint32_t)LE32(&obj[20 + (0 * 40) + 20]);
	coff->rdata_sec = (uint32_t)LE32(&obj[20 + (1 * 40) + 20]);
	coff->data_sec  = (uint32_t)LE32(&obj[20 + (2 * 40) + 20]);
	coff->bss_sec   = (uint32_t)LE32(&obj[20 + (3 * 40) + 20]);

	coff->text_rel  = (uint32_t)LE32(&obj[20 + (0 * 40) + 24]);
	coff->rdata_rel = (uint32_t)LE32(&obj[20 + (1 * 40) + 24]);
	coff->data_rel  = (uint32_t)LE32(&obj[20 + (2 * 40) + 24]);
	coff->bss_rel   = (uint32_t)LE32(&obj[20 + (3 * 40) + 24]);

	coff->text_nrel  = (uint32_t)LE16(&obj[20 + (0 * 40) + 32]);
	coff->rdata_nrel = (uint32_t)LE16(&obj[20 + (1 * 40) + 32]);
	coff->data_nrel  = (uint32_t)LE16(&obj[20 + (2 * 40) + 32]);
	coff->bss_nrel   = (uint32_t)LE16(&obj[20 + (3 * 40) + 32]);

	if (coff->bss_sec || coff->bss_rel || coff->bss_nrel)
		return DE_COFF_SECTION;

	{
		const uint64_t a  = 0x00000FFF;
		const uint64_t m = ~a;
		uint64_t total_size = 0;
		uint64_t stack_size = 0x1000;

		uint64_t text_aligned_size, rdata_aligned_size;
		uint64_t data_aligned_size, bss_aligned_size;
		uint64_t stack_aligned_size;

		text_aligned_size  = ((uint64_t)coff->text_size  + a) & m;
		rdata_aligned_size = ((uint64_t)coff->rdata_size + a) & m;
		data_aligned_size  = ((uint64_t)coff->data_size  + a) & m;
		bss_aligned_size   = ((uint64_t)coff->bss_size   + a) & m;

		/*
		 * There will always be a .bss section allocated.
		 */
		if (!bss_aligned_size)
			bss_aligned_size = 0x1000;

		total_size += text_aligned_size;
		total_size += rdata_aligned_size;
		total_size += data_aligned_size;
		total_size += bss_aligned_size;

		if (total_size > 0x60000000)
			return DE_COFF_SIZE;

		stack_size += (uint64_t)coff->symbol_count * (uint64_t)18;
		stack_size += (uint64_t)coff->text_nrel    * (uint64_t)10;
		stack_size += (uint64_t)coff->rdata_nrel   * (uint64_t)10;
		stack_size += (uint64_t)coff->data_nrel    * (uint64_t)10;

		stack_aligned_size = (stack_size + a) & m;

		if (stack_aligned_size <= 0x1000)
			return DE_UNEXPECTED;

		if (stack_aligned_size > 0x08000000)
			return DE_COFF_SIZE;

		coff->text_aligned_size  = (size_t)text_aligned_size;
		coff->rdata_aligned_size = (size_t)rdata_aligned_size;
		coff->data_aligned_size  = (size_t)data_aligned_size;
		coff->bss_aligned_size   = (size_t)bss_aligned_size;
		coff->stack_aligned_size = (size_t)stack_aligned_size;

		coff->text_rel_size  = coff->text_nrel  * 10;
		coff->rdata_rel_size = coff->rdata_nrel * 10;
		coff->data_rel_size  = coff->data_nrel  * 10;
	}

	if ((r = coff_allocate(coff)) != 0)
		return r;

	/*
	 * Copy all the sections.
	 */
	if (coff->text_size) {
		uint32_t o = coff->text_sec;
		size_t s = coff->text_size;
		addr_t v = coff->text_vaddr;

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	if (coff->rdata_size) {
		uint32_t o = coff->rdata_sec;
		size_t s = coff->rdata_size;
		addr_t v = coff->rdata_vaddr;

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	if (coff->data_size) {
		uint32_t o = coff->data_sec;
		size_t s = coff->data_size;
		addr_t v = coff->data_vaddr;

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	/*
	 * Copy all the relocations.
	 */
	if (coff->text_rel_size) {
		uint32_t o = coff->text_rel;
		size_t s = coff->text_rel_size;
		addr_t v = coff->stack_vaddr[0];

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	if (coff->rdata_rel_size) {
		uint32_t o = coff->rdata_rel;
		size_t s = coff->rdata_rel_size;
		addr_t v = coff->stack_vaddr[1];

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	if (coff->data_rel_size) {
		uint32_t o = coff->data_rel;
		size_t s = coff->data_rel_size;
		addr_t v = coff->stack_vaddr[2];

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	/*
	 * Copy the symbol table.
	 */
	{
		uint32_t o = coff->symbol_table;
		size_t s = (size_t)(coff->symbol_count * 18);
		addr_t v = coff->stack_vaddr[3];

		if ((r = coff_copy(node, o, s, v)) != 0)
			return r;
	}

	/*
	 * Add the base address to the symbol values and
	 * find the start symbol.
	 */
	{
		unsigned char *st = (void *)coff->stack_vaddr[3];
		uint32_t i;

		for (i = 0; i < coff->symbol_count; i++) {
			unsigned long val = LE32(&st[8]);
			int section_number = (int)LE16(&st[12]);

			if (st[17])
				return DE_COFF_SYMBOL;

			if (!section_number)
				return DE_COFF_EXTERNAL;

			switch (section_number) {
			case 1:
				val += (unsigned long)(coff->text_vaddr);
				if (*start_addr == 0 && st[16] == 2) {
					const char *s = (const char *)st;
					if (!strncmp(s, coff_start, 8))
						*start_addr = (addr_t)val;
				}
				break;
			case 2:
				val += (unsigned long)(coff->rdata_vaddr);
				break;
			case 3:
				val += (unsigned long)(coff->data_vaddr);
				break;
			case 4:
				val += (unsigned long)(coff->bss_vaddr);
				break;
			default:
				break;
			}
			W_LE32(&st[8], val);
			st += 18;
		}
	}

	/*
	 * Relocate .text section.
	 */
	if (coff->text_nrel) {
		void *base = (void *)coff->text_vaddr;
		unsigned char *reloc = (void *)coff->stack_vaddr[0];
		unsigned char *st = (void *)coff->stack_vaddr[3];
		unsigned char *symbol;
		uint32_t i;

		for (i = 0; i < coff->text_nrel; i++) {
			uint32_t st_v = (uint32_t)LE32(&reloc[0]);
			uint32_t st_i = (uint32_t)LE32(&reloc[4]);

			if (st_v >= coff->text_size)
				return DE_COFF_RELOCATION;

			if (st_i >= coff->symbol_count)
				return DE_COFF_RELOCATION;

			symbol = st + (st_i * 18);
			coff_relocate(base, reloc, symbol);
			reloc += 10;
		}
	}

	/*
	 * Relocate .rdata section.
	 */
	if (coff->rdata_nrel) {
		void *base = (void *)coff->rdata_vaddr;
		unsigned char *reloc = (void *)coff->stack_vaddr[1];
		unsigned char *st = (void *)coff->stack_vaddr[3];
		unsigned char *symbol;
		uint32_t i;

		for (i = 0; i < coff->rdata_nrel; i++) {
			uint32_t st_v = (uint32_t)LE32(&reloc[0]);
			uint32_t st_i = (uint32_t)LE32(&reloc[4]);

			if (st_v >= coff->rdata_size)
				return DE_COFF_RELOCATION;

			if (st_i >= coff->symbol_count)
				return DE_COFF_RELOCATION;

			symbol = st + (st_i * 18);
			coff_relocate(base, reloc, symbol);
			reloc += 10;
		}
	}

	/*
	 * Relocate .data section.
	 */
	if (coff->data_nrel) {
		void *base = (void *)coff->data_vaddr;
		unsigned char *reloc = (void *)coff->stack_vaddr[2];
		unsigned char *st = (void *)coff->stack_vaddr[3];
		unsigned char *symbol;
		uint32_t i;

		for (i = 0; i < coff->data_nrel; i++) {
			uint32_t st_v = (uint32_t)LE32(&reloc[0]);
			uint32_t st_i = (uint32_t)LE32(&reloc[4]);

			if (st_v >= coff->data_size)
				return DE_COFF_RELOCATION;

			if (st_i >= coff->symbol_count)
				return DE_COFF_RELOCATION;

			symbol = st + (st_i * 18);
			coff_relocate(base, reloc, symbol);
			reloc += 10;
		}
	}

	/*
	 * Check the start symbol value.
	 */
	if (*start_addr < 0x10000000 || *start_addr < coff->text_vaddr)
		return DE_COFF_START;
	if (*start_addr >= coff->text_vaddr + (addr_t)coff->text_size)
		return DE_COFF_START;

	/*
	 * Clear the stack memory area.
	 */
	memset((void *)coff->stack_vaddr[0], 0, coff->stack_aligned_size);

	return 0;
}
