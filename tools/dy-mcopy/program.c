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
 * dy-mcopy/program.c
 *      Program for adding files into FAT12/FAT16 file system images
 */

#include "program.h"

static size_t image_file_size;
static size_t source_file_size;
static unsigned char *image_file;
static unsigned char *source_file;
static int opt_verbose;

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	const size_t chunk = 16384;
	unsigned char *ptr;
	FILE *fp;

	fp = (errno = 0, fopen(name, "rb"));
	if (!fp) {
		const char *fmt = "Error: reading file \"%s\" (%s)\n";
		fprintf(stderr, fmt, name, strerror(errno));
		return 1;
	}

	*size = 0;
	ptr = malloc(chunk);
	for (;;) {
		size_t bytes_read;
		int my_errno;
		int stop;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			return (void)fclose(fp), 1;
		}
		*out = ptr;
		ptr += *size;

		bytes_read = (errno = 0, fread(ptr, 1u, chunk, fp));
		my_errno = errno;
		stop = feof(fp);

		if (ferror(fp) || (!stop && bytes_read != chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(my_errno));
			return (void)fclose(fp), 1;
		}

		*size += ((bytes_read <= chunk) ? bytes_read : chunk);
		if (stop)
			break;

		if (!(*size < SIZE_MAX - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return (void)fclose(fp), 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return (void)fclose(fp), 0;
}

static void free_files(void)
{
	if (opt_verbose) {
		unsigned long s1 = (unsigned long)image_file_size;
		unsigned long s2 = (unsigned long)source_file_size;
		printf("Info: free image buffer, %lu bytes\n", s1);
		printf("Info: free source file buffer, %lu bytes\n", s2);
	}
	if (image_file) {
		free(image_file);
		image_file_size = 0;
		image_file = NULL;
	}
	if (source_file) {
		free(source_file);
		source_file_size = 0;
		source_file = NULL;
	}
}

int program(struct options *opt)
{
	if (!opt->arg_i)
		return opt->error = "missing image file", 1;
	if (!opt->operands[0])
		return opt->error = "missing source-file", 1;
	if (!opt->operands[1])
		return opt->error = "missing path/destination-file", 1;

	opt_verbose = opt->verbose;
	if ((errno = 0, atexit(free_files)))
		return perror("atexit error"), 1;

	if (read_file(opt->arg_i, &image_file, &image_file_size))
		return 1;
	if (read_file(opt->operands[0], &source_file, &source_file_size))
		return 1;

	return 0;
}
