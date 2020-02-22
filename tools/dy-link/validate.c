/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * dy-link/validate.c
 *      Linker for Dancy operating system
 */

#include "program.h"

static int validate_reloc_type(unsigned arch_type, unsigned reloc_type)
{
	/*
	 * The return value tells how many bytes must be accessible from
	 * the reloc offset in the relocatable section. Accessible does
	 * not necessarily mean that the relocation modifies all bytes.
	 */
	int bytes_to_access = 0;

	if (arch_type == 0x014C && reloc_type < 0x8000) {
		switch ((int)reloc_type) {
		case 0x06: bytes_to_access = 0x04; break;
		case 0x07: bytes_to_access = 0x04; break;
		case 0x14: bytes_to_access = 0x04; break;
		default: break;
		}
	}
	if (arch_type == 0x8664 && reloc_type < 0x8000) {
		switch ((int)reloc_type) {
		case 0x01: bytes_to_access = 0x08; break;
		case 0x02: bytes_to_access = 0x04; break;
		case 0x03: bytes_to_access = 0x04; break;
		case 0x04: bytes_to_access = 0x04; break;
		case 0x05: bytes_to_access = 0x05; break;
		case 0x06: bytes_to_access = 0x06; break;
		case 0x07: bytes_to_access = 0x07; break;
		case 0x08: bytes_to_access = 0x08; break;
		case 0x09: bytes_to_access = 0x09; break;
		default: break;
		}
	}
	if (!bytes_to_access)
		fprintf(stderr, "Unknown reloc type %04X\n", reloc_type);
	return bytes_to_access;
}

static int validate_name(const char *name, unsigned long flags)
{
	int err = 0;

	/*
	 * Check the (de facto) standard section names.
	 */
	if (!strncmp(name, ".text", 5)) {
		if ((flags & 0xF0ul) != 0x20ul) {
			fputs("Error: .text flags\n", stderr);
			err = 1;
		}
	}
	if (!strncmp(name, ".rdata", 6)) {
		if ((flags & 0xF0ul) != 0x40ul) {
			fputs("Error: .rdata flags\n", stderr);
			err = 1;
		}
	}
	if (!strncmp(name, ".data", 5)) {
		if ((flags & 0xF0ul) != 0x40ul) {
			fputs("Error: .data flags\n", stderr);
			err = 1;
		}
	}
	if (!strncmp(name, ".bss", 4)) {
		if ((flags & 0xF0ul) != 0x80ul) {
			fputs("Error: .bss flags\n", stderr);
			err = 1;
		}
	}
	return err;
}

