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
 * libc/libgen/basename.c
 *      Return the last component of a pathname
 */

#include <libgen.h>
#include <string.h>

char *basename(char *path)
{
	static char b[4];
	size_t i;

	if (path == NULL || !strcmp(path, ""))
		return strcpy(&b[0], ".");

	i = strlen(path);

	while (i > 1 && path[i - 1] == '/')
		path[--i] = '\0';

	if (!strcmp(path, "/"))
		return path;

	while (i > 0) {
		if (path[--i] == '/')
			return &path[i + 1];
	}

	return path;
}
