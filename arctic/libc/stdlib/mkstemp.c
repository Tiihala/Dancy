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
 * libc/stdlib/mkstemp.c
 *      Create a unique temporary file
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mkstemp(char *template)
{
	size_t t = strlen(template);
	int i, e = errno;

	const char *x6 = "XXXXXX";
	char *p;

	if (t < 6 || strcmp((p = &template[t - 6]), x6))
		return (errno = EINVAL), -1;

	for (i = 0; i < 10000; i++) {
		int r = snprintf(p, 7, "99%04d", i);
		int fd, flags = O_RDWR | O_CREAT | O_EXCL;

		if (r != 6)
			return strcpy(p, x6), (errno = ENOTSUP), -1;

		if ((fd = open(template, flags, 0600)) >= 0)
			return (errno = e), fd;
	}

	return strcpy(p, x6), (errno = ENOSPC), -1;
}
