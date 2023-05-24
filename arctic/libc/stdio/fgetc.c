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
 * libc/stdio/fgetc.c
 *      Get a byte from a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static int internal_fgetc(FILE *stream)
{
	int buffer_mode = stream->__state & 0xFF;

	if (stream->__mode == O_WRONLY) {
		stream->__error = 1;
		return (errno = EBADF), EOF;
	}

	if (stream->__buffer_start != 0 || stream->__buffer_end != 0) {
		if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			stream->__error = 1;
			return (errno = EBADF), EOF;
		}
	}

	if (stream->__ungetc) {
		int c = (int)(stream->__ungetc & 0xFF);
		stream->__ungetc = 0;
		return c;
	}

	if (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		if (stream->__buffer_start >= stream->__buffer_end) {
			size_t size = stream->__buffer_size;
			ssize_t w;

			if (size == 0) {
				stream->__error = 1;
				return (errno = EBADF), EOF;
			}

			w = read(stream->__fd, stream->__buffer, size);

			if (w < 0) {
				stream->__error = 1;
				return EOF;
			}

			if (w == 0) {
				stream->__eof = 1;
				return EOF;
			}

			stream->__state &= ~__DANCY_FILE_WRITTEN_BYTES;
			stream->__buffer_start = 0;
			stream->__buffer_end = (int)w;
		}

		return (int)stream->__buffer[stream->__buffer_start++];
	}

	if (buffer_mode == _IONBF) {
		size_t size = 1;
		unsigned char buffer[1];
		ssize_t w;

		w = read(stream->__fd, &buffer, size);

		if (w < 0) {
			stream->__error = 1;
			return EOF;
		}

		if (w == 0) {
			stream->__eof = 1;
			return EOF;
		}

		return (int)buffer[0];
	}

	stream->__error = 1;
	return (errno = EBADF), EOF;
}

int fgetc(FILE *stream)
{
	int r;

	if (mtx_lock(&stream->__mtx) != thrd_success) {
		stream->__error = 1;
		return (errno = EBADF), EOF;
	}

	r = internal_fgetc(stream);
	mtx_unlock(&stream->__mtx);

	return r;
}
