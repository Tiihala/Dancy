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
 * libc/stdio/ftello.c
 *      Return a file offset in a stream
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static off_t my_ftello(FILE *stream)
{
	off_t offset = lseek(stream->__fd, 0, SEEK_CUR);

	if (offset < 0)
		return -1;

	{
		int count = (stream->__buffer_end - stream->__buffer_start);
		int extra = (stream->__ungetc != 0) ? 1 : 0;

		if (count < 0)
			return (errno = EBADF), -1;

		if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			off_t limit = (off_t)(LLONG_MAX);

			if (offset > (limit - (off_t)count))
				return (errno = EOVERFLOW), -1;

			count = -count;
			extra = 0;
		}

		offset -= ((off_t)(count + extra));
	}

	if (offset < 0)
		return 0;

	return offset;
}

off_t ftello(FILE *stream)
{
	off_t r;

	if (mtx_lock(&stream->__mtx) != thrd_success)
		return (errno = EBADF), -1;

	r = my_ftello(stream);
	mtx_unlock(&stream->__mtx);

	return r;
}
