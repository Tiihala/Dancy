/*
 * Copyright (c) 2026 Antti Tiihala
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
 * libc/stdlib/wcstombs.c
 *      The C Standard Library
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

size_t wcstombs(char *dest, const wchar_t *str, size_t size)
{
	size_t i = 0;

	while (i < size || dest == NULL) {
		char b[8];
		mbstate_t s;
		size_t j, r;

		memset(&b[0], 0, sizeof(b));
		memset(&s, 0, sizeof(s));

		r = wcrtomb(&b[0], *str, &s);

		if (r == 0 || r > 4)
			return (errno = EILSEQ), (size_t)(-1);

		for (j = 0; j < r; j++) {
			char c = b[j];

			if (dest != NULL)
				dest[i] = c;

			if (c == '\0')
				return i;

			if (++i >= size && dest != NULL)
				return i;
		}

		str += 1;
	}

	return i;
}
