/*
 * Copyright (c) 2025 Antti Tiihala
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
 * libc/wchar/wcrtomb.c
 *      Convert from wchar_t to UTF-8
 */

#include <errno.h>
#include <wchar.h>

static int utf8_encode(unsigned int unicode, unsigned char *out)
{
	if (unicode <= 0x7F) {
		out[0] = (unsigned char)unicode;
		return 1;
	}

	if (unicode <= 0x07FF) {
		out[0] = (unsigned char)(0xC0 + (unicode >> 6));
		out[1] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 2;
	}

	if ((unicode >= 0xD800 && unicode <= 0xDFFF))
		return 0;

	if (unicode <= 0xFFFF) {
		out[0] = (unsigned char)(0xE0 + (unicode >> 12));
		out[1] = (unsigned char)(0x80 + ((unicode >> 6) & 0x3F));
		out[2] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 3;
	}

	if (unicode <= 0x10FFFF) {
		out[0] = (unsigned char)(0xF0 + (unicode >> 18));
		out[1] = (unsigned char)(0x80 + ((unicode >> 12) & 0x3F));
		out[2] = (unsigned char)(0x80 + ((unicode >> 6) & 0x3F));
		out[3] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 4;
	}

	return 0;
}

size_t wcrtomb(char *s, wchar_t w, mbstate_t *ps)
{
	int r;

	if (s == NULL)
		return 1;

	r = utf8_encode((unsigned int)w, (void *)s);

	if (r < 1)
		return (errno = EILSEQ), (size_t)(-1);

	return (void)ps, (size_t)r;
}
