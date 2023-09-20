/*
 * Copyright (c) 2023 Antti Tiihala
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
	const char m[] = "\tWelcome to the Dancy Operating System!\n\n";
	write(1, &m[0], sizeof(m));
}

static int demonstration(void)
{
	static unsigned char buffer[0x20000];
	ssize_t size[2];

	size[0] = read(0, &buffer[0], sizeof(buffer));
	size[1] = 0;

	if (size[0] < 0)
		return perror("read"), EXIT_FAILURE;

	while (size[0] > 0) {
		int c = (int)buffer[size[0] - 1];

		if (c != '\r' && c != '\n')
			break;

		buffer[(size[0] -= 1)] = 0;
	}

	if (size[0] > 0 && size[0] < (ssize_t)sizeof(buffer)) {
		buffer[size[0]] = '\n';
		size[0] += 1;
	}

	while (size[0] > size[1]) {
		size_t s = (size_t)(size[0] - size[1]);
		ssize_t w;

		w = (errno = EIO, write(1, &buffer[size[1]], s));

		if (w <= 0)
			return perror("write"), EXIT_FAILURE;

		size[1] += w;
	}

	return 0;
}

int operate(struct options *opt)
{
	int r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	welcome();

	while (state != 0) {
		char buffer[256];
		char cwd[256];
		ssize_t w;

		r = snprintf(&buffer[0], sizeof(buffer),
			"\033[1;32m[%s]$\033[0m ",
			getcwd(&cwd[0], sizeof(cwd)));

		if (r <= 0)
			return EXIT_FAILURE;

		w = (errno = EIO, write(1, &buffer[0], (size_t)r));

		if (w <= 0)
			return perror("write"), EXIT_FAILURE;

		demonstration();
	}

	return 0;
}
