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
 * init/ld.c
 *      Dynamic linker and loader
 */

#include <init.h>

#if defined (DANCY_32)
static const unsigned obj_magic = 0x014C;
#elif defined (DANCY_64)
static const unsigned obj_magic = 0x8664;
#endif

struct global_symbol *global_symbols;
size_t global_symbols_size;

static struct global_symbol *symtab, *symtab_end, *symtab_ptr;

static int ld_error(const char *name, const char *msg)
{
	b_print("Error: %s (%s)\n", name, msg);
	return 1;
}

int ld_init(size_t symbols)
{
	symtab = calloc(symbols, sizeof(symtab[0]));

	if (!symtab)
		return ld_error("ld_init", "out of memory");

	symtab_end = &symtab[symbols - 1];
	symtab_ptr = &symtab[0];

	return 0;
}

int ld_add(const struct global_symbol *symbol)
{
	if (symtab_ptr >= symtab_end)
		return ld_error("ld_add", "symbol table overflow");

	memcpy(symtab_ptr, symbol, sizeof(symbol[0]));
	symtab_ptr += 1;

	global_symbols = symtab;
	global_symbols_size += sizeof(symbol[0]);

	return 0;
}

int ld_find(const char *name, struct global_symbol **symbol)
{
	struct global_symbol *p;

	for (p = symtab; p < symtab_ptr; p++) {
		if (!strcmp(name, &p->name[0]))
			return (*symbol = p), 0;
	}
	return (*symbol = NULL), 1;
}

void ld_free(void)
{
	free(symtab);
	symtab = NULL;
	symtab_end = NULL;
	symtab_ptr = NULL;

	global_symbols = NULL;
	global_symbols_size = 0;
}

int ld_link(const char *name, unsigned char *obj)
{
	unsigned char *image = NULL;
	size_t symbols = (size_t)LE32(&obj[44]);

	size_t text_size  = (size_t)LE32(&obj[52 + (0 * 40) + 16]);
	size_t rdata_size = (size_t)LE32(&obj[52 + (1 * 40) + 16]);
	size_t data_size  = (size_t)LE32(&obj[52 + (2 * 40) + 16]);
	size_t bss_size   = (size_t)LE32(&obj[52 + (3 * 40) + 16]);

	unsigned char *text_sec  = obj + LE32(&obj[52 + (0 * 40) + 20]);
	unsigned char *rdata_sec = obj + LE32(&obj[52 + (1 * 40) + 20]);
	unsigned char *data_sec  = obj + LE32(&obj[52 + (2 * 40) + 20]);
	unsigned char *bss_sec   = NULL;

	unsigned char *text_rel  = obj + LE32(&obj[52 + (0 * 40) + 24]);
	unsigned char *rdata_rel = obj + LE32(&obj[52 + (1 * 40) + 24]);
	unsigned char *data_rel  = obj + LE32(&obj[52 + (2 * 40) + 24]);

	unsigned text_nrel  = (unsigned)LE16(&obj[52 + (0 * 40) + 32]);
	unsigned rdata_nrel = (unsigned)LE16(&obj[52 + (1 * 40) + 32]);
	unsigned data_nrel  = (unsigned)LE16(&obj[52 + (2 * 40) + 32]);

	unsigned char *symbol_table = obj + LE32(&obj[40]);
	char *string_table = (char *)symbol_table + (symbols * 18);

	/*
	 * Allocate contiguous memory for the executable image.
	 */
	{
		const size_t mask = 0xFFFFF000u;
		const size_t add  = 0x00000FFFu;
		size_t size = 0;

		size += (text_size  + add) & mask;
		size += (rdata_size + add) & mask;
		size += (data_size  + add) & mask;
		size += (bss_size   + add) & mask;

		if (!size)
			return ld_error(name, "empty object");

		image = aligned_alloc(4096, size);

		if (!image)
			return ld_error(name, "out of memory");

		memset(image, 0, size);
	}

	/*
	 * Copy section data and adjust pointers.
	 */
	{
		const size_t mask = 0xFFFFF000u;
		const size_t add  = 0x00000FFFu;
		unsigned char *p = image;

		memcpy(p, text_sec, text_size);
		text_sec = p;
		p += (text_size + add) & mask;

		memcpy(p, rdata_sec, rdata_size);
		rdata_sec = p;
		p += (rdata_size + add) & mask;

		memcpy(p, data_sec, data_size);
		data_sec = p;
		p += (data_size + add) & mask;

		bss_sec = p;
	}

	/*
	 * Add the base address to the symbol values.
	 */
	{
		unsigned char *st = symbol_table;
		size_t i;

		for (i = 0; i < symbols; i++) {
			unsigned long val = LE32(&st[8]);
			unsigned section_number = (unsigned)LE16(&st[12]);

			switch (section_number) {
			case 1:
				val += (unsigned long)((addr_t)text_sec);
				break;
			case 2:
				val += (unsigned long)((addr_t)rdata_sec);
				break;
			case 3:
				val += (unsigned long)((addr_t)data_sec);
				break;
			case 4:
				val += (unsigned long)((addr_t)bss_sec);
				break;
			default:
				break;
			}
			W_LE32(&st[8], val);
			st += 18;
		}
	}

	/*
	 * Resolve all external symbols.
	 */
	{
		unsigned char *st = symbol_table;
		size_t i, size;

		for (i = 0; i < symbols; i++) {
			unsigned section_number = (unsigned)LE16(&st[12]);

			if (section_number == 0) {
				struct global_symbol *sym;
				char sname[32];
				unsigned long val;
				const char *s;

				if (LE32(&st[0])) {
					s = (char *)st;
					size = 8;
				} else {
					s = string_table + LE32(&st[4]);
					size = sizeof(sname) - 1;
				}
				strncpy(&sname[0], s, size);
				sname[size] = '\0';

				if (ld_find(&sname[0], &sym) != 0)
					return ld_error("symbol", &sname[0]);

				val = (unsigned long)sym->value;
				W_LE32(&st[8], val);
			}
			st += 18;
		}
	}

	/*
	 * Add all new symbols to the global symbol table.
	 */
	{
		unsigned char *st = symbol_table;
		size_t i, size;

		for (i = 0; i < symbols; i++) {
			unsigned section_number = (unsigned)LE16(&st[12]);

			if (LE32(&st[8]) == 0)
				return ld_error("ld_link", "null symbol");

			if (section_number != 0 && st[16] == 0x02) {
				struct global_symbol sym;
				const char *s;
				int r;

				memset(&sym, 0, sizeof(sym));

				if (LE32(&st[0])) {
					s = (char *)st;
					size = 8;
				} else {
					s = string_table + LE32(&st[4]);
					size = sizeof(sym.name) - 1;
				}

				sym.value = (uint32_t)LE32(&st[8]);
				strncpy(&sym.name[0], s, size);

				if ((r = ld_add(&sym)) != 0)
					return r;
			}
			st += 18;
		}
	}

	/*
	 * Relocate .text section.
	 */
	if (text_nrel) {
		unsigned char *reloc = text_rel;
		unsigned char *symbol;
		unsigned i;

		for (i = 0; i < text_nrel; i++) {
			symbol = symbol_table + (LE32(&reloc[4]) * 18);

			ld_relocate(text_sec, reloc, symbol);
			reloc += 10;
		}
	}

	/*
	 * Relocate .rdata section.
	 */
	if (rdata_nrel) {
		unsigned char *reloc = rdata_rel;
		unsigned char *symbol;
		unsigned i;

		for (i = 0; i < rdata_nrel; i++) {
			symbol = symbol_table + (LE32(&reloc[4]) * 18);

			ld_relocate(rdata_sec, reloc, symbol);
			reloc += 10;
		}
	}

	/*
	 * Relocate .data section.
	 */
	if (data_nrel) {
		unsigned char *reloc = data_rel;
		unsigned char *symbol;
		unsigned i;

		for (i = 0; i < data_nrel; i++) {
			symbol = symbol_table + (LE32(&reloc[4]) * 18);

			ld_relocate(data_sec, reloc, symbol);
			reloc += 10;
		}
	}

	return 0;
}

