/*
 * Copyright (c) 2024 Antti Tiihala
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
 * more/cat.c
 *      Display files on a page-by-page basis
 */

#include "main.h"

static int read_write(int fd)
{
	static unsigned char buffer[0x20000];
	ssize_t size[2];

	for (;;) {
		size[0] = read(fd, &buffer[0], sizeof(buffer));
		size[1] = 0;

		if (size[0] < 0)
			return perror("read"), EXIT_FAILURE;

		if (size[0] == 0)
			break;

		while (size[0] > size[1]) {
			size_t s = (size_t)(size[0] - size[1]);
			ssize_t w;

			w = (errno = EIO, write(1, &buffer[size[1]], s));

			if (w <= 0)
				return perror("write"), EXIT_FAILURE;

			size[1] += w;
		}
	}

	return 0;
}

int more_cat(const char *path)
{
	int fd, r;

	if (path == NULL)
		return read_write(0);

	if ((fd = open(path, O_RDONLY)) < 0)
		return perror(path), EXIT_FAILURE;

	r = read_write(fd);
	close(fd);

	return r;
}
