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
 * init/ld.c
 *      Dynamic linker and loader
 */

#include <dancy.h>

static int ld_error(const char *name, const char *msg)
{
	b_log("Error: %s (%s)\n", name, msg);
	b_print("Error: %s (%s)\n", name, msg);
	return 1;
}

static int validate_reloc_type(unsigned reloc_type)
{
	/*
	 * The return value tells how many bytes must be accessible from
	 * the reloc offset in the relocatable section. Accessible does
	 * not necessarily mean that the relocation modifies all bytes.
	 */
	int bytes_to_access = 0;

#if defined (DANCY_32)
	if (reloc_type < 0x8000) {
		switch (reloc_type) {
		case 0x06: bytes_to_access = 0x04; break;
		case 0x07: bytes_to_access = 0x04; break;
		case 0x14: bytes_to_access = 0x04; break;
		default: break;
		}
	}
#elif defined (DANCY_64)
	if (reloc_type < 0x8000) {
		switch (reloc_type) {
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
#endif
	return bytes_to_access;
}

int ld_validate(const char *name, const unsigned char *obj, size_t size)
{
	unsigned long strtab_size = 0;
	int total_size = 0;

#if defined (DANCY_32)
	const unsigned obj_magic = 0x014C;
#elif defined (DANCY_64)
	const unsigned obj_magic = 0x8664;
#endif

	if (size < 180 || size > 16777216)
		return ld_error(name, "unsupported object format");
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

	/*
	 * Check that there is enough space for the symbol and string tables.
	 */
	if (LE32(&obj[12]) >= 4) {
		unsigned long offset = LE32(&obj[8]);
		unsigned long symbols = LE32(&obj[12]);
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
		if (err)
			return ld_error(name, "symbol table overflow");

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
			strtab_size = LE32(&obj[offset]);
		}

		if (strtab_size < 4)
			err = 1;
		if (offset > ULONG_MAX - strtab_size)
			err = 1;
		if (offset + strtab_size > (unsigned long)size)
			err = 1;
		if (!err && obj[offset + strtab_size - 1])
			err = 1;
		if (err)
			return ld_error(name, "string table error");

		/*
		 * Simple check for symbol table entries. All pointer values
		 * to the string table must reside within the string table.
		 */
		offset = LE32(&obj[8]);
		symbols = LE32(&obj[12]);
		if (symbols) {
			unsigned long i = 0;
			const unsigned char *sym;
			unsigned long section_number;

			while (!err && i < symbols) {
				sym = &obj[offset + i * 18];
				if (!LE32(&sym[0])) {
					if (LE32(&sym[4]) < 4)
						err = 1;
					if (LE32(&sym[4]) >= strtab_size)
						err = 1;
				}
				section_number = LE16(&sym[12]);
				if (section_number > 4) {
					err = 1;
				}

				if (!section_number) {
					if (LE32(&sym[8]))
						err = 1;
					if ((unsigned)sym[16] != 2)
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
				 * Do not allow extra records.
				 */
				if ((unsigned)sym[17] != 0x00)
						err = 1;
				i += 1;
			}
			if (i != symbols)
				err = 1;
		}
		if (err) {
			return ld_error(name, "symbol table error");
		}
	} else {
		return ld_error(name, "no symbols");
	}

	/*
	 * Check the section table. The values for virtual size and address
	 * must be zero for supported object files. Section data and reloc
	 * records must reside within the real file data. A so-called .bss
	 * section must not have data in the file.
	 */
	{
		unsigned nr_sections = (unsigned)LE16(&obj[2]);
		unsigned long nr_symbols = LE32(&obj[12]);
		int err = 0;
		int i = 0;
		while (!err && (unsigned)i < nr_sections) {
			const unsigned char *sect = &obj[20 + i++ * 40];
			unsigned long data_size = LE32(&sect[16]);
			unsigned long data_offset = LE32(&sect[20]);
			unsigned long relo_offset = LE32(&sect[24]);
			unsigned long relo_size = LE16(&sect[32]) * 10;
			unsigned long flags = LE32(&sect[36]);
			const unsigned char *relo_ptr;

			if (LE32(&sect[8]) || LE32(&sect[12]))
				err = 1;

			/*
			 * No line numbers.
			 */
			if (LE32(&sect[28]) || LE16(&sect[34]))
				err = 1;

			/*
			 * The section cannot overlap with the header unless
			 * it is a bss section ("uninitialized data").
			 */
			if (flags & 0x00000080ul) {
				if (data_offset)
					err = 1;
				if (relo_offset || relo_size)
					return ld_error(name, "bss relocs");
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
			relo_ptr = &obj[relo_offset];
			while (!err && relo_size) {
				unsigned long r_off = LE32(relo_ptr+0);
				unsigned long r_idx = LE32(relo_ptr+4);
				unsigned r_typ = (unsigned)LE16(relo_ptr+8);
				int bytes;

				bytes = validate_reloc_type(r_typ);
				if (!bytes)
					return ld_error(name, "reloc type");
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
			 * Check the total size of all sections.
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
			return ld_error(name, "section table error");
		}
	}

	return 0;
}