int validate_obj(const char *name, const unsigned char *buf, int size)
{
	static unsigned type;
	unsigned long strtab_size = 0;
	const unsigned char *strtab_off = NULL;
	int total_size = 0;

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
	if (type != 0x014C && type != 0x8664) {
		const char *fmt = "%s: unknown magic word (0x%04X)\n";
		fprintf(stderr, fmt, name, type);
		return 1;
	}

	/*
	 * There must not be an optional header.
	 */
	if (buf[16] || buf[17]) {
		fprintf(stderr, "%s: unsupported header format\n", name);
		return 1;
	}

	/*
	 * Check that there is enough space for the section table. The
	 * limit of section table entries is artificial.
	 */
	if (LE16(&buf[2]) * 40ul + 20 > (unsigned long)size) {
		fprintf(stderr, "%s: section table error\n", name);
		return 1;
	}
	if (LE16(&buf[2]) > 16384) {
		fprintf(stderr, "%s: too many sections\n", name);
		return 1;
	}

	/*
	 * Check that there is enough space for the symbol and string tables.
	 */
	if (LE32(&buf[12])) {
		unsigned long offset = LE32(&buf[8]);
		unsigned long symbols = LE32(&buf[12]);
		int err = 1;

		/*
		 * The limit of symbol table entries is artificial but
		 * it prevents overflow when doing the multiplication.
		 */
		if (symbols < 0x00FFFFFF)
			err = 0, symbols = symbols * 18;
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
		 * no actual string table entries. The last byte of the
		 * table must be zero so that strlen-like functions are
		 * safe even if the data itself were not valid.
		 */
		offset = offset + symbols;
		if (offset > ULONG_MAX - 4ul) {
			err = 1;
		} else {
			strtab_off = &buf[offset];
			strtab_size = LE32(&buf[offset]);
		}

		if (strtab_size < 4)
			err = 1;
		if (offset > ULONG_MAX - strtab_size)
			err = 1;
		if (offset + strtab_size > (unsigned long)size)
			err = 1;
		if (!err && buf[offset + strtab_size - 1])
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
			unsigned long i = 0;
			const unsigned char *sym;
			unsigned long section_number;

			while (!err && i < symbols) {
				sym = &buf[offset + i * 18];
				if (!LE32(&sym[0])) {
					if (LE32(&sym[4]) < 4)
						err = 1;
					if (LE32(&sym[4]) >= strtab_size)
						err = 1;
				}
				section_number = LE16(&sym[12]);
				if (section_number < 0xFFFE) {
					if (section_number > LE16(&buf[2]))
						err = 1;
				}
				if (!section_number && !LE32(&sym[8])) {
					if ((unsigned)sym[16] != 2)
						if ((unsigned)sym[16] != 105)
							err = 1;
				}
				/*
				 * Check that all first 4 bytes are zero
				 * if the first one is. Check that there
				 * are no hidden information after the
				 * first zero if it is a valid short name.
				 */
				if (!sym[0]) {
					if (LE32(&sym[0]))
						err = 1;
				} else {
					int i7 = 0;
					while (i7 < 8) {
						if (!sym[i7++])
							break;
					}
					while (i7 < 8) {
						if (sym[i7++])
							err = 1;
					}
				}
				/*
				 * Allow more than one extra record for long
				 * file names only. Other structures should
				 * not need more than one. Change if needed.
				 */
				if ((unsigned)sym[17] > 0x01) {
					if ((unsigned)sym[16] != 0x67)
						err = 1;
				}
				i = i + (unsigned long)sym[17] + 1;
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
		unsigned long nr_symbols = LE32(&buf[12]);
		int err = 0;
		int i = 0;
		while (!err && (unsigned)i < nr_sections) {
			const unsigned char *sect = &buf[20 + i++ * 40];
			unsigned long data_size = LE32(&sect[16]);
			unsigned long data_offset = LE32(&sect[20]);
			unsigned long relo_offset = LE32(&sect[24]);
			unsigned long relo_size = LE16(&sect[32]) * 10;
			unsigned long flags = LE32(&sect[36]);
			const unsigned char *relo_ptr;

			/*
			 * If the section name starts with a '/', then use the
			 * following number as an offset to the string table.
			 * The biggest number is 9999999 and unsigned long can
			 * hold it without overflows. Check that there are no
			 * trailing zeros (no "octal numbers" supported).
			 */
			if ((unsigned)sect[0] == 0x2F) {
				int j;
				unsigned long offset = 0;

				if ((unsigned)sect[1] < 0x31)
					err = 1;
				for (j = 1; j < 8 && sect[j]; j++) {
					unsigned long a;
					if ((unsigned)sect[j] < 0x30)
						err = 1;
					if ((unsigned)sect[j] > 0x39)
						err = 1;
					a = (unsigned long)sect[j] - 0x30;
					offset *= 10;
					offset += a;
				}
				if (offset < strtab_size) {
					const void *t1 = strtab_off + offset;
					err |= validate_name(t1, flags);
				} else {
					err = 1;
				}
			} else {
				const void *t1 = &sect[0];
				err |= validate_name(t1, flags);
			}

			if (LE32(&sect[12]))
				err = 1;
			/*
			 * The section cannot overlap with the header unless
			 * it is a bss section ("uninitialized data"). At this
			 * point, relocs for bss sections are not supported.
			 */
			if (flags & 0x00000080ul) {
				if (data_offset)
					err = 1;
				if (relo_offset || relo_size) {
					fputs("Error: bss relocs\n", stderr);
					err = 1;
				}
			} else {
				if (data_size && data_offset < 0x14)
					err = 1;
			}
			/*
			 * Do not support any "special relocations" or data
			 * alignment requirements more strict than 4096. If
			 * the section is marked as "executable" it must be
			 * a code section.
			 */
			if (flags & 0x01000000ul)
				err = 1;
			if ((flags & 0x00F00000ul) > 0x00D00000)
				err = 1;
			if (flags & 0x20000000ul) {
				if (!(flags & 0x20ul))
					err = 1;
			}
			/*
			 * Sections can be code, data or bss but not any
			 * combination of these categories. It could make
			 * sense to have text and data combined. At this
			 * point this linker implementation does not have
			 * support for this. If there were many files not
			 * meeting the criteria, this should be changed.
			 */
			if (flags & 0x20ul) {
				if ((flags & 0x40ul) || (flags & 0x80ul))
					err = 1;
			} else if (flags & 0x40ul) {
				if ((flags & 0x20ul) || (flags & 0x80ul))
					err = 1;
			} else if (flags & 0x80ul) {
				if ((flags & 0x20ul) || (flags & 0x40ul))
					err = 1;
			}

			if (data_size > ULONG_MAX - data_offset)
				err = 1;
			if (data_size + data_offset > (unsigned long)size)
				if (!(flags & 0x80ul))
					err = 1;

			if (relo_size > ULONG_MAX - relo_offset)
				err = 1;
			if (relo_size + relo_offset > (unsigned long)size)
				err = 1;
			/*
			 * Check the relocation entries if those are available
			 * and make sure reloc offsets, symbol table indices,
			 * and relocation types are safe to use.
			 */
			if ((relo_size % 10ul) != 0)
				err = 1;
			relo_ptr = &buf[relo_offset];
			while (!err && relo_size) {
				unsigned long r_off = LE32(relo_ptr+0);
				unsigned long r_idx = LE32(relo_ptr+4);
				unsigned r_typ = (unsigned)LE16(relo_ptr+8);
				int bytes;

				bytes = validate_reloc_type(type, r_typ);
				if (!bytes)
					err = 1;
				if (r_off & 0x80000000ul)
					err = 1;
				if (r_off + (unsigned long)bytes > data_size)
					err = 1;
				if (r_idx >= nr_symbols)
					err = 1;

				relo_ptr += 10;
				relo_size -= 10;
			}
			/*
			 * Check the total size of all sections and add 40
			 * bytes (the size of a section entry) to each one
			 * as an extra size. The reason for this check is
			 * to make sure that the size of all sections would
			 * not be more than INT_MAX.
			 */
			if (data_size > ULONG_MAX - 40ul)
				err = 1;
			if (!err) {
				unsigned long add = data_size + 40;
				if (add < (unsigned long)INT_MAX) {
					if (total_size < INT_MAX - (int)add)
						total_size += (int)add;
					else
						err = 1;
				} else {
					err = 1;
				}
			}
		}
		if (err) {
			fprintf(stderr, "%s: section table error\n", name);
			return 1;
		}
	}
	return 0;
}
