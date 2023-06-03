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
 * libc/stdio/fwrite.c
 *      Put bytes on a stream
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

static size_t my_write(const unsigned char *buffer, size_t size, FILE *stream)
{
	int buffer_mode = stream->__state & 0xFF;
	size_t ret_size = 0;

	if (stream->__mode == O_RDONLY) {
		stream->__error = 1;
		return (errno = EBADF), ret_size;
	}

	if (stream->__buffer_start != 0 || stream->__buffer_end != 0) {
		if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) == 0) {
			stream->__error = 1;
			return (errno = EBADF), ret_size;
		}
	}

	if (buffer_mode == _IOFBF || buffer_mode == _IOLBF) {
		int buffer_size = (int)stream->__buffer_size;
		unsigned char *p = stream->__buffer;

		while (ret_size < size) {
			unsigned char c;

			if (stream->__buffer_end >= buffer_size) {
				if ( __dancy_internal_fflush(stream)) {
					stream->__error = 1;
					return ret_size;
				}
			}

			p[stream->__buffer_end++] = (c = buffer[ret_size++]);
			stream->__state |= __DANCY_FILE_WRITTEN_BYTES;

			if (buffer_mode == _IOLBF && c == 0x0A) {
				if ( __dancy_internal_fflush(stream)) {
					stream->__error = 1;
					return ret_size;
				}
			}
		}

		return ret_size;
	}

	if (buffer_mode == _IONBF) {
		ssize_t w = write(stream->__fd, buffer, size);

		if (w > 0)
			ret_size += (size_t)w;

		if (w < 0 || (size_t)w < size)
			stream->__error = 1;

		return ret_size;
	}

	stream->__error = 1;
	return (errno = EBADF), ret_size;
}

size_t fwrite(const void *buffer, size_t size, size_t nmemb, FILE *stream)
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

	r = my_write(buffer, total_size, stream);
	mtx_unlock(&stream->__mtx);

	return (r / size);
}
