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
 * libc/stdio/sscanf.c
 *      Convert formatted input
 */

#include <stdio.h>

struct state {
	const char *input;
	size_t offset;
};

static int get(void *stream)
{
	struct state *st = stream;

	if (st->input[st->offset] == '\0')
		return EOF;

	return (int)((unsigned char)st->input[st->offset++]);
}

static int unget(int c, void *stream)
{
	struct state *st = stream;

	if (st->offset == 0)
		return EOF;

	if ((int)((unsigned char)st->input[st->offset - 1]) != c)
		return EOF;

	return (int)((unsigned char)st->input[--st->offset]);
}

int sscanf(const char *s, const char *format, ...)
{
	struct state st;
	int r;

	va_list va;
	va_start(va, format);

	st.input = s;
	st.offset = 0;

	r = __dancy_scanf(get, unget, &st, format, va);

	va_end(va);

	return r;
}
