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
 * dy-patch/program.c
 *      Patch program clone (Unix)
 */

#include "program.h"

static size_t patch_size;
static unsigned char *patch_file;

static void free_files(void)
{
	if (patch_file) {
		free(patch_file);
		patch_size = 0;
		patch_file = NULL;
	}
}

int program(struct options *opt)
{
	const char *patch_name = opt->arg_i ? opt->arg_i : "-";
	int ret;

	if (opt->operands[0])
		return opt->error = "too many operands", 1;

	if (read_file(patch_name, &patch_file, &patch_size))
		return free_files(), 1;

	ret = patch(opt, (char *)patch_file, patch_size);
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

static void try_create_dirs(const char *name)
{
	char buf[512];
	const char *ptr;
	size_t len;

	/*
	 * This static function is a hack. If there was a good portable
	 * way to create directories, this function should be upgraded.
	 */
	if (strchr(name, '\\') || strlen(name) > 500)
		return;

	ptr = strchr(name, '/');
	while (ptr != NULL) {
		len = (size_t)(ptr - name);
		ptr = strchr(ptr + 1, '/');

		strcpy(&buf[0], "mkdir \"");
		strncpy(&buf[7], name, len);
		buf[len + 7] = '\"';
		buf[len + 8] = '\0';

		if (system(&buf[0])) {
			/*
			 * Handle warn_unused_result attribute.
			 */
			buf[len + 9] = '\0';
		}
	}
}

int write_file(const char *name, unsigned char *data, size_t size)
{
	FILE *fp;

	if (!name || name[0] == '\0' || !data)
		return 0;

	fp = (errno = 0, fopen(name, "wb"));
	if (!fp) {
		try_create_dirs(name);
		fp = (errno = 0, fopen(name, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return 1;
		}
	}

	if (size) {
		if ((errno = 0, fwrite(data, 1, size, fp)) != size) {
			perror("Error"), (void)fclose(fp);
			return 1;
		}
	}
	return (errno = 0, fclose(fp)) ? perror("Error"), 1 : 0;
}
