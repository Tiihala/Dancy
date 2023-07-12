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
 * libc/stdlib/atoi.c
 *      The C Standard Library
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

int atoi(const char *str)
{
	unsigned long long r = 0;
	int sign = 0;
	int r_errno;

	while (isspace((int)(*str)))
		str += 1;

	if (*str == '+' || *str == '-')
		sign = (int)(*str++);

	r_errno = __dancy_ull(str, NULL, 10, &r);

	if (r_errno == EINVAL)
		return (errno = r_errno), 0;

	if (r_errno == ERANGE) {
		int m = ((sign == '-') ? INT_MIN : INT_MAX);
		return (errno = r_errno), m;
	}

	if (sign == '-') {
		int m = INT_MIN;

		if (r > (unsigned long long)(-(m + 1)) + 1ULL)
			return (errno = ERANGE), m;

		if (r == (unsigned long long)(-(m + 1)) + 1ULL)
			return m;

		return -((int)r);
	}

	if (r > (unsigned long long)(INT_MAX))
		return (errno = ERANGE), INT_MAX;

	return (int)r;
}
