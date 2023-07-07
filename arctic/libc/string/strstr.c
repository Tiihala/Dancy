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
 * libc/string/strstr.c
 *      The C Standard Library
 */

#include <string.h>

char *strstr(const char *s1, const char *s2)
{
	char *r = NULL;

	if (*s2 == '\0')
		return (char *)s1;

	while (*s1 != '\0' && r == NULL) {
		size_t i;

		if (s1[0] != s2[0]) {
			s1 += 1;
			continue;
		}

		for (i = 0, r = (char *)s1; s2[i] != '\0'; i++) {
			if (s1[i] == '\0' || s1[i] != s2[i]) {
				r = NULL;
				break;
			}
		}

		s1 += 1;
	}

	return r;
}
