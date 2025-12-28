/*
 * Copyright (c) 2025 Antti Tiihala
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
 * libc/stdio/getdelim.c
 *      Read a delimited record from the stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static ssize_t locked(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
	size_t i = 0, size = *n;
	unsigned char *line = (unsigned char *)(*lineptr);
	ssize_t r = -1;

	if (line == NULL && size != 0)
		line = malloc(size);

	if (line == NULL || size == 0) {
		const size_t default_size = 128;

		if ((line = malloc(default_size)) == NULL)
			return (errno = ENOMEM), -1;

		size = default_size;
	}

	line[0] = 0;

	for (;;) {
		int c;

		if (i + 16 >= 0x10000000) {
			errno = EOVERFLOW;
			break;
		}

		if (i + 16 >= size) {
			size_t new_size = ((size + 0x1000) & 0xFFFFF000);
			void *new_line = malloc(new_size);

			if (new_line == NULL) {
				errno = ENOMEM;
				break;
			}

			if (size > 0)
				memcpy(new_line, line, size);

			line = new_line;
			size = new_size;
		}

		if ((c = internal_fgetc(stream)) == EOF)
			break;

		line[i++] = (unsigned char)c;
		line[i] = 0;

		if ((unsigned char)c == (unsigned char)delimiter) {
			r = (ssize_t)i;
			break;
		}
	}

	if (r < 0) {
		if (*lineptr != (void *)line) {
			int saved_errno = errno;
			free(line);
			errno = saved_errno;
		}
		return r;
	}

	if (*lineptr != NULL && *lineptr != (void *)line)
		free(*lineptr);

	*lineptr = (void *)line;
	*n = size;

	return r;
}

ssize_t getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
	ssize_t r;

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), -1;
	}

	r = locked(lineptr, n, delimiter, stream);
	mtx_unlock(&stream->_mtx);

	return r;
}
