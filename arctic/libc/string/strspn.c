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
 * libc/string/strspn.c
 *      The C Standard Library
 */

#include <string.h>

size_t strspn(const char *s1, const char *s2)
{
	size_t r = 0;
	size_t i, j;

	for (i = 0; s1[i] != '\0'; i++) {
		char c = s1[i];
		size_t add = 0;

		for (j = 0; s2[j] != '\0'; j++) {
			if (c == s2[j]) {
				add = 1;
				break;
			}
		}

		if (add == 0)
			break;

		r += add;
	}

	return r;
}
