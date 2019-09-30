/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
	size_t size = 20;
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

static void native_obj(unsigned char *data, int size)
{
	const int at_size = AT_HEADER_SIZE + (20 + 4 * 40);
	const int ef_size = EF_HEADER_SIZE + (20 + 4 * 40);
	const int in_size = IN_HEADER_SIZE + (20 + 4 * 40);
	unsigned char header[20 + 4 * 40];

	if (size < 32)
		return;
	if (LE32(&data[0]) != 0x00000000 && LE32(&data[0]) != 0x0D54418D)
		return;
	/*
	 * Normal executables.
	 */
	if (LE32(&data[8]) == 0x450C430C && size >= at_size) {
		if (LE16(&data[AT_HEADER_SIZE + 2]) != 4)
			return;
		if (LE32(&data[AT_HEADER_SIZE + 16]) != 0)
			return;
		memcpy(&header[0], &data[AT_HEADER_SIZE], sizeof(header));
		memset(&data[0], 0, (size_t)at_size);
		memcpy(&data[0], &header[0], sizeof(header));
		return;
	}
	/*
	 * Init executables (with header).
	 */
	if (LE32(&data[8]) == 0x4E0C490C && size >= in_size) {
		if (LE16(&data[IN_HEADER_SIZE + 2]) != 4)
			return;
		if (LE32(&data[IN_HEADER_SIZE + 16]) != 0)
			return;
		memcpy(&header[0], &data[IN_HEADER_SIZE], sizeof(header));
		memset(&data[0], 0, (size_t)in_size);
		memcpy(&data[0], &header[0], sizeof(header));
		return;
	}
	/*
	 * Init executables (without header).
	 */
	if (size >= in_size && LE16(&data[IN_HEADER_SIZE + 2]) == 4) {
		int i;
		for (i = 4; i < IN_HEADER_SIZE; i++) {
			if (data[i])
				return;
		}
		if (LE32(&data[IN_HEADER_SIZE + 16]) != 0)
			return;
		memcpy(&header[0], &data[IN_HEADER_SIZE], sizeof(header));
		memset(&data[0], 0, (size_t)in_size);
		memcpy(&data[0], &header[0], sizeof(header));
		return;
	}
	/*
	 * Uefi executables.
	 */
	if (size >= ef_size && LE16(&data[EF_HEADER_SIZE + 2]) == 4) {
		int i;
		for (i = 4; i < EF_HEADER_SIZE; i++) {
			if (data[i])
				return;
		}
		if (LE32(&data[EF_HEADER_SIZE + 16]) != 0)
			return;
		memcpy(&header[0], &data[EF_HEADER_SIZE], sizeof(header));
		memset(&data[0], 0, (size_t)ef_size);
		memcpy(&data[0], &header[0], sizeof(header));
		return;
	}
}

int program(struct options *opt)
{
	int i;

	if (opt->arg_f && strcmp(opt->arg_f, "obj")) {
		if (!strcmp(opt->arg_f, "at")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "init")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "uefi")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "32")) {
			if (default_obj(opt, 0x014C))
				return 1;
		} else if (!strcmp(opt->arg_f, "64")) {
			if (default_obj(opt, 0x8664))
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
		 * Translate native executables to normal objects.
		 */
		native_obj(obj->data, obj->size);
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
	 * Set default alignment bits for sections.
	 */
	opt->alignbits_t = 0x00500000;
	opt->alignbits_r = 0x00300000;
	opt->alignbits_d = 0x00300000;
	opt->alignbits_b = 0x00500000;
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
