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
 * which/operate.c
 *      Show the full path of commands
 */

#include "main.h"

static void func_1(struct options *opt, const char *path)
{
	struct stat s;

	if (stat(path, &s) == 0) {
		if (!opt->silent)
			printf("%s\n", path);
		opt->state = 1;
	}
}

static void func_2(struct options *opt,
	const char *dir_name, const char *name)
{
	char path[512];

	if (strlen(dir_name) >= 250 || strlen(name) >= 250)
		return;

	strcpy(&path[0], dir_name);
	strcat(&path[0], "/");
	strcat(&path[0], name);

	func_1(opt, &path[0]);
}

static void func_3(struct options *opt, const char *name)
{
	const char *p1 = getenv("PATH");
	char *p2, *p3, *p4;

	if (p1 == NULL || *p1 == '\0')
		p1 = "/bin";

	if ((p2 = strdup(p1), p3 = p2) == NULL) {
		fputs("which: Cannot allocate memory\n", stderr);
		opt->state = -1;
		return;
	}

	while (p3 && *p3 != '\0') {
		if ((p4 = strchr(p3, ':')) != NULL)
			*p4++ = '\0';
		else
			p4 = strchr(p3, '\0');

		if (*p3 == '/' && p4 != NULL)
			func_2(opt, p3, name);

		if (!opt->print_all && opt->state == 1)
			break;

		p3 = p4;

	}

	free(p2);
}

int operate(struct options *opt)
{
	int i, r = 0;

	if (opt->operands[0] == NULL)
		return EXIT_FAILURE;

	for (i = 0; opt->operands[i] != NULL; i++) {
		const char *operand = opt->operands[i];

		opt->state = 0;

		if (strchr(operand, '/'))
			func_1(opt, operand);
		else
			func_3(opt, operand);

		if (opt->state <= 0)
			r = EXIT_FAILURE;

		if (opt->state < 0)
			break;
	}

	return r;
}
