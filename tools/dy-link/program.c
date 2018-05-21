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

int program(struct options *opt)
{
	unsigned long nr_symbols = (unsigned long)opt->nr_mfiles;
	size_t siz_symbols;
	void *buf_symbols;
	int i;

	if (!nr_symbols)
		return fputs("Warning: no input\n", stderr), 1;

	for (i = 0; i < opt->nr_mfiles; i++) {
		struct mfile *obj = &opt->mfiles[i];
		unsigned long sym;
		/*
		 * Validating the input files is very important. Other code
		 * procedures assume that the data buffers are safe to use.
		 */
		if (validate_obj(opt->operands[i], obj->data, obj->size))
			return 1;
		if (opt->dump)
			dump_obj(opt->operands[i], obj->data);
		if (opt->dump_ext)
			dump_ext(opt->operands[i], obj->data);
		/*
		 * Find out how much memory is needed for the global symbol
		 * table. It is expected that the INT_MAX limit will not be
		 * a problem in practice (with 32-bit int or bigger). These
		 * checks are only a safety measure and for handling legacy
		 * platforms on which INT_MAX is the standard's minimum.
		 */
		sym = LE32(obj->data + 12);
		if (nr_symbols < ULONG_MAX - sym) {
			nr_symbols += sym;
			sym = (unsigned long)sizeof(struct symbol);
			if (nr_symbols < (unsigned long)INT_MAX / sym)
				continue;
		}
		return fputs("Error: too many symbols\n", stderr), 1;
	}

	siz_symbols = (size_t)nr_symbols * sizeof(struct symbol);
	buf_symbols = malloc(siz_symbols);
	if (!buf_symbols)
		return fputs("Error: not enough memory\n", stderr), 1;
	if (opt->verbose) {
		int size = (int)siz_symbols;
		printf("Allocated global symbol table, %i bytes\n", size);
	}

	memset(buf_symbols, 0, siz_symbols);
	symbol_init(buf_symbols);
	siz_symbols = (size_t)opt->nr_mfiles * sizeof(struct symbol);

	for (i = 0; i < opt->nr_mfiles; i++) {
		struct mfile *obj = &opt->mfiles[i];
		unsigned long sym = LE32(obj->data + 12);
		void *ptr;
		int j;

		ptr = (unsigned char *)buf_symbols + (int)siz_symbols;
		siz_symbols += sym * (unsigned long)(sizeof(struct symbol));

		((struct symbol *)buf_symbols)[i].next = ptr;
		((struct symbol *)buf_symbols)[i].data = obj->data;
		/*
		 * Build the global symbol table. The memory footprint is
		 * not optimized because relocation indices are used for
		 * finding the correct structures from the global table.
		 */
		for (j = 0; j < (int)sym; j++)
			symbol_add(i, j);
	}
	if (opt->verbose) {
		int size = (int)siz_symbols;
		printf("Occupied global symbol table, %i bytes\n", size);
	}

	free(buf_symbols);
	return 0;
}
