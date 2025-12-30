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
 * libc/stdio/puts.c
 *      Put a string and newline on a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

static int my_puts(const char *s, FILE *stream)
{
	int buffer_mode = stream->_state & 0xFF;
	size_t size = strlen(s);

	if (stream->_mode == O_RDONLY) {
		stream->_error = 1;
		return (errno = EBADF), EOF;
	}

	if (stream->_buffer_start != 0 || stream->_buffer_end != 0) {
		if ((stream->_state & __DANCY_FILE_WRITTEN_BYTES) == 0) {
			stream->_error = 1;
			return (errno = EBADF), EOF;
		}
	}

	if (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		int buffer_size = (int)stream->_buffer_size;
		unsigned char *p = stream->_buffer;
		size_t put_size = 0;
		char c;

		if (size == __DANCY_SIZE_MAX)
			return (errno = EOVERFLOW), EOF;

		while (put_size <= size) {
			if (stream->_buffer_end >= buffer_size) {
				stream->_error = 1;
				return (errno = EBADF), EOF;
			}

			if (put_size < size)
				c = s[put_size++];
			else
				c = 0x0A, put_size++;

			p[stream->_buffer_end++] = (unsigned char)c;
			stream->_state |= __DANCY_FILE_WRITTEN_BYTES;

			if (buffer_mode == _IOLBF && c == 0x0A) {
				if ( __dancy_internal_fflush(stream)) {
					stream->_error = 1;
					return EOF;
				}
			}

			if (stream->_buffer_end >= buffer_size) {
				if ( __dancy_internal_fflush(stream)) {
					stream->_error = 1;
					return EOF;
				}
			}
		}

		return 0;
	}

	if (buffer_mode == _IONBF) {
		unsigned char newline[1] = { 0x0A };
		ssize_t w = 0;

		if (size > 0)
			w = write(stream->_fd, s, size);

		if (w < 0 || (size_t)w < size) {
			stream->_error = 1;
			return EOF;
		}

		w = write(stream->_fd, &newline[0], sizeof(newline));

		if (w < 0 || (size_t)w < sizeof(newline)) {
			stream->_error = 1;
			return EOF;
		}

		return 0;
	}

	stream->_error = 1;
	return (errno = EBADF), EOF;
}

int puts(const char *s)
{
	FILE *stream = stdout;
	int r;

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), EOF;
	}

	r = my_puts(s, stream);
	mtx_unlock(&stream->_mtx);

	return r;
}
