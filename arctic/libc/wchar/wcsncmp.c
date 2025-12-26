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
 * libc/wchar/wcsncmp.c
 *      Compare part of two wchar_t strings
 */

#include <wchar.h>

int wcsncmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
	wchar_t w1, w2;

	while (n != 0) {
		w1 = *((const wchar_t *)ws1++);
		w2 = *((const wchar_t *)ws2++);

		if (!--n || !w1 || w1 != w2) {
			if (!w1 && !w2)
				break;
			if (w1 < w2)
				return -1;
			if (w1 > w2)
				return 1;
		}
	}

	return 0;
}
