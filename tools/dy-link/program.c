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

#include "program.h"

static int default_obj(struct options *opt, unsigned magic)
{
	size_t size = 20u;
	unsigned char *m;

	if (opt->nr_mfiles) {
		const char *e = "Error: format %s with input files\n";
		fprintf(stderr, e, opt->arg_f);
		return 1;
	}
	/*
	 * This memory allocation is handled like mfiles in general.
	 */
	if ((m = calloc(size, sizeof(unsigned char))) != NULL) {
		W_LE16(&m[0], magic);
		opt->mfiles[0].data = m;
		opt->mfiles[0].size = (int)size;
	} else {
		return fputs("Error: not enough memory\n", stderr), 1;
	}
	return opt->nr_mfiles++, 0;
}

int program(struct options *opt)
{
	int i;

	if (opt->arg_f && strcmp(opt->arg_f, "obj")) {
		if (!strcmp(opt->arg_f, "at")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "init")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "32")) {
			if (default_obj(opt, 0x014Cu))
				return 1;
		} else if (!strcmp(opt->arg_f, "64")) {
			if (default_obj(opt, 0x8664u))
				return 1;
		} else {
			fputs("Error: unknown output format\n", stderr);
			return 1;
		}
	} else {
		opt->arg_f = "obj";
	}

	if (!opt->nr_mfiles)
		return fputs("Warning: no input\n", stderr), 1;

	for (i = 0; i < opt->nr_mfiles; i++) {
		struct mfile *obj = &opt->mfiles[i];
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
	}
	/*
	 * Handle grouped sections first and then check
	 * the total size of all relevant sections.
	 */
	if (section_group(opt))
		return 1;
	if (section_check_sizes(opt) == INT_MAX)
		return 1;
	if (symbol_check_sizes(opt) == INT_MAX)
		return 1;
	return link_main(opt);
}
