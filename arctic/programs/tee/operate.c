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
 * tee/operate.c
 *      Duplicate standard input
 */

#include "main.h"

static int tee(int *fd_array, int fd_count)
{
	static unsigned char buffer[0x20000];
	int i , r = 0;

	for (;;) {
		ssize_t size_in = read(0, &buffer[0], sizeof(buffer));

		if (size_in < 0)
			return perror("read"), EXIT_FAILURE;

		if (size_in == 0)
			break;

		for (i = 0; i < fd_count; i++) {
			ssize_t w, size_out = 0;

			if (fd_array[i] < 0)
				continue;

			while (size_in > size_out) {
				size_t n = (size_t)(size_in - size_out);

				w = write(fd_array[i], &buffer[size_out], n);

				if (w <= 0) {
					if (i > 0) {
						close(fd_array[i]);
						fd_array[i] = -1;
					}
					r = EXIT_FAILURE;
					break;
				}

				size_out += w;
			}
		}
	}

	return r;
}

int operate(struct options *opt)
{
	int fd_count = 0, fd_array[32] = { 0 };
	int i, r = 0;

	fd_array[fd_count++] = 1;

	if (opt->ignore_sigint)
		signal(SIGINT, SIG_IGN);

	for (i = 0; opt->operands[i] != NULL; i++) {
		int fd, flags = O_WRONLY | O_CREAT | O_TRUNC;

		if (opt->append_output)
			flags = O_WRONLY | O_CREAT | O_APPEND;

		if (fd_count == 32) {
			fputs("tee: too many output files\n", stderr);
			r = EXIT_FAILURE;
			break;
		}

		if ((fd = open(opt->operands[i], flags, 0777)) < 0) {
			fprintf(stderr, "tee: \'%s\': %s\n",
				opt->operands[i], strerror(errno));
			r = EXIT_FAILURE;
		}

		fd_array[fd_count++] = fd;
	}

	r |= tee(&fd_array[0], fd_count);

	for (i = 1; i < fd_count; i++) {
		if (fd_array[i] < 0)
			continue;
		close(fd_array[i]);
	}

	return r;
}
