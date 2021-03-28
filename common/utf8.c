/*
 * Copyright (c) 2021 Antti Tiihala
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
 * utf8.c
 *      Unicode Transformation Format (8-bit).
 */

#include <limits.h>

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

int utf8_decode(int *state, unsigned char c);
int utf8_encode(unsigned int unicode, unsigned char *out);

#define UTF8_WAITING_NEXT  (-1)
#define UTF8_ERROR_CONSUME (-2)
#define UTF8_ERROR_RETRY   (-3)

static const signed char utf8_table[32] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, -1
};

static const int utf8_mask_table[4] = {
	0x00, 0x1F, 0x0F, 0x07
};

static const int utf8_overlong_table[4] = {
	0x00, 0x80, 0x800, 0x10000
};

int utf8_decode(int *state, unsigned char c)
{
	int offset, val;

	if ((val = *state) == 0) {
		if (c < 0x80)
			return (int)c;

		offset = (int)(c >> 3);
		offset = (int)utf8_table[offset];

		if (offset <= 1)
			return UTF8_ERROR_CONSUME;

		val = (int)c & utf8_mask_table[--offset];

		*state = (val << 4) | (offset << 2) | offset;
		return UTF8_WAITING_NEXT;
	}

	offset = (int)(c >> 3);
	offset = (int)utf8_table[offset];
	*state = 0;

	if (offset != 1)
		return UTF8_ERROR_RETRY;

	offset = val & 0x0F;
	val = ((val & 0x7FFF0) << 2) | ((int)c & 0x3F);

	if ((offset & 0x03) > 1) {
		*state = (val << 4) | (offset - 1);
		return UTF8_WAITING_NEXT;
	}

	if (val < utf8_overlong_table[offset >> 2])
		return UTF8_ERROR_CONSUME;

	if ((val >= 0xD800 && val <= 0xDFFF) || val > 0x10FFFF)
		return UTF8_ERROR_CONSUME;

	return val;
}

int utf8_encode(unsigned int unicode, unsigned char *out)
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
