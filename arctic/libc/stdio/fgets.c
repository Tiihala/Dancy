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
 * libc/stdio/fgets.c
 *      Get a string from a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static int internal_fgetc(FILE *stream)
{
	int buffer_mode = stream->_state & 0xFF;

	if (stream->_mode == O_WRONLY) {
		stream->_error = 1;
		return (errno = EBADF), EOF;
	}

	if (stream->_buffer_start != 0 || stream->_buffer_end != 0) {
		if ((stream->_state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			stream->_error = 1;
			return (errno = EBADF), EOF;
		}
	}

	if (stream->_ungetc) {
		int c = (int)(stream->_ungetc & 0xFF);
		stream->_ungetc = 0;
		return c;
	}

	if (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		if (stream->_buffer_start >= stream->_buffer_end) {
			size_t size = stream->_buffer_size;
			ssize_t w;

			if (size == 0) {
				stream->_error = 1;
				return (errno = EBADF), EOF;
			}

			w = read(stream->_fd, stream->_buffer, size);

			if (w < 0) {
				stream->_error = 1;
				return EOF;
			}

			if (w == 0) {
				stream->_eof = 1;
				return EOF;
			}

			stream->_state &= ~__DANCY_FILE_WRITTEN_BYTES;
			stream->_buffer_start = 0;
			stream->_buffer_end = (int)w;
		}

		return (int)stream->_buffer[stream->_buffer_start++];
	}

	if (buffer_mode == _IONBF) {
		size_t size = 1;
		unsigned char buffer[1];
		ssize_t w;

		w = read(stream->_fd, &buffer, size);

		if (w < 0) {
			stream->_error = 1;
			return EOF;
		}

		if (w == 0) {
			stream->_eof = 1;
			return EOF;
		}

		return (int)buffer[0];
	}

	stream->_error = 1;
	return (errno = EBADF), EOF;
}

char *fgets(char *s, int n, FILE *stream)
{
	int i = 0, c = 0;

	if (s == NULL || n < 1)
		return (errno = EINVAL), NULL;

	if (n == 1)
		return (s[0] = '\0'), s;

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), NULL;
	}

	while (i < (n - 1)) {
		if ((c = internal_fgetc(stream)) == EOF)
			break;

		s[i++] = (char)c;

		if (c == '\n')
			break;
	}

	if (i == 0)
		s = NULL;

	if (c == EOF && stream->_eof == 0)
		s = NULL;

	mtx_unlock(&stream->_mtx);

	if (s != NULL)
		s[i] = '\0';

	return s;
}
