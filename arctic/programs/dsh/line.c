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

int dsh_get_input(char *buffer, size_t size)
{
	size_t i = 0;

	if (size < 2)
		return EXIT_FAILURE;

	buffer[0] = '\0';

	dsh_save_termios();

	while (i < size - 2) {
		int c = getchar();
		char b[4];

		if (c == EOF || c == '\r' || c == '\n' || (char)c == '\0')
			break;

		if (c == 0x7F) {
			if (i > 0) {
				c = (int)buffer[--i];
				buffer[i] = '\0';

				if (c >= 0 && c <= 0x1F)
					fputs("\x08\x20\x08", stdout);
				fputs("\x08\x20\x08", stdout);

				while (i > 0 && (c & 0xC0) == 0x80) {
					c = (int)buffer[--i];
					buffer[i] = '\0';
				}
			}
			continue;
		}

		buffer[i++] = (char)c;

		if (c >= 0 && c <= 0x1F)
			snprintf(&b[0], sizeof(b), "^%c", '@' + c);
		else
			b[0] = (char)c, b[1] = '\0';

		fputs(&b[0], stdout);
	}

	dsh_restore_termios();

	if (buffer[0] != '\0') {
		buffer[i + 0] = '\n';
		buffer[i + 1] = '\0';
	}

	return 0;
}
