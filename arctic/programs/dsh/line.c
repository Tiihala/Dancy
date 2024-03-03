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

char *dsh_get_input(const char *prompt, size_t offset)
{
	const size_t size = 0x2000;
	char *buffer = malloc(size);

	size_t column = offset;
	size_t i = 0;

	int utf8_state = 0;
	unsigned char u[8];

	if (!buffer)
		return NULL;

	fputs(prompt, stdout);

	dsh_save_termios();
	get_winsize();

	while (i < size - 2) {
		int c = getchar();

		if (c == EOF || c == '\r' || c == '\n' || (char)c == '\0')
			break;

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

		if (c >= 0 && c <= 0x1F)
			u[0] = 0xEF, u[1] = 0xB7, u[2] = 0xBF, u[3] = 0x00;

		fputs((const char *)&u[0], stdout);
		column += 1;
	}

	buffer[i] = '\0';
	dsh_restore_termios();

	return buffer;
}
