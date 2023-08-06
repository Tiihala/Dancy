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
 * libc/stdio/fseek.c
 *      Reposition a file-position indicator in a stream
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static int reposition(FILE *stream, long offset, int whence)
{
	if ( __dancy_internal_fflush(stream))
		return -1;

	if (lseek(stream->_fd, (off_t)offset, whence) < 0)
		return -1;

	stream->_eof = 0;
	stream->_ungetc = 0;
	stream->_state &= ~__DANCY_FILE_WRITTEN_BYTES;
	stream->_buffer_start = 0;
	stream->_buffer_end = 0;

	return 0;
}

static int my_fseek(FILE *stream, long offset, int whence)
{
	if (whence == SEEK_SET) {
		if (offset < 0)
			return (errno = EINVAL), -1;

		return reposition(stream, offset, whence);
	}

	if (whence == SEEK_CUR) {
		int count = (stream->_buffer_end - stream->_buffer_start);
		int extra = (stream->_ungetc != 0) ? 1 : 0;
		long sub;

		if (count < 0)
			return (errno = EBADF), -1;

		sub = (long)(count + extra);

		if (offset < LONG_MIN + sub)
			return (errno = EOVERFLOW), -1;

		return reposition(stream, (offset - sub), whence);
	}

	if (whence == SEEK_END)
		return reposition(stream, offset, whence);

	return (errno = EINVAL), -1;
}

int fseek(FILE *stream, long offset, int whence)
{
	int r;

	if (mtx_lock(&stream->_mtx) != thrd_success)
		return (errno = EBADF), -1;

	r = my_fseek(stream, offset, whence);
	mtx_unlock(&stream->_mtx);

	return r;
}
