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
 * libc/stdlib/getenv.c
 *      The C Standard Library
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

char *getenv(const char *name)
{
	size_t length = 0;
	size_t i = 0;
	char c;

	while ((c = name[i++]) != '\0') {
		if (c == '=')
			return NULL;
		length += 1;
	}

	for (i = 0; environ[i] != NULL; i++) {
		char *e = environ[i];

		if (strncmp(e, name, length))
			continue;

		if (e[length] == '=')
			return &e[length + 1];
	}

	return NULL;
}
