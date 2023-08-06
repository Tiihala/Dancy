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
 * libc/stdio/ftell.c
 *      Return a file offset in a stream
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static long int my_ftell(FILE *stream)
{
	off_t offset = lseek(stream->_fd, 0, SEEK_CUR);

	if (offset < 0)
		return -1;

	{
		int count = (stream->_buffer_end - stream->_buffer_start);
		int extra = (stream->_ungetc != 0) ? 1 : 0;

		if (count < 0)
			return (errno = EBADF), -1;

		if ((stream->_state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			off_t limit = (off_t)(LONG_MAX);

			if (offset > (limit - (off_t)count))
				return (errno = EOVERFLOW), -1;

			count = -count;
			extra = 0;
		}

		offset -= ((off_t)(count + extra));
	}

	if (offset < 0)
		return 0;

	if (offset > (off_t)(LONG_MAX))
		return (errno = EOVERFLOW), -1;

	return (long int)offset;
}

long int ftell(FILE *stream)
{
	int saved_errno = errno;
	long int r;

	if (mtx_lock(&stream->_mtx) != thrd_success)
		return (errno = EBADF), -1;

	r = my_ftell(stream);
	mtx_unlock(&stream->_mtx);

	if (r >= 0 && errno != saved_errno)
		errno = saved_errno;

	return r;
}
