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
 * libc/stdio/fopen.c
 *      Open a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

FILE *fopen(const char *path, const char *mode)
{
	FILE *r = NULL;
	int o_flags = -1;
	mode_t o_mode = 0;
	int i, c, fd;

	if (mode == NULL)
		return (errno = EINVAL), NULL;

	c = (int)((unsigned char)mode[0]);

	switch (c) {
		case 'r':
			o_flags = O_RDONLY;
			break;
		case 'w':
			o_flags = O_WRONLY;
			o_flags |= O_CREAT;
			o_flags |= O_TRUNC;
			break;
		case 'a':
			o_flags = O_WRONLY;
			o_flags |= O_CREAT;
			o_flags |= O_APPEND;
			break;
		default:
			return (errno = EINVAL), NULL;
	}

	for (i = 1; /* void */; i++) {
		if (i > 4)
			return (errno = EINVAL), NULL;

		if ((c = (int)((unsigned char)mode[i])) == '\0')
			break;

		switch (c) {
			case '+':
				o_flags &= (~(O_ACCMODE));
				o_flags |= O_RDWR;
				break;
			case 'b':
				break;
			case 'x':
				o_flags |= O_EXCL;
				break;
			default:
				return (errno = EINVAL), NULL;
		}
	}

	if ((fd = open(path, o_flags, o_mode)) < 0)
		return NULL;

	if ((r = malloc(sizeof(FILE))) == NULL)
		return close(fd), (errno = ENOMEM), NULL;

	memset(r, 0, sizeof(FILE));

	if (mtx_init(&r->__mtx, mtx_plain) != thrd_success)
		return free(r), close(fd), (errno = ENOMEM), NULL;

	if ((r->__buffer = malloc(BUFSIZ)) == NULL) {
		mtx_destroy(&r->__mtx);
		return free(r), close(fd), (errno = ENOMEM), NULL;
	}

	memset(r->__buffer, 0, BUFSIZ);

	r->__fd = fd;
	r->__i = -1;

	r->__mode = (unsigned int)(o_flags & O_ACCMODE);
	r->__state = _IOLBF;
	r->__state |= __DANCY_FILE_MALLOC_STRUCT;

	r->__buffer_size = BUFSIZ;
	r->__name = NULL;

	if (mtx_lock(&__dancy_io_array_mtx) != thrd_success) {
		free(r->__name), free(r->__buffer);
		mtx_destroy(&r->__mtx);
		return free(r), close(fd), (errno = EMFILE), NULL;
	}

	for (i = 0; i < FOPEN_MAX; i++) {
		if (__dancy_io_array[i] == NULL) {
			r->__i = i, __dancy_io_array[i] = r;
			break;
		}
	}

	mtx_unlock(&__dancy_io_array_mtx);

	if (r->__i < 0) {
		free(r->__name), free(r->__buffer);
		mtx_destroy(&r->__mtx);
		return free(r), close(fd), (errno = EMFILE), NULL;
	}

	return r;
}
