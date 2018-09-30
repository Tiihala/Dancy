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
