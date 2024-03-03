/*
 * Copyright (c) 2023, 2024 Antti Tiihala
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
 * dsh/operate.c
 *      The Dancy Shell
 */

#include "main.h"

static int state = 1;

static void welcome(void)
{
	fputs("\tWelcome to the Dancy Operating System!\n\n", stdout);
}

int operate(struct options *opt)
{
	int r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	welcome();

	while (state != 0) {
		char *buffer;
		char prompt[2048];
		char cwd[256];
		const size_t cwdmax = 32;
		size_t offset;

		if ((errno = 0, getcwd(&cwd[0], sizeof(cwd))) == NULL)
			return perror("getcwd"), EXIT_FAILURE;

		if ((offset = strlen(&cwd[0])) > cwdmax)
			offset = cwdmax, strcpy(&cwd[offset - 3], "...");

		offset += 4;

		r = snprintf(&prompt[0], sizeof(prompt),
			"\033[1;32m[%s]$\033[0m ", &cwd[0]);

		if (r <= 0)
			return EXIT_FAILURE;

		if ((buffer = dsh_get_input(&prompt[0], offset)) == NULL)
			break;

		if (buffer[0] != '\0')
			fprintf(stdout, "\n%s\n", &buffer[0]);
		else
			fputs("\n", stdout);

		free(buffer);
	}

	return 0;
}
