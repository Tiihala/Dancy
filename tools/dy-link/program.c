/*
 * Copyright (c) 2018, 2019, 2020, 2022 Antti Tiihala
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

static void native_to_obj(unsigned char *data, int size)
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
	 * Kernel executables.
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

static int do_link_main(struct options *opt)
{
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

static int do_lib_mode(struct options *opt)
{
	int nr_ofiles = opt->nr_ofiles;
	size_t size;
	int i, r = 0;

	size = (size_t)nr_ofiles * sizeof(*opt->lib_ofiles);

	if ((opt->lib_ofiles = malloc(size)) == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}

	memset(opt->lib_ofiles, 0, size);

	opt->arg_f = "obj";
	opt->nr_ofiles = 1;
	opt->lib_mode = 1;
	opt->nr_lib_ofiles = 0;

	for (i = 0; !r && i < nr_ofiles; i++) {
		r = do_link_main(opt);
		opt->ofiles += 1;
	}

	if (!r)
		r = lib_main(opt);

	for (i = 0; i < opt->nr_lib_ofiles; i++) {
		free(opt->lib_ofiles[i].data);
		opt->lib_ofiles[i].name = NULL;
		opt->lib_ofiles[i].data = NULL;
		opt->lib_ofiles[i].size = 0;
		opt->lib_ofiles[i].type = 0;
	}

	free(opt->lib_ofiles), opt->lib_ofiles = NULL;
	opt->nr_lib_ofiles = 0;

	return r;
}

int program(struct options *opt)
{
	int i;

	if (opt->arg_f) {
		if (!strcmp(opt->arg_f, "at")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "default")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "init")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "lib")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "obj")) {
			; /* Accept */
		} else if (!strcmp(opt->arg_f, "uefi")) {
			; /* Accept */
		} else {
			fputs("Error: unknown output format\n", stderr);
			return 1;
		}
	} else {
		opt->arg_f = "default";
	}

	if (!opt->nr_ofiles)
		return fputs("Warning: no input\n", stderr), 1;

	for (i = 0; i < opt->nr_ofiles; i++) {
		struct file *obj = &opt->ofiles[i];

		/*
		 * Translate native executables to normal objects.
		 */
		native_to_obj(obj->data, obj->size);

		/*
		 * Validating the input files is very important. Other code
		 * procedures assume that the data buffers are safe to use.
		 */
		if (validate_obj(obj->name, obj->data, obj->size))
			return 1;

		if (opt->dump)
			dump_obj(obj->name, obj->data);
		if (opt->dump_ext)
			dump_ext(obj->name, obj->data);
	}

	if (!strcmp(opt->arg_f, "lib"))
		return do_lib_mode(opt);

	return do_link_main(opt);
}
