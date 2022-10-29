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

static int lib_error(const char *lib_name, const char *msg)
{
	fprintf(stderr, "%s (library): %s\n", lib_name, msg);
	return 1;
}

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

	lib_data += 8;
	lib_size -= 8;

	while (lib_size > 0) {
		int obj_file_size = 0;
		int id[2];
		int j;

		/*
		 * Ignore the padding bytes.
		 */
		while (lib_data[0] == 0x0A && lib_size > 0) {
			lib_data += 1;
			lib_size -= 1;
		}

		if (lib_size < 60)
			break;

		id[0] = (int)lib_data[0];
		id[1] = (int)lib_data[1];

		/*
		 * Check header ending characters.
		 */
		if (lib_data[58] != 0x60 || lib_data[59] != 0x0A)
			return lib_error(lib_name, "header format");

		for (j = 0; j < 48; j++) {
			int c = (int)lib_data[j];

			if (c < 0x20 || c > 0x7E)
				return lib_error(lib_name, "header format");
		}

		if ((int)lib_data[48] == '0' && (int)lib_data[49] != 0x20)
			return lib_error(lib_name, "octal file size");

		/*
		 * File sizes are limited to 999 999 999 bytes.
		 */
		if ((int)lib_data[57] != 0x20)
			return lib_error(lib_name, "object file size");

		for (j = 48; j < 58; j++) {
			int c = (int)lib_data[j];

			if (c == 0x20)
				break;
			if (c < '0' || c > '9')
				return lib_error(lib_name, "header format");

			obj_file_size *= 10;
			obj_file_size += (c - '0');
		}

		lib_data += 60;
		lib_size -= 60;

		if (lib_size < obj_file_size)
			return lib_error(lib_name, "erroneous format");

		/*
		 * Ignore special library members.
		 */
		if (id[0] == '/' && (id[1] < '0' || id[1] > '9')) {
			lib_data += obj_file_size;
			lib_size -= obj_file_size;
			continue;
		}

		/*
		 * Check the minimum size of the supported object files.
		 */
		if (obj_file_size < 20) {
			lib_data += obj_file_size;
			lib_size -= obj_file_size;
			continue;
		}

		/*
		 * Check the supported object file signatures.
		 */
		{
			unsigned long arch_type = LE16(&lib_data[0]);

			if (arch_type != 0x014C && arch_type != 0x8664) {
				lib_data += obj_file_size;
				lib_size -= obj_file_size;
				continue;
			}
		}

		/*
		 * Check the array limit.
		 */
		if (opt->nr_ofiles >= OFILES_LIMIT) {
			fputs("Error: too many library objects\n", stderr);
			return 1;
		}

		/*
		 * Allocate a buffer for the object file and add it to
		 * the input file array.
		 */
		{
			size_t buffer_size = (size_t)obj_file_size;
			void *buffer = malloc(buffer_size);

			if (!buffer) {
				fputs("Error: not enough memory\n", stderr);
				return 1;
			}

			memcpy(buffer, &lib_data[0], buffer_size);

			opt->ofiles[opt->nr_ofiles].name = "object";
			opt->ofiles[opt->nr_ofiles].data = buffer;
			opt->ofiles[opt->nr_ofiles].size = (int)buffer_size;
			opt->ofiles[opt->nr_ofiles].type = 1;
		}

		opt->nr_ofiles += 1;

		lib_data += obj_file_size;
		lib_size -= obj_file_size;
	}

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

static int add_size(unsigned long *output_buffer_size, unsigned long add)
{
	if (*output_buffer_size > ULONG_MAX - add)
		*output_buffer_size = 0xFFFFFFFF;

	if (*output_buffer_size > 0x7FFFFFFF) {
		fputs("Error: library size overflow\n", stderr);
		return 1;
	}

	*output_buffer_size += add;

	return 0;
}

