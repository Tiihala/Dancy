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
 * more/term.c
 *      Display files on a page-by-page basis
 */

#include "main.h"

static struct termios _default_termios;
static struct termios *default_termios;

static void set_unbuffered_mode(void)
{
	struct termios new_termios;

	memcpy(&new_termios, default_termios, sizeof(*default_termios));

	new_termios.c_lflag &= ~((tcflag_t)ECHO);
	new_termios.c_lflag &= ~((tcflag_t)ECHONL);
	new_termios.c_lflag &= ~((tcflag_t)ICANON);
	new_termios.c_lflag &= ~((tcflag_t)IEXTEN);

	if ((errno = 0, tcsetattr(1, TCSAFLUSH, &new_termios)) != 0)
		perror("tcsetattr");
}

int save_termios(void)
{
	if (default_termios)
		restore_termios();

	if ((errno = 0, tcgetattr(1, &_default_termios)) != 0) {
		if (errno == ENOTTY)
			return 0;
		return perror("tcgetattr"), EXIT_FAILURE;
	}

	default_termios = &_default_termios;
	set_unbuffered_mode();

	return 0;
}

void restore_termios(void)
{
	if (!default_termios)
		return;

	if ((errno = 0, tcsetattr(1, TCSAFLUSH, default_termios)) != 0)
		perror("tcsetattr");

	memset(default_termios, 0, sizeof(*default_termios));
	default_termios = NULL;
}
