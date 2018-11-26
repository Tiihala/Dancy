/*
 * Copyright (c) 2018 Antti Tiihala
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
 * init/init.c
 *      Initialization of Dancy Operating System
 */

#include <dancy.h>

static void sleep(int seconds)
{
	int i;

	/*
	 * This "sleep" function is not accurate. The pause
	 * function waits approximately 500 milliseconds.
	 */
	for (i = 0; i < (seconds * 2); i++)
		b_pause();
}

void init(void *map)
{
	(void)map;

	b_output_string("hello, world\r\n", 0);

	for (;;) {
		const char *cursor = "\r ";
		b_output_string_hl(cursor, 0);
		sleep(1);
		b_output_string(cursor, 0);
		sleep(1);
	}
}
