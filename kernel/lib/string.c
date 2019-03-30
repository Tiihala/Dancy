/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
 * lib/string.c
 *      C standard library functions
 */

#include <dancy.h>

char *strcat(char *s1, const char *s2)
{
	char *r = s1;

	for (;;) {
		if (*s1 == '\0')
			break;
		s1++;
	}
	for (;;) {
		if ((*s1 = *s2) == '\0')
			break;
		s1++, s2++;
	}
	return r;
}

int strcmp(const char *s1, const char *s2)
{
	unsigned char c1, c2;

	for (;;) {
		c1 = *((const unsigned char *)s1++);
		c2 = *((const unsigned char *)s2++);
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

char *strcpy(char *s1, const char *s2)
{
	char *r = s1;

	for (;;) {
		if ((*s1 = *s2) == '\0')
			break;
		s1++, s2++;
	}
	return r;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	unsigned char c1, c2;

	while (n != 0) {
		c1 = *((const unsigned char *)s1++);
		c2 = *((const unsigned char *)s2++);
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

char *strncpy(char *s1, const char *s2, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		if ((s1[i] = *s2) != '\0')
			s2++;
	}
	return s1;
}
