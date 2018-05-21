/*
 * Copyright (c) 2018 Antti Tiihala
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
 * dy-link/dump.c
 *      Linker for Dancy operating system
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"

void dump_ext(const char *name, const unsigned char *buf)
{
	unsigned long symtab_offset = LE32(&buf[8]);
	unsigned long symtab_count = LE32(&buf[12]);
	int i;

	printf("undefined_syms: %s\n\n", name);

	for (i = 0; i < (int)symtab_count; i++) {
		const unsigned char *sym = &buf[(int)symtab_offset + i * 18];

		/*
		 * Dump only symbol table entries that are unresolved. It
		 * means that other object files must have them defined.
		 */
		if (LE32(&sym[8]) || LE16(&sym[12])) {
			i = i + (int)sym[17];
			continue;
		}
		printf("  [%08lX]    ", (unsigned long)i);
		printf("%08lX  ", LE32(&sym[8]));
		/*
		 * It is expected that only extern is found but all other
		 * types are also dumped (debug), i.e. type0, type1, etc.
		 */
		if (sym[16] == 0x02u)
			printf("%-8s", "extern");
		else
			printf("type%-3u ", (unsigned)sym[16]);

		if (LE16(&sym[14]) == 0x0000ul)
			printf("%-6s", "-");
		else if (LE16(&sym[14]) == 0x0020ul)
			printf("%-6s", "func");
		else
			printf("%-6s", "\?");

		printf("%-6s-   ", "-");
		if (LE32(&sym[0])) {
			printf("-> %.8s", &sym[0]);
		} else {
			const unsigned char *str;
			str = &buf[(int)(symtab_offset + symtab_count * 18u)];
			str = str + (int)LE32(&sym[4]);
			printf("-> %s", str);
		}
		printf("\n");
		i = i + (int)sym[17];
	}
	printf("\n");
}

void dump_obj(const char *name, const unsigned char *buf)
{
	unsigned long arch_type = LE16(&buf[0]);
	unsigned long section_count = LE16(&buf[2]);
	unsigned long symtab_offset = LE32(&buf[8]);
	unsigned long symtab_count = LE32(&buf[12]);
	const char *arch_name = (arch_type == 0x014Cu) ? "i386" : "x86-64";
	int i;

	printf("file_name:      %s\n", name);
	printf("magic_word:     %04lX     -> %s\n", arch_type, arch_name);
	printf("section_count:  %04lX\n", section_count);
	printf("symtab_offset:  %08lX\n", symtab_offset);
	printf("symtab_count:   %08lX\n\n", symtab_count);

	for (i = 0; i < (int)LE16(&buf[2]); i++) {
		const unsigned char *sect = &buf[20 + i * 40];
		unsigned long flags = LE32(&sect[36]);
		unsigned sect_alignment;

		printf("section_name:   %-8.8s -> #%i\n", &sect[0], i + 1);
		printf("section_flags:  %08lX ->", flags);

		/*
		 * Code, data, or bss. These are the main categories
		 * as far as sections are concerned.
		 */
		if (flags & 0x00000020ul)
			printf(" code");
		if (flags & 0x00000040ul)
			printf(" data");
		if (flags & 0x00000080ul)
			printf(" bss");
		/*
		 * Some sections can be ignored. This will simplify the
		 * linker, i.e. unknown data structures can be skipped.
		 */
		if (flags & 0x00000200ul)
			printf(" info");
		if (flags & 0x00000800ul)
			printf(" rem");
		if (flags & 0x00001000ul)
			printf(" comd");
		if (flags & 0x02000000ul)
			printf(" disc");
		/*
		 * "Shared, "executable", "read", and "write" flags. This
		 * information can be used for paging.
		 */
		if (flags & 0x10000000ul)
			printf(" share");
		if (flags & 0x20000000ul)
			printf(" exec");
		if (flags & 0x40000000ul)
			printf(" read");
		if (flags & 0x80000000ul)
			printf(" write");

		sect_alignment = (unsigned)((flags & 0x00F00000ul) >> 20);
		if (sect_alignment)
			printf(" align%u", (1u << (sect_alignment - 1)));
		printf("\n");

		printf("data_size:      %08lX\n", LE32(&sect[16]));
		if (LE32(&sect[20]))
			printf("data_offset:    %08lX\n", LE32(&sect[20]));
		if (LE16(&sect[32])) {
			unsigned long reloc_offset = LE32(&sect[24]);
			unsigned long reloc_count = LE16(&sect[32]);
			printf("reloc_offset:   %08lX\n", reloc_offset);
			printf("reloc_count:    %04lX\n", reloc_count);
		}
		printf("\n");
	}

	for (i = 0; i < (int)symtab_count; i++) {
		const unsigned char *sym = &buf[(int)symtab_offset + i * 18];
		int is_section = 0;

		/*
		 * Detect whether a symbol record refers to a section. There
		 * should be some additional information available in the
		 * next symbol table entry ("the extended symbol entry").
		 */
		if (!LE32(&sym[8]) && sym[16] == 0x03u && sym[17]) {
			if (LE16(&sym[12]) && LE16(&sym[12]) < 0x8000ul)
				is_section = 1;
		}
		printf("  [%08lX]", (unsigned long)i);
		if (sym[17])
			printf("+%-2X ", (unsigned)sym[17]);
		else
			printf("%-4s", " ");
		printf("%08lX  ", LE32(&sym[8]));

		if (sym[16] == 0x02u)
			printf("%-8s", "extern");
		else if (sym[16] == 0x03u)
			printf("%-8s", "static");
		else if (sym[16] == 0x06u)
			printf("%-8s", "label");
		else if (sym[16] == 0x67u)
			printf("%-8s", "file");
		else
			printf("type%-3u ", (unsigned)sym[16]);

		if (LE16(&sym[14]) == 0x0000ul)
			printf("%-6s", "-");
		else if (LE16(&sym[14]) == 0x0020ul)
			printf("%-6s", "func");
		else
			printf("%-6s", "\?");

		if (LE16(&sym[12]) == 0x0000ul)
			printf("%-6s", "-");
		else if (LE16(&sym[12]) == 0xFFFFul)
			printf("%-6s", "abs");
		else if (LE16(&sym[12]) == 0xFFFEul)
			printf("%-6s", "dbg");
		else if (LE16(&sym[12]) <= section_count)
			printf("#%-5lu", LE16(&sym[12]));
		else
			printf("%04lX  ", LE16(&sym[12]));
		/*
		 * Print value at offset 14 from the next symbol entry if
		 * this is the section symbol.
		 */
		if (is_section)
			printf("%02X  ", (unsigned)sym[18 + 14]);
		else
			printf("%-4s", "-");

		if (LE32(&sym[0])) {
			printf("-> %.8s", &sym[0]);
		} else {
			const unsigned char *str;
			str = &buf[(int)(symtab_offset + symtab_count * 18u)];
			str = str + (int)LE32(&sym[4]);
			printf("-> %s", str);
		}
		printf("\n");
		i = i + (int)sym[17];
	}
	printf("\n");
}
