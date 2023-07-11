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
 * libc/stdlib/strtoul.c
 *      The C Standard Library
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

unsigned long strtoul(const char *str, char **endptr, int base)
{
	unsigned long m = ULONG_MAX;
	unsigned long long r = 0;
	char *end = NULL;
	int sign = 0;
	int r_errno;

	if (endptr != NULL)
		*endptr = (char *)str;

	while (isspace((int)(*str)))
		str += 1;

	if (*str == '+' || *str == '-')
		sign = (int)(*str++);

	r_errno = __dancy_ull(str, &end, base, &r);

	if (r_errno == EINVAL)
		return (errno = r_errno), 0;

	if (endptr != NULL)
		*endptr = end;

	if (r_errno == ERANGE || r > m)
		return (errno = ERANGE), m;

	if (sign == '-')
		r = ((unsigned long long)m - r) + 1ULL;

	return (unsigned long)r;
}
