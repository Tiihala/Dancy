/*
 * Copyright (c) 2024 Antti Tiihala
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
 * more/operate.c
 *      Display files on a page-by-page basis
 */

#include "main.h"

static int ws_col;
static int ws_row;

static int temporary_implementation(FILE *stream)
{
	int r;

	save_termios();

	for (;;) {
		char buffer[4096];

		if (fgets(&buffer[0], 4096, stream) == NULL)
			break;

		fputs(&buffer[0], stdout);

		printf("\033[2K\r\033[7m--More--\033[0m\033[%dG", ws_col);
		fflush(stdout);

		r = fgetc(stderr);

		printf("\033[2K\r");
		fflush(stdout);

		if (r == EOF || r == 'q' || r == 'Q')
			break;
	}

	restore_termios();

	return 0;
}

static int more_stream(FILE *stream)
{
	return temporary_implementation(stream);
}

static int more(const char *path)
{
	FILE *stream;
	int r;

	if (ws_col == 0 || (path == NULL && isatty(fileno(stdin))))
		return more_cat(path);

	if (path == NULL)
		return more_stream(stdin);

	if ((stream = fopen(path, "rb")) == NULL)
		return perror(path), EXIT_FAILURE;

	r = more_stream(stream);
	fclose(stream);

	return r;
}

int operate(struct options *opt)
{
	struct winsize ws = { 0, 0, 0, 0 };

	ioctl(fileno(stdout), TIOCGWINSZ, &ws);

	if (ws.ws_col > 0 && ws.ws_row > 0) {
		ws_col = ws.ws_col;
		ws_row = ws.ws_row;
	}

	if (opt->operands[0] == NULL)
		return more(NULL);

	if (opt->operands[1] != NULL) {
		opt->error = "too many operands";
		return EXIT_FAILURE;
	}

	return more(opt->operands[0]);
}
