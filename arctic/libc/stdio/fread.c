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
	int buffer_mode = stream->__state & 0xFF;
	size_t ret_size = 0;

	if (stream->__mode == O_WRONLY) {
		stream->__error = 1;
		return (errno = EBADF), ret_size;
	}

	if (stream->__buffer_start != 0 || stream->__buffer_end != 0) {
		if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) != 0) {
			stream->__error = 1;
			return (errno = EBADF), ret_size;
		}
	}

	while (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		if (stream->__buffer_start >= stream->__buffer_end) {
			int fd = stream->__fd;
			size_t buffer_size = stream->__buffer_size;
			ssize_t w;

			if (buffer_size== 0) {
				stream->__error = 1;
				return (errno = EBADF), ret_size;
			}

			w = read(fd, stream->__buffer, buffer_size);

			if (w < 0) {
				stream->__error = 1;
				return ret_size;
			}

			if (w == 0) {
				stream->__eof = 1;
				return ret_size;
			}

			stream->__state &= ~__DANCY_FILE_WRITTEN_BYTES;
			stream->__buffer_start = 0;
			stream->__buffer_end = (int)w;
		}

		if (stream->__ungetc && ret_size < size) {
			unsigned char *p = &buffer[ret_size++];

			*p = (unsigned char)((int)(stream->__ungetc & 0xFF));
			stream->__ungetc = 0;
		}

		while (ret_size < size) {
			unsigned char *p = &buffer[ret_size++];
			unsigned char c;

			*p = (c = stream->__buffer[stream->__buffer_start++]);

			if (buffer_mode == _IOLBF && c == 0x0A)
				return ret_size;

			if (stream->__buffer_start >= stream->__buffer_end)
				break;
		}

		if (ret_size == size)
			return ret_size;
	}

	if (buffer_mode == _IONBF) {
		ssize_t w;

		if (stream->__ungetc && ret_size < size) {
			unsigned char *p = &buffer[ret_size++];

			*p = (unsigned char)((int)(stream->__ungetc & 0xFF));
			stream->__ungetc = 0;

			if (--size == 0)
				return ret_size;
			buffer += 1;
		}

		w = read(stream->__fd, buffer, size);

		if (w < 0)
			stream->__error = 1;
		else if (w == 0)
			stream->__eof = 1;
		else
			ret_size += (size_t)w;

		return ret_size;
	}

	stream->__error = 1;
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

	if (mtx_lock(&stream->__mtx) != thrd_success) {
		stream->__error = 1;
		return (errno = EBADF), 0;
	}

	r = my_read(buffer, total_size, stream);
	mtx_unlock(&stream->__mtx);

	return r;
}
