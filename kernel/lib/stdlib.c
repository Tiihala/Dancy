/*
 * Copyright (c) 2019 Antti Tiihala
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
 * lib/stdlib.c
 *      C standard library functions
 */

#include <dancy.h>

void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *))
{
	/*
	 * Insertion sort is a simple sorting algorithm [...] It is
	 * much less efficient on large lists than more advanced
	 * algorithms [...] However, insertion sort provides several
	 * advantages:
	 *
	 *     Simple implementation
	 *     Efficient for (quite) small data sets
	 *     ...
	 *                                           - Wikipedia
	 */

	unsigned char *array_base = base;
	size_t i, j, k;

	if (size == 0)
		return;

	for (i = 1; i < nmemb; i++) {
		for (j = i; j > 0; j--) {
			unsigned char *t1 = array_base + (j - 1) * size;
			unsigned char *t2 = array_base + (j - 0) * size;

			if (compar(t1, t2) <= 0)
				break;
			for (k = 0; k < size; k++) {
				unsigned char t3 = t1[k];
				t1[k] = t2[k];
				t2[k] = t3;
			}
		}
	}
}
