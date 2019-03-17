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
 * dy-hack/command.c
 *      Program for generating miscellaneous output
 */

#include "program.h"

int command_objects_mk(struct options *opt)
{
	unsigned char *input = NULL;
	size_t size = 0;
	FILE *output = stdout;
	int is_acpica = 0;
	int is_stdout = 1;
	size_t i, j;

	if (!opt->operands[0])
		return opt->error = "missing input file", 1;
	if (opt->operands[1])
		return opt->error = "too many operands", 1;
	if (read_file(opt->operands[0], &input, &size))
		return 1;

	if (size && input[size - 1] != '\n') {
		fputs("Error: input does not end with newline\n", stderr);
		return 1;
	}

	if (opt->arg_o && strcmp(opt->arg_o, "-")) {
		output = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!output)
			return perror("Error"), 1;
		is_stdout = 0;
	}

	fputs("# Dancy Operating System\n\n", output);
	fputs("########################################", output);
	fputs("######################################\n", output);

	for (i = 0, j = 0; i < size; i++) {
		char c = (char)input[i];
		const char *name;
		size_t len;

		if (c == '\\')
			c = '/', input[i] = (unsigned char)c;
		else if (c == '\r')
			c = '\0', input[i] = (unsigned char)c;
		else if (c == '\n')
			c = '\0', input[i] = (unsigned char)c;

		if (c != '\0')
			continue;

		name = (const char *)&input[j], j = i + 1;
		while (name[0] == '.' || name[0] == '/')
			name += 1;

		if (strncmp(name, "common", 6) && strncmp(name, "kernel", 6))
			continue;
		if ((len = strlen(name)) > 64)
			continue;

		if (strstr(name, "/acpica/"))
			is_acpica = 1;
		else
			is_acpica = 0;

		if (!is_acpica && !strncmp(name + (len - 4), ".asm", 4)) {
			int l = (int)len;
			if (strstr(name, "/a32/") == NULL)
				continue;
			fprintf(output, "\n./o32%.*so: ", l - 9, name + 6);
			fprintf(output, "./%s $(DANCY_DEPS)\n", name);
			fprintf(output, "\t$(DANCY_A32)$@ ./%s\n", name);
		}

		if (!is_acpica && !strncmp(name + (len - 2), ".c", 2)) {
			int l = (int)len;
			fprintf(output, "\n./o32%.*so: ", l - 7, name + 6);
			fprintf(output, "./%s $(DANCY_DEPS)\n", name);
			fprintf(output, "\t$(DANCY_O32)$@ ./%s\n", name);
		}

		if (is_acpica && !strncmp(name + (len - 2), ".c", 2)) {
			int l = (int)len;
			fprintf(output, "\n./o32%.*so: ", l - 7, name + 6);
			fprintf(output, "$(ACPICA_DEPS)\n", name);
			fprintf(output, "\t$(ACPICA_O32)$@ ./%s\n", name);
		}
	}

	for (i = 0, j = 0; i < size; i++) {
		char c = (char)input[i];
		const char *name;
		size_t len;

		if (c != '\0')
			continue;

		name = (const char *)&input[j], j = i + 1;
		while (name[0] == '.' || name[0] == '/')
			name += 1;

		if (strncmp(name, "common", 6) && strncmp(name, "kernel", 6))
			continue;
		if ((len = strlen(name)) > 64)
			continue;

		if (strstr(name, "/acpica/"))
			is_acpica = 1;
		else
			is_acpica = 0;

		if (!is_acpica && !strncmp(name + (len - 4), ".asm", 4)) {
			int l = (int)len;
			if (strstr(name, "/a64/") == NULL)
				continue;
			fprintf(output, "\n./o64%.*so: ", l - 9, name + 6);
			fprintf(output, "./%s $(DANCY_DEPS)\n", name);
			fprintf(output, "\t$(DANCY_A64)$@ ./%s\n", name);
		}

		if (!is_acpica && !strncmp(name + (len - 2), ".c", 2)) {
			int l = (int)len;
			fprintf(output, "\n./o64%.*so: ", l - 7, name + 6);
			fprintf(output, "./%s $(DANCY_DEPS)\n", name);
			fprintf(output, "\t$(DANCY_O64)$@ ./%s\n", name);
		}

		if (is_acpica && !strncmp(name + (len - 2), ".c", 2)) {
			int l = (int)len;
			fprintf(output, "\n./o64%.*so: ", l - 7, name + 6);
			fprintf(output, "$(ACPICA_DEPS)\n", name);
			fprintf(output, "\t$(ACPICA_O64)$@ ./%s\n", name);
		}
	}

	if (!is_stdout && (errno = 0, fclose(output)))
		return perror("Error"), 1;
	return 0;
}
