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
 * dy-patch/patch.c
 *      Patch program clone (Unix)
 */

#include "program.h"

static size_t original_ptr;
static size_t original_size;
static size_t original_line;
static unsigned char *original_file;

static size_t target_size;
static size_t target_bufsize;
static size_t target_line;
static unsigned char *target_file;

static int patch_crlf;
static int target_crlf;

static int state;
static char target_name[512];
static unsigned long range[4];

static int write_target_buffer(const char *line, size_t len)
{
	if (!len) {
		for (;;) {
			char c = line[len];
			if (c == '\r' || c == '\n' || c == '\0')
				break;
			len += 1;
		}
	}

	if (len >= 4095) {
		fputs("Error: line length overflow\n", stderr);
		return -1;
	}
	if (!target_file) {
		target_size = 0;
		target_bufsize = 0x1000;
		target_line = 0;
		if ((target_file = malloc(target_bufsize)) == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return -1;
		}
	}
	if (target_size + len + 2 >= target_bufsize) {
		void *buf;
		if (target_bufsize < 0x40000000) {
			target_bufsize <<= 1;
		} else {
			fputs("Error: too big target file\n", stderr);
			return -1;
		}
		if ((buf = realloc(target_file, target_bufsize)) == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return -1;
		}
		target_file = buf;
	}

	memcpy(&target_file[target_size], &line[0], len);
	target_size += len;

	if (target_crlf)
		target_file[target_size++] = (unsigned char)('\r');
	target_file[target_size++] = (unsigned char)('\n');

	target_line++;
	return 0;
}

static void error_context(void)
{
	printf("Error: file \"%s\" context on line", &target_name[0]);
	printf(" %lu does not match\n", (unsigned long)original_line);
}

static int check_context(const char *line)
{
	const char *s = (const char *)original_file;
	size_t len = strlen(line);
	size_t i;

	for (i = 0; i < len; i++) {
		if (original_ptr >= original_size)
			return error_context(), -1;
		if (s[original_ptr++] != line[i])
			return error_context(), -1;
	}

	if (original_ptr == original_size) {
		original_line += 1;
		return 0;
	}
	if (original_ptr > original_size)
		return error_context(), -1;

	if (s[original_ptr] == '\r') {
		if (++original_ptr >= original_size)
			return error_context(), -1;
	}

	if (s[original_ptr] != '\n')
		return error_context(), -1;

	original_line += 1;
	original_ptr += 1;
	return 0;
}

static int target(const char *line, int verbose)
{
	size_t i;
	char c;

	if (verbose)
		printf("%s\n", line);

	if (range[0]) {
		if (range[0] < original_line + 1) {
			fputs("Error: ranges are not ascending\n", stderr);
			return 1;
		}
		for (i = 0; range[0] != original_line + 1; /* void */) {
			if (original_ptr >= original_size) {
				fputs("Error: malformed patch\n", stderr);
				return -1;
			}
			c = (char)original_file[original_ptr + (i++)];
			if (c == '\n') {
				const char *s = (const char *)original_file;
				if (write_target_buffer(s + original_ptr, 0))
					return -1;
				original_line += 1;
				original_ptr += i;
				i = 0;
			}
		}
	}

	if (line[0] == '!') {
		for (i = 0; original_ptr + i < original_size; /* void */) {
			c = (char)original_file[original_ptr + (i++)];
			if (c == '\n') {
				const char *s = (const char *)original_file;
				if (write_target_buffer(s + original_ptr, 0))
					return -1;
				original_line += 1;
				original_ptr += i;
				i = 0;
			}
		}
		if (i != 0) {
			const char *s = (const char *)original_file;
			if (write_target_buffer(s + original_ptr, i))
				return -1;
			original_line += 1;
			original_ptr += i;
			i = 0;
		}
		if (range[1] != 0 || range[3] != 0)
			return fputs("Error: range mismatch\n", stderr), -1;

	} else if (line[0] == '-') {
		if (range[0] == 0 || range[1] == 0)
			return fputs("Error: range length (-)\n", stderr), -1;
		if (check_context(line + 1))
			return -1;
		range[0] += 1, range[1] -= 1;

	} else if (line[0] == '+') {
		if (range[3] == 0)
			return fputs("Error: range length (+)\n", stderr), -1;
		if (write_target_buffer(line + 1, 0))
			return -1;
		if (range[2] != target_line)
			return fputs("Error: range base (+)\n", stderr), -1;
		range[2] += 1, range[3] -= 1;

	} else if (line[0] == ' ') {
		if (range[0] == 0 || range[1] == 0 || range[3] == 0)
			return fputs("Error: range length\n", stderr), -1;
		if (check_context(line + 1))
			return -1;
		if (write_target_buffer(line + 1, 0))
			return -1;
		if (range[2] != target_line)
			return fputs("Error: range base\n", stderr), -1;
		range[0] += 1, range[1] -= 1;
		range[2] += 1, range[3] -= 1;
	}
	return 0;
}

