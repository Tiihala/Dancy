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
 * libc/stdio/freopen.c
 *      Open a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

static FILE *my_reopen(const char *path,
	int o_flags, mode_t o_mode, FILE *stream)
{
	int fd;

	__dancy_internal_fflush(stream);

	/*
	 * "It is implementation-defined which changes of mode are
	 * permitted (if any), and under what circumstances."
	 *
	 * This implementation does not support those changes.
	 */
	if (path == NULL)
		return (errno = ENOSYS), NULL;

	fd = stream->_fd;
	stream->_fd = -1;

	(void)close(fd);

	stream->_error = 0;
	stream->_eof = 0;
	stream->_ungetc = 0;
	stream->_mode = 0;

	stream->_state &= ~__DANCY_FILE_WRITTEN_BYTES;
	stream->_buffer_start = 0;
	stream->_buffer_end = 0;

	if ((fd = open(path, o_flags, o_mode)) < 0)
		return NULL;

	stream->_fd = fd;
	stream->_mode = (unsigned int)(o_flags & O_ACCMODE);

	return stream;
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
	FILE *r = NULL;
	int o_flags = -1;
	mode_t o_mode = 0;
	int i, c;

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

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), NULL;
	}

	r = my_reopen(path, o_flags, o_mode, stream);
	mtx_unlock(&stream->_mtx);

	return r;
}
