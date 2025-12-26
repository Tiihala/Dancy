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
 * libc/wchar/mbrtowc.c
 *      Convert from UTF-8 to wchar_t
 */

#include <errno.h>
#include <wchar.h>

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

static int utf8_decode(int *state, unsigned char c)
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

size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
	static mbstate_t internal_state;
	size_t i;

	if (s == NULL)
		return 0;

	if (ps == NULL)
		ps = &internal_state;

	for (i = 0; i < n; i++) {
		int *state = (void *)(&ps->_state[0]);
		int u = utf8_decode(state, ((const unsigned char *)s)[i]);

		if (u >= 0) {
			if (u >= 0x10000)
				u = 0xFFFD;

			if (pwc != NULL)
				*pwc = (wchar_t)u;

			if (u == 0)
				return 0;

			return i + 1;
		}

		if (u != UTF8_WAITING_NEXT)
			return (errno = EILSEQ), (size_t)(-1);
	}

	return (size_t)(-2);
}
