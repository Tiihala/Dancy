/*
 * Copyright (c) 2022 Antti Tiihala
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
 * dy-link/lib.c
 *      Linker for Dancy operating system
 */

#include "program.h"

static int process_lib_file(struct options *opt, int i)
{
	const char *lib_name = opt->ofiles[i].name;
	unsigned char *lib_data = opt->ofiles[i].data;
	int lib_size = opt->ofiles[i].size;

	if (opt->verbose) {
		const char *fmt = "Library: %s (%.7s, %i bytes)\n";
		printf(fmt, lib_name, (const char *)lib_data, lib_size);
	}

	if (lib_size == 8)
		return 0;

	return 0;
}

static int process_lib_slot(struct options *opt, int i)
{
	const size_t empty_size = 64;
	unsigned char *empty;
	int r;

	if ((empty = malloc(empty_size)) == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}

	memset(&empty[0], 0, empty_size);

	r = process_lib_file(opt, i);
	free(opt->ofiles[i].data);

	opt->ofiles[i].name = "empty";
	opt->ofiles[i].data = empty;
	opt->ofiles[i].size = (int)empty_size;
	opt->ofiles[i].type = -1;

	return r;
}

int lib_read_ofiles(struct options *opt)
{
	const unsigned char header[8] =
		{ 0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E, 0x0A };
	int i;

	if (opt->nr_ofiles == 0) {
		unsigned char *arch_empty;

		if ((arch_empty = malloc(sizeof(header))) == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return 1;
		}

		memcpy(&arch_empty[0], &header[0], sizeof(header));
		opt->nr_ofiles = 1;

		opt->ofiles[0].name = "arch";
		opt->ofiles[0].data = arch_empty;
		opt->ofiles[0].size = (int)sizeof(header);
		opt->ofiles[0].type = 0;
	}

	for (i = 0; i < opt->nr_ofiles; i++) {
		if (opt->ofiles[i].size < (int)sizeof(header))
			continue;
		if (memcmp(opt->ofiles[i].data, header, sizeof(header)))
			continue;
		if (process_lib_slot(opt, i))
			return 1;
	}

	return 0;
}

struct lib_symbol {
	const char *name;
	int ofile;
	int section;
	char _buffer[12];
};

static struct lib_symbol *read_external_symbols(struct options *opt)
{
	unsigned long symbols_total = 0;
	struct lib_symbol *r, *ls;
	int i, j;

	for (i = 0; i < opt->nr_ofiles; i++) {
		unsigned char *obj_data = opt->ofiles[i].data;
		unsigned long symbols = LE32(&obj_data[12]);

		if (symbols_total > ULONG_MAX - symbols) {
			fputs("Error: symbol buffer overflow\n", stderr);
			return NULL;
		}

		symbols_total += symbols;

		if (symbols_total > 0x00FFFFFF) {
			fputs("Error: too many symbols\n", stderr);
			return NULL;
		}
	}

	symbols_total += 1;

	{
		size_t size = (size_t)symbols_total * sizeof(*r);

		if ((r = malloc(size)) == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return NULL;
		}

		memset(r, 0, size);
		ls = r;
	}

	for (i = 0; i < opt->nr_ofiles; i++) {
		unsigned char *obj_data = opt->ofiles[i].data;
		unsigned long offset = LE32(&obj_data[8]);
		unsigned long symbols = LE32(&obj_data[12]);

		for (j = 0; j < (int)symbols; j++) {
			unsigned char *sym = &obj_data[(int)offset + j * 18];

			if (LE32(&sym[0])) {
				memcpy(&ls->_buffer[0], &sym[0], 8);
				ls->name = &ls->_buffer[0];
			} else {
				unsigned char *str = obj_data;
				str += (offset + symbols * 18);
				str += LE32(&sym[4]);
				ls->name = (const char *)str;
			}

			ls->ofile = i;
			ls->section = (int)LE16(&sym[12]);

			if ((int)sym[16] != 2 || ls->name[0] == '\0')
				memset(ls, 0, sizeof(*ls));
			else
				ls += 1;

			j += (int)sym[17];
		}
	}

	return r;
}

int lib_main(struct options *opt)
{
	int i;

	for (i = 0; i < opt->nr_lib_ofiles; i++) {
		const char *obj_name = opt->lib_ofiles[i].name;
		unsigned char *obj_data = opt->lib_ofiles[i].data;
		int obj_size = opt->lib_ofiles[i].size;

		if (validate_obj(obj_name, obj_data, obj_size))
			return 1;
	}

	return 0;
}

int lib_set_ofiles(struct options *opt)
{
	struct lib_symbol *ls = read_external_symbols(opt);
	int i, j;

	if (!ls)
		return 1;

	/*
	 * Check what object files from libraries are relevant.
	 */
	for (;;) {
		int state = 0;

		for (i = 0; ls[i].name != NULL; i++) {
			if (ls[i].section != 0)
				continue;
			if (opt->ofiles[ls[i].ofile].type != 0)
				continue;

			for (j = 0; ls[j].name != NULL; j++) {
				if (ls[j].section <= 0 || i == j)
					continue;
				if (ls[i].name[0] != ls[j].name[0])
					continue;
				if (strcmp(&ls[i].name[0], &ls[j].name[0]))
					continue;

				if (opt->ofiles[ls[j].ofile].type == 1) {
					opt->ofiles[ls[j].ofile].type = 0;
					state = 1;
				}
				break;
			}

			ls[i].section = -1;
		}

		if (state == 0)
			break;
	}

	/*
	 * Remove all unnecessary object files from libraries.
	 */
	for (i = 0; i < opt->nr_ofiles; i++) {
		const size_t empty_size = 64;
		unsigned char *empty;

		if (opt->ofiles[i].type != 1)
			continue;

		if ((empty = malloc(empty_size)) == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return free(ls), 1;
		}

		memset(&empty[0], 0, empty_size);
		free(opt->ofiles[i].data);

		opt->ofiles[i].name = "empty";
		opt->ofiles[i].data = empty;
		opt->ofiles[i].size = (int)empty_size;
		opt->ofiles[i].type = -1;
	}

	return free(ls), 0;
}
