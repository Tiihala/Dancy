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
 * libc/stdio/fread.c
 *      Get bytes from a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static size_t my_read(unsigned char *buffer, size_t size, FILE *stream)
{
	int buffer_mode = stream->_state & 0xFF;
	size_t ret_size = 0;

	if (stream->_mode == O_WRONLY) {
		stream->_error = 1;
		return (errno = EBADF), ret_size;
	}

	if (stream->_buffer_start != 0 || stream->_buffer_end != 0) {
		if ((stream->_state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			stream->_error = 1;
			return (errno = EBADF), ret_size;
		}
	}

	while (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		if (stream->_buffer_start >= stream->_buffer_end) {
			int fd = stream->_fd;
			size_t buffer_size = stream->_buffer_size;
			ssize_t w;

			if (buffer_size== 0) {
				stream->_error = 1;
				return (errno = EBADF), ret_size;
			}

			w = read(fd, stream->_buffer, buffer_size);

			if (w < 0) {
				stream->_error = 1;
				return ret_size;
			}

			if (w == 0) {
				stream->_eof = 1;
				return ret_size;
			}

			stream->_state &= ~__DANCY_FILE_WRITTEN_BYTES;
			stream->_buffer_start = 0;
			stream->_buffer_end = (int)w;
		}

		if (stream->_ungetc && ret_size < size) {
			unsigned char *p = &buffer[ret_size++];

			*p = (unsigned char)((int)(stream->_ungetc & 0xFF));
			stream->_ungetc = 0;
		}

		while (ret_size < size) {
			unsigned char *p = &buffer[ret_size++];
			unsigned char c;

			*p = (c = stream->_buffer[stream->_buffer_start++]);

			if (stream->_buffer_start >= stream->_buffer_end)
				break;
		}

		if (ret_size == size)
			return ret_size;
	}

	if (buffer_mode == _IONBF) {
		ssize_t w;

		if (stream->_ungetc && ret_size < size) {
			unsigned char *p = &buffer[ret_size++];

			*p = (unsigned char)((int)(stream->_ungetc & 0xFF));
			stream->_ungetc = 0;

			if (--size == 0)
				return ret_size;
			buffer += 1;
		}

		w = read(stream->_fd, buffer, size);

		if (w > 0 && (size_t)w < size) {
			ret_size += (size_t)w;
			buffer += w;
			size -= (size_t)w;

			w = read(stream->_fd, buffer, size);
		}

		if (w < 0)
			stream->_error = 1;
		else if (w == 0)
			stream->_eof = 1;
		else
			ret_size += (size_t)w;

		return ret_size;
	}

	stream->_error = 1;
	return (errno = EBADF), ret_size;
}

size_t fread(void *buffer, size_t size, size_t nmemb, FILE *stream)
{
	size_t total_size = size * nmemb;
	size_t r;

	if (size == 0 || nmemb == 0)
		return 0;

	if ((total_size / nmemb) != size)
		total_size = __DANCY_SIZE_MAX;

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), 0;
	}

	r = my_read(buffer, total_size, stream);
	mtx_unlock(&stream->_mtx);

	return (r / size);
}
