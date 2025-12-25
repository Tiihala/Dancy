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
 * libc/stdio/asprintf.c
 *      Write the output string to dynamically allocated memory
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int asprintf(char **ptr, const char *format, ...)
{
	int ret;

	va_list va;
	va_start(va, format);

	ret = vasprintf(ptr, format, va);
	va_end(va);

	return ret;
}

int vasprintf(char **ptr, const char *format, va_list arg)
{
	size_t size = 256;

	while (size < 0x10000000) {
		char *buffer = malloc(size);

		if (buffer == NULL)
			break;

		buffer[0] = '\0';

		{
			size_t n = size - 16;
			int ret = vsnprintf(buffer, n, format, arg);

			if (ret >= 0 && (size_t)ret < n)
				return (*ptr = buffer), ret;
		}

		free(buffer);
		size <<= 1;
	}

	errno = ENOMEM;
	*ptr = NULL;

	return -1;
}