static int write_lib(struct options *opt, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!opt->arg_o)
		return free(out), 0;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return free(out), 1;
		}
		is_stdout = 0;
	}

	if ((errno = 0, fwrite(out, 1, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return free(out), 1;
	}

	free(out);
	errno = 0;

	if (is_stdout)
		return fflush(fp) ? perror("Error"), 1 : 0;

	return fclose(fp) ? perror("Error"), 1 : 0;
}

int lib_main(struct options *opt)
{
	unsigned long symbols = 0;
	unsigned long output_buffer_size = 128;
	unsigned char *output_buffer;
	size_t output_real_size = 0;
	struct lib_symbol *ls;
	int i;

	for (i = 0; i < opt->nr_lib_ofiles; i++) {
		const char *obj_name = opt->lib_ofiles[i].name;
		unsigned char *obj_data = opt->lib_ofiles[i].data;
		int obj_size = opt->lib_ofiles[i].size;

		if (LE16(&obj_data[0]) == 0)
			continue;

		if (validate_obj(obj_name, obj_data, obj_size))
			return 1;

		if (add_size(&output_buffer_size, (unsigned long)obj_size))
			return 1;

		if (add_size(&output_buffer_size, 64))
			return 1;
	}

	/*
	 * Read all external symbols from the new object files.
	 */
	{
		int nr_ofiles = opt->nr_ofiles;
		void *ofiles = opt->ofiles;

		opt->nr_ofiles = opt->nr_lib_ofiles;
		opt->ofiles = opt->lib_ofiles;

		ls = read_external_symbols(opt);

		opt->nr_ofiles = nr_ofiles;
		opt->ofiles = ofiles;
	}

	if (!ls)
		return 1;

	for (i = 0; ls[i].name != NULL; i++) {
		size_t len = strlen(ls[i].name);

		if (add_size(&output_buffer_size, (unsigned long)len))
			return free(ls), 1;

		if (add_size(&output_buffer_size, 5))
			return free(ls), 1;

		symbols += 1;
	}

	/*
	 * Allocate a buffer that has more than enough space
	 * for the output file.
	 */
	if ((output_buffer = malloc((size_t)output_buffer_size)) == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return free(ls), 1;
	}

	memset(output_buffer, 0, (size_t)output_buffer_size);

	/*
	 * Write the first special member.
	 */
	{
		const unsigned char header[8] =
			{ 0x21, 0x3C, 0x61, 0x72, 0x63, 0x68, 0x3E, 0x0A };
		unsigned long member_size = 0;

		memcpy(&output_buffer[output_real_size], &header[0], 8);
		output_real_size += 8;

		memset(&output_buffer[output_real_size], 0x20, 60);

		output_buffer[output_real_size + 0] = '/';
		output_buffer[output_real_size + 16] = '-';
		output_buffer[output_real_size + 17] = '1';
		output_buffer[output_real_size + 40] = '0';

		W_BE32(&output_buffer[output_real_size + 60 + 0], symbols);
		member_size += 4;

		for (i = 0; i < (int)symbols; i++) {
			unsigned long val = (unsigned long)ls[i].ofile;
			unsigned char *p = output_buffer;

			p += 60;
			p += output_real_size;
			p += member_size;

			/*
			 * Use temporary values in the offset array.
			 */
			val = (val << 1) | 1;
			W_LE32(&p[0], val);
			member_size += 4;
		}

		for (i = 0; i < (int)symbols; i++) {
			const char *name = ls[i].name;
			char *p = (char *)output_buffer;

			p += 60;
			p += output_real_size;
			p += member_size;

			strcpy(p, name);
			member_size += (unsigned long)(strlen(name) + 1);
		}

		if (symbols == 0)
			member_size += 1;

		sprintf((char *)&output_buffer[output_real_size + 48],
			"%-10lu", member_size);

		output_buffer[output_real_size + 58] = 0x60;
		output_buffer[output_real_size + 59] = 0x0A;

		output_real_size += 60;
		output_real_size += member_size;

		/*
		 * Add padding bytes to align headers.
		 */
		while ((output_real_size & 1) != 0)
			output_buffer[output_real_size++] = 0x0A;
	}

	free(ls), ls = NULL;

	for (i = 0; i < opt->nr_lib_ofiles; i++) {
		unsigned char *obj_data = opt->lib_ofiles[i].data;
		int obj_size = opt->lib_ofiles[i].size;
		unsigned long member_size = (unsigned long)obj_size;
		unsigned long j;

		if (LE16(&obj_data[0]) == 0)
			continue;

		memset(&output_buffer[output_real_size], 0x20, 60);

		sprintf((char *)&output_buffer[output_real_size + 0],
			"%lX.o/", (unsigned long)output_real_size);

		for (j = 0; j < 16; j++) {
			if (output_buffer[output_real_size + j] == 0x00) {
				output_buffer[output_real_size + j] = 0x20;
				break;
			}
		}

		output_buffer[output_real_size + 16] = '-';
		output_buffer[output_real_size + 17] = '1';
		output_buffer[output_real_size + 40] = '0';

		sprintf((char *)&output_buffer[output_real_size + 48],
			"%-10lu", member_size);

		output_buffer[output_real_size + 58] = 0x60;
		output_buffer[output_real_size + 59] = 0x0A;

		memcpy(&output_buffer[output_real_size + 60],
			&obj_data[0], (size_t)member_size);

		/*
		 * Update the temporary values in the first member.
		 */
		for (j = 0; j < symbols; j++) {
			unsigned char *p = output_buffer;
			unsigned long val;

			p += 72;
			p += (j * 4);
			val = LE32(&p[0]) >> 1;

			if ((int)val == i)
				W_LE32(&p[0], output_real_size);
		}

		output_real_size += 60;
		output_real_size += member_size;

		/*
		 * Add padding bytes to align headers.
		 */
		while ((output_real_size & 1) != 0)
			output_buffer[output_real_size++] = 0x0A;
	}

	/*
	 * Check and convert the temporary values to final
	 * values and use big endian integers.
	 */
	for (i = 0; i < (int)symbols; i++) {
		unsigned char *p = output_buffer;
		unsigned long val;

		p += 72;
		p += (i * 4);
		val = LE32(&p[0]);

		if (val == 0 || (val & 1) != 0) {
			fputs("Library: unexpected behavior\n", stderr);
			return free(output_buffer), 1;
		}

		W_BE32(&p[0], val);
	}

	if (output_buffer_size < output_real_size)
		fputs("Library: unexpected behavior\n", stderr), exit(1);

	if (opt->verbose) {
		printf("Library output buffer, %lu bytes\n",
			(unsigned long)output_buffer_size);
		printf("Library output file, %lu bytes\n",
			(unsigned long)output_real_size);
	}

	return write_lib(opt, output_buffer, output_real_size);
}

int lib_set_ofiles(struct options *opt)
{
	struct lib_symbol *ls = read_external_symbols(opt);
	int i, j;

	if (!ls)
		return 1;

	/*
	 * Check what object files from libraries are relevant. It means
	 * that in normal object files there are unresolved symbols that
	 * can be made "resolved" by using the libraries.
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
	 * Remove all unnecessary object files that were read from libraries.
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