int ld_validate(const char *name, unsigned char *obj, size_t size)
{
	if (size < 212)
		return ld_error(name, "unsupported format");
	if (LE32(&obj[16]) != size)
		return ld_error(name, "size error");

	{
		unsigned long crc_value = LE32(&obj[24]);
		int err;

		W_LE32(&obj[24], 0);
		err = (crc32c(obj, size) != crc_value);
		W_LE32(&obj[24], crc_value);

		if (err)
			return ld_error(name, "CRC-32C");
	}

	obj += 32;

	if (LE16(&obj[0]) != obj_magic)
		return ld_error(name, "magic number");
	if (LE16(&obj[2]) != 4)
		return ld_error(name, "number of sections");
	if (obj[16] || obj[17])
		return ld_error(name, "optional header");

	if (memcmp(&obj[20], ".text", 6))
		return ld_error(name, ".text section");
	if (memcmp(&obj[60], ".rdata", 7))
		return ld_error(name, ".rdata section");
	if (memcmp(&obj[100], ".data", 6))
		return ld_error(name, ".data section");
	if (memcmp(&obj[140], ".bss", 5))
		return ld_error(name, ".bss section");

	if ((LE32(&obj[20+36]) & 0xFF0FFFFF) != 0x60000020)
		return ld_error(name, ".text flags");
	if ((LE32(&obj[60+36]) & 0xFF0FFFFF) != 0x40000040)
		return ld_error(name, ".rdata flags");
	if ((LE32(&obj[100+36]) & 0xFF0FFFFF) != 0xC0000040)
		return ld_error(name, ".data flags");
	if ((LE32(&obj[140+36]) & 0xFF0FFFFF) != 0xC0000080)
		return ld_error(name, ".bss flags");

	return 0;
}