static int read_original(struct options *opt, char *line)
{
	char *ptr = line + 4;
	size_t i;

	while (ptr[0] != '\0' && ptr[0] != '\t') {
		if (ptr[0] == '\\')
			ptr[0] = '/';
		ptr++;
	}
	ptr[0] = '\0';
	while (ptr[-1] == ' ') {
		ptr[-1] = '\0';
		ptr--;
	}
	ptr = line + 4;

	if (opt->p1) {
		while (ptr[0] != '\0' && ptr[-1] != '/')
			ptr++;
	}
	if (!opt->p0 && !opt->p1) {
		char *last = NULL;
		for (i = 0; ptr[i] != '\0'; i++) {
			if (ptr[i] == '/')
				last = &ptr[i];
		}
		if (last)
			ptr = last + 1;
	}

	if (original_file) {
		free(original_file);
		original_ptr = 0;
		original_size = 0;
		original_line = 0;
		original_file = NULL;
	}

	if (!isgraph((int)ptr[0]) || ptr[0] == '/') {
		if (!strncmp(ptr, "/dev/null", 9))
			return 1;
		fputs("Error: path format\n", stderr);
		return -1;
	}

	if (strlen(ptr) >= sizeof(target_name)) {
		fputs("Error: path too long\n", stderr);
		return -1;
	}

	strcpy(&target_name[0], &ptr[0]);
	if (read_file(ptr, NULL, NULL))
		return 1;
	if (read_file(ptr, &original_file, &original_size))
		return -1;

	for (i = 1; i < original_size; i++) {
		char c1 = (char)original_file[i - 1];
		char c2 = (char)original_file[i];
		if (c1 == '\r' && c2 == '\n') {
			target_crlf = 1;
			break;
		}
	}
	if (!original_size)
		target_crlf = patch_crlf;
	return 0;
}

static int handle_line(struct options *opt, char *line)
{
	int ret = 0;

	switch (state) {
	case 0:
		if (!strncmp(line, "--- ", 4)) {
			int r = read_original(opt, line);
			if (r < 0)
				ret = -1;
			state = (r > 0) ? 1 : 2;
		}
		break;
	case 1:
		if (!strncmp(line, "+++ ", 4)) {
			(void)read_original(opt, line);
			state = 2;
		}
		break;
	case 2:
		if (!strncmp(line, "@@ -", 4)) {
			static const char *err = "Error: range format\n";
			char *next;

			range[0] = 0, range[1] = 1;
			range[2] = 0, range[3] = 1;

			if ((range[0] = strtoul(line + 4, &next, 0)) == 0)
				if (*(line + 4) != '0')
					return fputs(err, stderr), -1;
			if (next[0] == ',' && isdigit((int)next[1]))
				range[1] = strtoul(next + 1, &next, 0);

			while (next[0] == ' ')
				next++;
			if (next[0] != '+')
				return fputs(err, stderr), -1;

			if ((range[2] = strtoul(next + 1, &next, 0)) == 0)
				if (*(next + 1) != '0')
					return fputs(err, stderr), -1;
			if (next[0] == ',' && isdigit((int)next[1]))
				range[3] = strtoul(next + 1, &next, 0);

			while (next[0] == ' ')
				next++;
			if (next[0] != '@' || next[1] != '@')
				return fputs(err, stderr), -1;

			if (!range[0] && range[1])
				return fputs(err, stderr), -1;
			if (!range[2] && range[3])
				return fputs(err, stderr), -1;

			if (opt->verbose) {
				printf("--- %s\n", &target_name[0]);
				printf("+++ %s\n", &target_name[0]);
				printf("@@ -%ld,%ld ", range[0], range[1]);
				printf("+%ld,%ld @@\n", range[2], range[3]);
			}
			state = 3;
		}
		break;
	case 3:
		if (line[0] == '-' || line[0] == '+' || line[0] == ' ') {
			if (target(line, opt->verbose))
				return -1;
		} else if (line[0] == '\0') {
			if (target(" ", opt->verbose))
				return -1;
		} else if (!strncmp(line, "@@ -", 4)) {
			state = 2;
			ret = 1;
		} else if (target_name[0] != '/') {
			if (target("!", 0))
				return -1;
			if (write_file(target_name, target_file, target_size))
				return -1;
			if (!opt->silent)
				printf("patching file %s\n", target_name);
			if (target_file)
				free(target_file);
			target_name[0] = '\0';
			target_file = NULL;
			target_size = 0;
			target_line = 0;
			target_crlf = 0;
			state = 0;
			ret = 1;
		} else {
			ret = -1;
		}
		break;
	default:
		ret = -1;
		break;
	}

	if (ret < 0) {
		const char *n = &target_name[0];
		fprintf(stderr, "Error: cannot patch \"%s\"\n", n);
	}
	return ret;
}

static void free_files(void)
{
	if (original_file) {
		free(original_file);
		original_size = 0;
		original_file = NULL;
	}
	if (target_file) {
		free(target_file);
		target_size = 0;
		target_file = NULL;
	}
}

int patch(struct options *opt, char *patch_file, size_t patch_size)
{
	char *line = patch_file;
	size_t i;

	if (!patch_size || patch_file[patch_size - 1] != '\n') {
		fputs("Error: patch file has no ending newline\n", stderr);
		return 1;
	}

	for (i = 0; i < patch_size; i++) {
		char c = (char)patch_file[i];
		int ret;

		if (c == '\r') {
			patch_file[i] = '\0';
			patch_crlf = 1;
		} else if (c == '\n') {
			patch_file[i] = '\0';
			if ((ret = handle_line(opt, line)) != 0) {
				if (ret != 1 || handle_line(opt, line))
					return free_files(), 1;
			}
			if (i + 1 == patch_size) {
				char last[2] = { '!', '\0' };
				if (handle_line(opt, &last[0]) < 0)
					return free_files(), 1;
				return free_files(), 0;
			}
			line = &patch_file[i + 1];
		}
	}
	return free_files(), 0;
}
