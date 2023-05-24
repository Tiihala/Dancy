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
 * libc/stdio/fputc.c
 *      Put a byte on a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static int internal_fputc(int c, FILE *stream)
{
	int buffer_mode = stream->__state & 0xFF;

	if (stream->__mode == O_RDONLY) {
		stream->__error = 1;
		return (errno = EBADF), EOF;
	}

	if (stream->__buffer_start != 0 || stream->__buffer_end != 0) {
		if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) == 0) {
			stream->__error = 1;
			return (errno = EBADF), EOF;
		}
	}

	if (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		if (stream->__buffer_end >= (int)stream->__buffer_size) {
			stream->__error = 1;
			return (errno = EBADF), EOF;
		}

		stream->__buffer[stream->__buffer_end++] = (unsigned char)c;
		stream->__state |= __DANCY_FILE_WRITTEN_BYTES;

		if (buffer_mode == _IOLBF && c == 0x0A) {
			if ( __dancy_internal_fflush(stream))
				return EOF;
		}

		if (stream->__buffer_end >= (int)stream->__buffer_size) {
			if ( __dancy_internal_fflush(stream))
				return EOF;
		}

		return c;
	}

	if (buffer_mode == _IONBF) {
		size_t size = 1;
		unsigned char buffer[1];
		ssize_t w;

		buffer[0] = (unsigned char)c;
		w = write(stream->__fd, &buffer, size);

		if (w < 0) {
			stream->__error = 1;
			return EOF;
		}

		if (w == 0) {
			stream->__error = 1;
			return (errno = EIO), EOF;
		}

		return c;
	}

	stream->__error = 1;
	return (errno = EBADF), EOF;
}

int fputc(int c, FILE *stream)
{
	int r;

	if (mtx_lock(&stream->__mtx) != thrd_success) {
		stream->__error = 1;
		return (errno = EBADF), EOF;
	}

	r = internal_fputc(c, stream);
	mtx_unlock(&stream->__mtx);

	return r;
}
