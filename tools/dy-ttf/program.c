/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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
 * dy-ttf/program.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static size_t input_size;
static unsigned char *input_file;

static void free_files(void)
{
	if (input_file) {
		free(input_file);
		input_size = 0;
		input_file = NULL;
	}
}

int program(struct options *opt)
{
	int ret;

	if (!opt->operands[0])
		return opt->error = "no input", 1;

	if (opt->render) {
		if (!opt->arg_o)
			return opt->error = "no output", 1;
		if (!opt->operands[1])
			return opt->error = "no code point", 1;

		opt->code_point = strtoul(opt->operands[1], NULL, 0);
		if (opt->code_point == 0 || opt->code_point > 0x10FFFF)
			return opt->error = "invalid code point", 1;
	}

	if (opt->operands[(opt->render ? 2 : 1)])
		return opt->error = "too many operands", 1;

	if (read_file(opt->operands[0], &input_file, &input_size))
		return free_files(), 1;

	if (opt->verbose) {
		const char *fmt = "Read file %s, %u bytes\n";
		printf(fmt, opt->operands[0], (unsigned)input_size);
	}

	/*
	 * Make sure that input_size is 4-byte aligned. It is safe
	 * to extend the buffer (see read_file and the chunk sizes).
	 */
	if (input_size != 0 && (input_size % 4) != 0) {
		size_t bytes_to_clear = (input_size % 4);

		while (bytes_to_clear--)
			input_file[input_size++] = 0;
	}

	if ((ret = table_init(input_file, input_size)) != 0) {
		fprintf(stderr, "Error: input file format (code %i)\n", ret);
		return free_files(), ret;
	}

	ret = ttf_main(opt);
	return free_files(), ret;
}

int read_file(const char *name, unsigned char **out, size_t *size)
{
	size_t chunk = 0x1000;
	const size_t size_max = ~((size_t)0);
	FILE *fp = stdin;
	int is_stdin = 1;
	unsigned char *ptr;

	if (strcmp(name, "-")) {
		fp = (errno = 0, fopen(name, "rb"));
		if (!fp) {
			const char *fmt = "Error: opening file \"%s\" (%s)\n";
			if (out)
				fprintf(stderr, fmt, name, strerror(errno));
			return 1;
		}
		is_stdin = 0;
	}
	if (!out)
		return !is_stdin ? ((void)fclose(fp), 0) : 0;

	*size = 0;
	ptr = malloc(chunk);
	for (;;) {
		size_t bytes_read;
		int my_errno;
		int stop;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}
		*out = ptr;
		ptr += *size;

		bytes_read = (errno = 0, fread(ptr, 1, chunk, fp));
		my_errno = errno;
		stop = feof(fp);

		if (ferror(fp) || (!stop && bytes_read != chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(my_errno));
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}

		*size += ((bytes_read <= chunk) ? bytes_read : chunk);
		if (stop)
			break;

		if (chunk < 0x00200000)
			chunk <<= 1;
		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return !is_stdin ? ((void)fclose(fp), 0) : 0;
}

int write_file(struct options *opt, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!opt->arg_o)
		return 0;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return 1;
		}
		is_stdout = 0;
	}
	if ((errno = 0, fwrite(out, 1, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return 1;
	}

	if (is_stdout)
		return (errno = 0, fflush(fp)) ? perror("Error"), 1 : 0;
	return fclose(fp) ? perror("Error"), 1 : 0;
}
