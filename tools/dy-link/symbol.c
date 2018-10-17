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
 * dy-link/symbol.c
 *      Linker for Dancy operating system
 */

#include "program.h"

int symbol_check_sizes(struct options *opt)
{
	int s1 = symbol_sizeof_string(opt);
	int s2 = symbol_sizeof_table(opt);

	if (s1 < INT_MAX - s2)
		return s1 + s2;
	return fputs("Error: total symbol table size\n", stderr), INT_MAX;
}

int symbol_copy_table(struct options *opt, unsigned char *out)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned char *sym = buf + LE32(&buf[8]);
		int syms = (int)LE32(&buf[12]);
		int j;

		for (j = 0; j < syms; j++) {
			unsigned long addr = 0ul;
			unsigned long sec = LE16(&sym[12]);
			int skip = 0;
			int add = 18;

			if (sec > 0x0000ul && sec < 0xFFFEul) {
				int s = (int)sec;
				s = section_reloc(opt, i, s, &addr);
				skip = s ? 0 : 1;
				sec = s;
			}

			if (LE32(&sym[0])) {
				memcpy(&out[0], &sym[0], 8);
			} else {
				W_LE16(&out[4], i);
				W_LE16(&out[6], j);
			}

			/*
			 * Copy the main entries and translate the values
			 * and section numbers.
			 */
			if (!skip) {
				unsigned long s8 = LE32(&sym[8]) + addr;
				unsigned long s12 = sec;
				unsigned long s14 = LE16(&sym[14]);
				unsigned char s16 = sym[16];

				W_LE32(&out[8], s8);
				W_LE16(&out[12], s12);
				W_LE16(&out[14], s14);
				out[16] = s16;
			} else {
				memcpy(&out[0], "___EMPTY", 8);
				W_LE32(&out[8], 0xFFFFFFFFul);
				out[16] = 2;
			}

			/*
			 * Copy the extra entries as they are. Those will be
			 * eventually deleted from the final object.
			 */
			if (sym[17]) {
				size_t n = (size_t)((int)sym[17] * 18);
				memcpy(out + 18, sym + 18, n);
				out[17] = sym[17];
				j += (int)sym[17];
				add += (int)n;
			}
			out += add, sym += add;
			total_size += add;
		}
	}
	return total_size;
}

static int symbol_delete(unsigned char *obj, int sym)
{
	int symtab = (int)LE32(&obj[8]);
	int syms = (int)LE32(&obj[12]);
	int i;

	if (sym >= syms)
		return 1;
	/*
	 * Check that relocation entries do not point
	 * to the symbol that will be deleted.
	 */
	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			if (sym == (int)LE32(&r_off[4]))
				return 2;
			r_off += 10;
		}
	}

	/*
	 * Delete the symbol from the table.
	 */
	{
		unsigned char *dst = obj + symtab + (sym * 18) + 0;
		unsigned char *src = obj + symtab + (sym * 18) + 18;
		size_t size = (size_t)((syms - sym - 1) * 18);

		if (size)
			memmove(dst, src, size);
		dst = obj + symtab + (syms * 18) - 18;
		memset(dst, 0, 18u);
	}

	/*
	 * Fix relocation entries.
	 */
	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			int s = (int)LE32(&r_off[4]);
			if (sym < s--)
				W_LE32(&r_off[4], s);
			r_off += 10;
		}
	}

	syms -= 1;
	W_LE32(&obj[12], syms);
	return 0;
}

int symbol_process(struct options *opt, unsigned char *obj)
{
	int symtab = (int)LE32(&obj[8]);
	int syms = (int)LE32(&obj[12]);
	int i;

	for (i = 0; i < syms; i++) {

	}
	return 0;
}

int symbol_sizeof_string(struct options *opt)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long off = LE32(&buf[8]);
		unsigned long syms = LE32(&buf[12]);
		unsigned long add;

		if (!off || !syms)
			continue;
		buf += off;
		buf += syms * 18ul;

		add = LE32(&buf[0]);
		if (add >= 4ul && add < (unsigned long)INT_MAX) {
			if (total_size < INT_MAX - (int)add) {
				total_size += (int)(add - 4ul);
				continue;
			}
		}
		return INT_MAX;
	}
	return (total_size) ? total_size + 4 : 0;
}

int symbol_sizeof_table(struct options *opt)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long add = LE32(&buf[12]) * 18ul;

		if (total_size < INT_MAX - (int)add)
			total_size += add;
		else
			return INT_MAX;
	}
	return (total_size / 18 <= 32767) ? total_size : INT_MAX;
}
