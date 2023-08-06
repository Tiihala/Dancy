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
 * libc/stdio/setvbuf.c
 *      Assign buffering to a stream
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static int my_setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
	int dynamic_buffer = 0;

	if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF)
		return -1;

	if (buffer == NULL && size != 0) {
		if ((buffer = malloc(size)) == NULL)
			return -1;
		dynamic_buffer = 1;
	}

	if (size > 0x40000000)
		size = 0x40000000;

	if (size == 0)
		mode = _IONBF;

	if (mode == _IONBF) {
		buffer = NULL;
		size = 0;
	}

	stream->_state &= 0xFFFFFF00u;
	stream->_state &= ~__DANCY_FILE_WRITTEN_BYTES;
	stream->_state |= (unsigned int)mode;

	stream->_buffer_start = 0;
	stream->_buffer_end = 0;

	if ((stream->_state & __DANCY_FILE_STATIC_BUFFER) == 0)
		free(stream->_buffer);

	if (dynamic_buffer)
		stream->_state &= ~__DANCY_FILE_STATIC_BUFFER;
	else
		stream->_state |= __DANCY_FILE_STATIC_BUFFER;

	stream->_buffer = (unsigned char *)buffer;
	stream->_buffer_size = size;

	return 0;
}

int setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
	int r;

	if (mtx_lock(&stream->_mtx) != thrd_success) {
		stream->_error = 1;
		return (errno = EBADF), -1;
	}

	r = my_setvbuf(stream, buffer, mode, size);
	mtx_unlock(&stream->_mtx);

	return r;
}
