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
 * libc/string/strtok.c
 *      Split string into tokens
 */

#include <string.h>

char *strtok(char *s1, const char *s2)
{
	static char *state = NULL;
	static size_t size = 0;

	char *r;

	if (s1 == NULL) {
		if (state == NULL)
			return NULL;
	} else {
		state = s1, size = 0;
	}

	r = &state[size];

	while (s2 != NULL) {
		size_t i = 0;
		int c1 = (int)state[size];
		int c2 = (int)s2[i];

		if (c1 == '\0') {
			if (r == &state[size])
				r = NULL;
			state = NULL, size = 0;
			return r;
		}

		if (c2 == '\0') {
			state = NULL, size = 0;
			return r;
		}

		do {
			if (c1 == c2) {
				if (r == &state[size]) {
					r = &state[size + 1];
					break;
				}
				state[size++] = '\0';
				return r;
			}

		} while ((c2 = (int)s2[++i]) != '\0');

		size += 1;
	}

	return NULL;
}
