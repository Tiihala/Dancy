/*
 * Copyright (c) 2026 Antti Tiihala
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
 * ld-dancy/operate.c
 *      The executable loader
 */

#include "main.h"

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	size_t chunk = 0x1000;
	const size_t size_max = ~((size_t)0);
	unsigned char *ptr;
	FILE *fp;

	fp = (errno = 0, fopen(name, "rb"));

	if (!fp) {
		const char *fmt = "Error: reading file \'%s\' (%s)\n";
		fprintf(stderr, fmt, name, strerror(errno));
		return EXIT_FAILURE;
	}

	*size = 0;
	ptr = malloc(chunk);

	for (;;) {
		size_t bytes_read;
		int my_errno;
		int stop;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			return (void)fclose(fp), EXIT_FAILURE;
		}

		*out = ptr;
		ptr += *size;

		bytes_read = (errno = 0, fread(ptr, 1, chunk, fp));
		my_errno = errno;
		stop = feof(fp);

		if (ferror(fp) || (!stop && bytes_read != chunk)) {
			const char *fmt = "Error: reading file \'%s\' (%s)\n";
			fprintf(stderr, fmt, name, strerror(my_errno));
			return (void)fclose(fp), EXIT_FAILURE;
		}

		*size += ((bytes_read <= chunk) ? bytes_read : chunk);
		if (stop)
			break;

		if (chunk < 0x00200000)
			chunk <<= 1;

		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \'%s\' (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return (void)fclose(fp), EXIT_FAILURE;
		}

		ptr = realloc(*out, *size + chunk);
	}

	return (void)fclose(fp), 0;
}

int operate(struct options *opt)
{
	int r = EXIT_FAILURE;
	unsigned char *p;
	size_t s;

	if (opt->operands[0] == NULL)
		return opt->error = "missing program", EXIT_FAILURE;

	if (read_file(opt->operands[0], &opt->program, &opt->program_size))
		return EXIT_FAILURE;

	p = opt->program;
	s = opt->program_size;

	if (s >= 4 && p[0] == 0x7F && !memcmp(&p[1], "ELF", 3))
		r = elf_execute(opt);
	else
		fprintf(stderr, "ld-dancy: %s: "
			"Exec format not supported\n", opt->operands[0]);

	free(opt->program), opt->program = NULL, opt->program_size = 0;

	return r;
}
