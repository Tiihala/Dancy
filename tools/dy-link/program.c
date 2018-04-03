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
 * dy-link/program.c
 *      Linker for Dancy operating system
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

static int validate_obj(const char *name, const unsigned char *buf, int size)
{
	static unsigned type;

	if (size < 20) {
		fprintf(stderr, "%s: unsupported file format\n", name);
		return 1;
	}
	if (type && type != (unsigned)LE16(&buf[0])) {
		fputs("Error: different object formats\n", stderr);
		return 1;
	} else {
		type = (unsigned)LE16(&buf[0]);
	}

	/*
	 * Detect the type of instructions. Types 80386 and AMD64
	 * are currently supported by this linker implementation.
	 */
	if (type != 0x014Cu && type != 0x8664u) {
		const char *fmt = "%s: unknown magic word (0x%04X)\n";
		fprintf(stderr, fmt, name, type);
		return 1;
	}

	/*
	 * There must not be an optional header or flags.
	 */
	if (buf[16] || buf[17] || buf[18] || buf[19]) {
		fprintf(stderr, "%s: unsupported header format\n", name);
		return 1;
	}

	/*
	 * Check that there is enough space for the section table. The
	 * limit of section table entries is artificial.
	 */
	if (LE16(&buf[2]) * 40ul + 20ul > (unsigned long)size) {
		fprintf(stderr, "%s: section table error\n", name);
		return 1;
	}
	if (LE16(&buf[2]) > 16384ul) {
		fprintf(stderr, "%s: too many sections\n", name);
		return 1;
	}

	/*
	 * Check that there is enough space for the symbol and string tables.
	 */
	if (LE32(&buf[12])) {
		unsigned long offset = LE32(&buf[8]);
		unsigned long symbols = LE32(&buf[12]);
		unsigned long strings = 0ul;
		int err = 1;

		/*
		 * The limit of symbol table entries is artificial but
		 * it prevents overflow when doing the multiplication.
		 */
		if (symbols < 0x00FFFFFFul)
			err = 0, symbols = symbols * 18ul;
		if (offset > ULONG_MAX - symbols)
			err = 1;
		if (offset + symbols > (unsigned long)size)
			err = 1;
		if (err) {
			fprintf(stderr, "%s: symbol table overflow\n", name);
			return 1;
		}

		/*
		 * The size of the string table is 4 bytes if there are
		 * no actual string table entries.
		 */
		offset = offset + symbols;
		if (offset > ULONG_MAX - 4ul)
			err = 1;
		else
			strings = LE32(&buf[(int)offset]);

		if (strings < 4ul)
			err = 1;
		if (offset > ULONG_MAX - strings)
			err = 1;
		if (offset + strings > (unsigned long)size)
			err = 1;
		if (err) {
			fprintf(stderr, "%s: string table error\n", name);
			return 1;
		}

		/*
		 * Simple check for symbol table entries. All pointer values
		 * to the string table must reside within the string table.
		 */
		offset = LE32(&buf[8]);
		symbols = LE32(&buf[12]);
		if (symbols) {
			unsigned long i = 0ul;
			const unsigned char *sym;
			unsigned long section_number;

			while (!err && i < symbols) {
				sym = &buf[(int)(offset + i * 18ul)];
				if (!LE32(&sym[0])) {
					if (LE32(&sym[4]) < 4ul)
						err = 1;
					if (LE32(&sym[4]) >= strings)
						err = 1;
				}
				section_number = LE16(&sym[12]);
				if (section_number < 0x8000ul) {
					if (section_number > LE16(&buf[2]))
						err = 1;
				}
				i = i + (unsigned long)sym[17] + 1ul;
			}
			if (i != symbols)
				err = 1;
		}
		if (err) {
			fprintf(stderr, "%s: symbol table error\n", name);
			return 1;
		}
	}

	/*
	 * Check the section table. The values for virtual size and address
	 * must be zero for supported object files. Section data and reloc
	 * records must reside within the real file data. A so-called .bss
	 * section must not have data in the file.
	 */
	if (LE16(&buf[2])) {
		unsigned nr_sections = (unsigned)LE16(&buf[2]);
		int err = 0;
		int i = 0;
		while (!err && (unsigned)i < nr_sections) {
			const unsigned char *sect = &buf[20 + i++ * 40];
			unsigned long data_size = LE32(&sect[16]);
			unsigned long data_offset = LE32(&sect[20]);
			unsigned long relo_offset = LE32(&sect[24]);
			unsigned long relo_size = LE16(&sect[32]) * 10ul;
			unsigned long flags = LE32(&sect[36]);

			if (LE32(&sect[8]) || LE32(&sect[12]))
				err = 1;
			if (data_offset && flags & 0x00000080ul)
				err = 1;
			/*
			 * Do not support any "special relocations" or data
			 * alignment requirements more strict than 4096.
			 */
			if (flags & 0x01000000ul)
				err = 1;
			if ((flags & 0x00F00000ul) > 0x00D00000ul)
				err = 1;

			if (data_size > ULONG_MAX - data_offset)
				err = 1;
			if (data_size + data_offset > (unsigned long)size)
				err = 1;

			if (relo_size > ULONG_MAX - relo_offset)
				err = 1;
			if (relo_size + relo_offset > (unsigned long)size)
				err = 1;
		}
		if (err) {
			fprintf(stderr, "%s: section table error\n", name);
			return 1;
		}
	}
	return 0;
}

static void dump_obj(const char *name, const unsigned char *buf)
{
	unsigned long section_count = LE16(&buf[2]);
	unsigned long symtab_offset = LE32(&buf[8]);
	unsigned long symtab_count = LE32(&buf[12]);
	int i;

	if (name && buf) {
		unsigned type = (unsigned)LE16(&buf[0]);
		const char *type_name = (type == 0x014Cu) ? "i386" : "x86-64";
		printf("file_name:      %s\n", name);
		printf("magic_word:     %04X     -> %s\n", type, type_name);
	} else {
		return;
	}
	printf("section_count:  %04lX\n", section_count);
	printf("symtab_offset:  %08lX\n", symtab_offset);
	printf("symtab_count:   %08lX\n\n", symtab_count);

	for (i = 0; i < (int)LE16(&buf[2]); i++) {
		const unsigned char *sect = &buf[20 + i * 40];
		unsigned long flags = LE32(&sect[36]);
		unsigned sect_alignment;

		printf("section_name:   %-8.8s -> #%i\n", &sect[0], i + 1);
		printf("section_flags:  %08lX ->", flags);

		if (flags & 0x00000020ul)
			printf(" code");
		if (flags & 0x00000040ul)
			printf(" data");
		if (flags & 0x00000080ul)
			printf(" bss");
		if (flags & 0x00000200ul)
			printf(" info");
		if (flags & 0x00000800ul)
			printf(" rem");
		if (flags & 0x00001000ul)
			printf(" comd");
		if (flags & 0x02000000ul)
			printf(" disc");
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

		if (LE32(&sym[0])) {
			printf("-> %-8.8s ", &sym[0]);
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

int program(struct options *opt)
{
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		struct mfile *obj = &opt->mfiles[i];
		if (validate_obj(opt->operands[i], obj->data, obj->size))
			return 1;
		if (opt->dump)
			dump_obj(opt->operands[i], obj->data);
	}
	return 0;
}
