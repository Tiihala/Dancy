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
 * dsh/line.c
 *      The Dancy Shell
 */

#include "main.h"

static int ws_col = 80;
static int ws_row = 25;

static void get_winsize(void)
{
	struct winsize ws = { 0, 0, 0, 0 };
	const unsigned int max = 0x7FFF;

	ioctl(1, TIOCGWINSZ, &ws);

	if (ws.ws_col == 0 || ws.ws_col > max)
		return;

	if (ws.ws_row == 0 || ws.ws_row > max)
		return;

	ws_col = ws.ws_col;
	ws_row = ws.ws_row;
}

static char *get_from_command_string(struct options *opt)
{
	size_t i = 0;
	size_t size = strlen(&opt->command_string[opt->command_string_i]) + 1;
	char *buffer;

	if (size == 1)
		return NULL;

	if ((buffer = malloc(size)) == NULL) {
		fputs("dsh: out of memory\n", stderr);
		return NULL;
	}

	for (;;) {
		char c = opt->command_string[opt->command_string_i];

		if (c == '\0')
			break;

		opt->command_string_i += 1;

		if (c == '\n')
			break;

		if (c == '\r')
			continue;

		buffer[i++] = c;
	}

	buffer[i] = '\0';

	return buffer;
}

static char *get_from_file(struct options *opt)
{
	size_t i = 0;
	size_t size = 0x2000;
	char *buffer;

	if (opt->input_stream_eof)
		return NULL;

	if ((buffer = malloc(size)) == NULL) {
		fputs("dsh: out of memory\n", stderr);
		return NULL;
	}

	for (;;) {
		int c = fgetc(opt->input_stream);

		if (c == EOF) {
			opt->input_stream_eof = 1;
			break;
		}

		if (c == '\n' || (char)c == '\0')
			break;

		if (c == '\r')
			continue;

		if (i + 2 > size) {
			fputs("dsh: line length error", stderr);
			return free(buffer), NULL;
		}

		buffer[i++] = (char)c;
	}

	buffer[i] = '\0';

	return buffer;
}

char *dsh_get_input(struct options *opt, const char *prompt, size_t offset)
{
	size_t column = offset;
	size_t i = 0;

	int utf8_state = 0;
	unsigned char u[8];

	const size_t size = 0x2000;
	char *buffer;

	if (opt->command_string != NULL)
		return get_from_command_string(opt);

	if (opt->input_stream != NULL)
		return get_from_file(opt);

	if ((buffer = malloc(size)) == NULL) {
		fputs("dsh: out of memory\n", stderr);
		return NULL;
	}

	fputs(prompt, stdout);

	dsh_save_termios();
	get_winsize();

	for (;;) {
		int c = getchar();

		if (c == EOF || c == '\n' || (char)c == '\0')
			break;

		if (c == '\r')
			continue;

		if (c == 0x7F) {
			if (i > 0) {
				c = (int)buffer[--i];
				buffer[i] = '\0';

				fputs("\x08\x20\x08", stdout);
				column -= 1;

				while (i > 0 && (c & 0xC0) == 0x80) {
					c = (int)buffer[--i];
					buffer[i] = '\0';
				}
			}
			continue;
		}

		if (column + 2 > (size_t)ws_col)
			continue;

		if (i + 2 > size) {
			fputs("dsh: line length error", stderr);
			dsh_restore_termios();
			return free(buffer), NULL;
		}

		buffer[i++] = (char)c;
		u[utf8_state++] = (unsigned char)c;

		if ((u[0] & 0xE0) == 0xC0 && utf8_state < 2)
			continue;
		if ((u[0] & 0xF0) == 0xE0 && utf8_state < 3)
			continue;
		if ((u[0] & 0xF8) == 0xF0 && utf8_state < 4)
			continue;

		u[utf8_state] = 0;
		utf8_state = 0;

		if (c == 0x1B) {
			u[0] = 0xC2, u[1] = 0xB7, u[2] = 0x00;

		} else if (c >= 0 && c <= 0x1F) {
			buffer[--i] = (char)(u[0] = 0x00);
			continue;
		}

		fputs((const char *)&u[0], stdout);
		column += 1;
	}

	buffer[i] = '\0';
	dsh_restore_termios();

	return buffer;
}
