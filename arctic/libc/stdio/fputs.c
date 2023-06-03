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
 * libc/stdio/fputs.c
 *      Put a string on a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

static int my_fputs(const char *s, FILE *stream)
{
	int buffer_mode = stream->__state & 0xFF;
	size_t size = strlen(s);

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
		int buffer_size = (int)stream->__buffer_size;
		unsigned char *p = stream->__buffer;
		size_t put_size = 0;
		char c;

		while (put_size < size) {
			if (stream->__buffer_end >= buffer_size) {
				if ( __dancy_internal_fflush(stream)) {
					stream->__error = 1;
					return EOF;
				}
			}

			c = s[put_size++];
			p[stream->__buffer_end++] = (unsigned char)c;
			stream->__state |= __DANCY_FILE_WRITTEN_BYTES;

			if (buffer_mode == _IOLBF && c == 0x0A) {
				if ( __dancy_internal_fflush(stream)) {
					stream->__error = 1;
					return EOF;
				}
			}
		}

		return 0;
	}

	if (buffer_mode == _IONBF) {
		ssize_t w = 0;

		if (size > 0)
			w = write(stream->__fd, s, size);

		if (w < 0 || (size_t)w < size) {
			stream->__error = 1;
			return EOF;
		}

		return 0;
	}

	stream->__error = 1;
	return (errno = EBADF), EOF;
}

int fputs(const char *s, FILE *stream)
{
	int r;

	if (mtx_lock(&stream->__mtx) != thrd_success) {
		stream->__error = 1;
		return (errno = EBADF), EOF;
	}

	r = my_fputs(s, stream);
	mtx_unlock(&stream->__mtx);

	return r;
}
