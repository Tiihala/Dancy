/*
 * Copyright (c) 2019 Antti Tiihala
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
 * dy-hack/program.c
 *      Program for generating miscellaneous output
 */

#include "program.h"

int program(struct options *opt)
{
	const char *cmd;

	if ((cmd = opt->arg_c) == NULL)
		return opt->error = "no command specified", 1;

	if (!strcmp(cmd, "objects"))
		return command_objects_mk(opt);

	return opt->error = "unknown command", 1;
}

int read_file(const char *name, unsigned char **out, size_t *size)
{
	const size_t chunk = 0x1000;
	FILE *fp = stdin;
	int is_stdin = 1;
	unsigned char *ptr;
	size_t size_max;

	size_max = 0, size_max--;

	if (strcmp(name, "-")) {
		fp = (errno = 0, fopen(name, "rb"));
		if (!fp) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return 1;
		}
		is_stdin = 0;
	}

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

		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return !is_stdin ? ((void)fclose(fp), 0) : 0;
}
