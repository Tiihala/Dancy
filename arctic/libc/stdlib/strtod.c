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
 * libc/stdlib/strtod.c
 *      The C Standard Library
 */

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

double strtod(const char *str, char **endptr)
{
	int c, e = errno;
	const char *p, *s = str;
	int sign = '+';

	long double base = 10.0;
	long double r = 0.0;

	if (endptr != NULL)
		*endptr = (char *)str;

	while (isspace((int)(*s)))
		s += 1;

	if (s[0] == '+' || s[0] == '-')
		sign = (int)(*s++);

	/*
	 * Detect the infinity case.
	 */
	{
		const char *infinity = "INFINITY";

		if (!strncasecmp(s, &infinity[0], 3)) {
			s += (!strncasecmp(s, &infinity[3], 5) ? 8 : 3);

			r = (DBL_MAX);
			r *= (sign == '-' ? -2.0 : 2.0);

			if (endptr != NULL)
				*endptr = (char *)s;

			return (errno = e), (double)r;
		}
	}

	/*
	 * Detect the not-a-number case.
	 */
	{
		const char *nan = "NAN";

		if (!strncasecmp(s, &nan[0], 3)) {
			s += 3;

			if (s[0] == '(' && (p = strchr(&s[1], ')')) != NULL)
				s += ((p + 1) - s);

			r = ((0.0 / 0.0) * -1.0);
			r *= (sign == '-' ? -1.0 : 1.0);

			if (endptr != NULL)
				*endptr = (char *)s;

			return (errno = e), (double)r;
		}
	}

	/*
	 * Detect the unknown case.
	 */
	if (s[0] != '.' && !isdigit((int)(((unsigned char *)s)[0])))
		return (errno = ERANGE), (double)r;

	/*
	 * Detect the hexadecimal case.
	 */
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		if (s[2] == '.' || isxdigit((int)(((unsigned char *)s)[2]))) {
			base = 16.0;
			s += 2;
		}
	}

	/*
	 * This is a simplified implementation.
	 */
	{
		const char *array = "0123456789ABCDEF";
		size_t n = (size_t)((base == 10.0) ? 10 : 16);

		for (;;) {
			c = toupper((int)(((unsigned char *)s)[0]));

			if ((p = memchr(&array[0], c, n)) == NULL)
				break;

			r *= base;
			r += (long double)((int)(p - &array[0]));

			s += 1;
		}

		if (s[0] == '.') {
			long double d[2] = { 0.0, 1.0 };

			s += 1;

			for (;;) {
				c = toupper((int)(((unsigned char *)s)[0]));

				if ((p = memchr(&array[0], c, n)) == NULL)
					break;

				d[0] *= base;
				d[0] += (long double)((int)(p - &array[0]));
				d[1] *= base;

				s += 1;
			}

			r += (d[0] / d[1]);
		}

		if (n == 10 && (s[0] == 'e' || s[0] == 'E')) {
			int e_sign = '+';
			long double d[2] = { 0.0, 1.0 };

			s += 1;

			if (s[0] == '+' || s[0] == '-')
				e_sign = (int)(*s++);

			for (;;) {
				c = (int)(((unsigned char *)s)[0]);

				if ((p = memchr(&array[0], c, 10)) == NULL)
					break;

				d[0] *= 10.0;
				d[0] += (long double)((int)(p - &array[0]));

				s += 1;
			}

			while (d[0] > 0.0) {
				d[0] -= 1.0;
				d[1] *= 10.0;
			}

			if (e_sign == '+')
				r *= d[1];
			else
				r /= d[1];
		}

		if (n == 16 && (s[0] == 'p' || s[0] == 'P')) {
			int e_sign = '+';
			long double d[2] = { 0.0, 1.0 };

			s += 1;

			if (s[0] == '+' || s[0] == '-')
				e_sign = (int)(*s++);

			for (;;) {
				c = (int)(((unsigned char *)s)[0]);

				if ((p = memchr(&array[0], c, 10)) == NULL)
					break;

				d[0] *= 10.0;
				d[0] += (long double)((int)(p - &array[0]));

				s += 1;
			}

			while (d[0] > 0.0) {
				d[0] -= 1.0;
				d[1] *= 2.0;
			}

			if (e_sign == '+')
				r *= d[1];
			else
				r /= d[1];
		}
	}

	if (sign == '-')
		r *= -1.0;

	if (endptr != NULL)
		*endptr = (char *)s;

	return (errno = e), (double)r;
}
