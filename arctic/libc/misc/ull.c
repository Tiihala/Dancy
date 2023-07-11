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
 * libc/misc/ull.c
 *      Convert string to unsigned long long
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>

int __dancy_ull(const char *str, char **end, int base, unsigned long long *r)
{
	int hex_prefix = (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'));
	unsigned long long value = 0;
	int r_errno = EINVAL;

	if (base == 0) {
		if (hex_prefix)
			base = 16;
		else if (str[0] != '0')
			base = 10;
		else
			base = 8;
	}

	if (base == 16 && hex_prefix)
		str += 2;

	while (base >= 2 && base <= 36) {
		int c = (int)(*str);
		unsigned long long u;

		if (c >= '0' && c <= '9')
			c = (c - '0') + 0;
		else if (c >= 'A' && c <= 'Z')
			c = (c - 'A') + 10;
		else if (c >= 'a' && c <= 'z')
			c = (c - 'a') + 10;
		else
			break;

		if (c >= base)
			break;

		str += 1;

		if (r_errno == ERANGE)
			continue;

		u = value * (unsigned int)base;

		if (value != (u / (unsigned int)base)) {
			value = ULLONG_MAX;
			r_errno = ERANGE;
			continue;
		}

		if (u > (ULLONG_MAX - (unsigned long long)c)) {
			value = ULLONG_MAX;
			r_errno = ERANGE;
			continue;
		}

		value = u + (unsigned long long)c;
		r_errno = 0;
	}

	if (r_errno == EINVAL && hex_prefix) {
		if (base == 16)
			str -= 1;
		r_errno = 0;
	}

	if (end != NULL)
		*end = (char *)str;

	return (*r = value), r_errno;
}
