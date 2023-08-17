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
 * libc/string/casecmp.c
 *      Case-insensitive string comparisons
 */

#include <string.h>

static int case_insensitive(int c)
{
	if (c >= 'A' && c <= 'Z')
		return ('a' + (c - 'A'));
	return c;
}

int strcasecmp(const char *s1, const char *s2)
{
	unsigned char c1, c2;

	for (;;) {
		c1 = *((const unsigned char *)s1++);
		c2 = *((const unsigned char *)s2++);

		c1 = (unsigned char)case_insensitive((int)c1);
		c2 = (unsigned char)case_insensitive((int)c2);

		if (!c1 || c1 != c2) {
			if (!c1 && !c2)
				break;
			if (c1 < c2)
				return -1;
			if (c1 > c2)
				return 1;
		}
	}

	return 0;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	unsigned char c1, c2;

	while (n != 0) {
		c1 = *((const unsigned char *)s1++);
		c2 = *((const unsigned char *)s2++);

		c1 = (unsigned char)case_insensitive((int)c1);
		c2 = (unsigned char)case_insensitive((int)c2);

		if (!--n || !c1 || c1 != c2) {
			if (!c1 && !c2)
				break;
			if (c1 < c2)
				return -1;
			if (c1 > c2)
				return 1;
		}
	}

	return 0;
}