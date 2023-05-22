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
 * libc/stdio/_io.c
 *      Standard input and output
 */

#include <__dancy/spin.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

FILE *__dancy_io_array[FOPEN_MAX];
__dancy_mtx_t __dancy_io_array_mtx;

static FILE default_streams[3];

FILE *stdin;
FILE *stdout;
FILE *stderr;

static char name_stdin[] = "stdin";
static char name_stdout[] = "stdout";
static char name_stderr[] = "stderr";

static unsigned char buffer_stdin[128];
static unsigned char buffer_stdout[128];

void __dancy_stdio_init(void)
{
	static int run_once;

	if (!__dancy_spin_trylock(&run_once))
		return;

	/*
	 * Default stream: stdin
	 */
	default_streams[0].__fd = 0;
	default_streams[0].__i = 0;
	default_streams[0].__mode = O_RDONLY;
	default_streams[0].__state = _IOLBF;
	default_streams[0].__state |= __DANCY_FILE_STATIC_BUFFER;
	default_streams[0].__state |= __DANCY_FILE_STATIC_NAME;

	mtx_init(&default_streams[0].__mtx, mtx_plain);
	default_streams[0].__buffer = &buffer_stdin[0];
	default_streams[0].__buffer_size = sizeof(buffer_stdin);
	default_streams[0].__name = &name_stdin[0];

	/*
	 * Default stream: stdout
	 */
	default_streams[1].__fd = 1;
	default_streams[1].__i = 1;
	default_streams[1].__mode = O_WRONLY;
	default_streams[1].__state = _IOLBF;
	default_streams[1].__state |= __DANCY_FILE_STATIC_BUFFER;
	default_streams[1].__state |= __DANCY_FILE_STATIC_NAME;

	mtx_init(&default_streams[1].__mtx, mtx_plain);
	default_streams[1].__buffer = &buffer_stdout[0];
	default_streams[1].__buffer_size = sizeof(buffer_stdout);
	default_streams[1].__name = &name_stdout[0];

	/*
	 * Default stream: stderr
	 */
	default_streams[2].__fd = 2;
	default_streams[2].__i = 2;
	default_streams[2].__mode = O_WRONLY;
	default_streams[2].__state = _IONBF;
	default_streams[2].__state |= __DANCY_FILE_STATIC_NAME;

	mtx_init(&default_streams[2].__mtx, mtx_plain);
	default_streams[2].__name = &name_stderr[0];

	/*
	 * Set the default I/O array streams.
	 */
	stdin = __dancy_io_array[0] = &default_streams[0];
	stdout = __dancy_io_array[1] = &default_streams[1];
	stderr = __dancy_io_array[2] = &default_streams[2];

	mtx_init(&__dancy_io_array_mtx, mtx_plain);
}

void __dancy_stdio_fini(void)
{
	static int run_once;
	int i;

	if (!__dancy_spin_trylock(&run_once))
		return;

	for (i = 0; i < FOPEN_MAX; i++) {
		FILE *stream = __dancy_io_array[i];
		if (stream != NULL)
			(void)fclose(stream);
	}
}

#define INTERNAL_FFLUSH_RETRY_COUNT 8

int __dancy_internal_fflush(FILE *stream)
{
	int retry_count = INTERNAL_FFLUSH_RETRY_COUNT;
	int r = 0;

	if ((stream->__state & __DANCY_FILE_WRITTEN_BYTES) == 0)
		return 0;

	while (stream->__buffer_start < stream->__buffer_end) {
		int start = stream->__buffer_start;

		const void *buffer = &stream->__buffer[start];
		size_t size = (size_t)stream->__buffer_end - (size_t)start;
		ssize_t w = write(stream->__fd, buffer, size);

		if (w < 0 && (--retry_count) <= 0) {
			stream->__error = 1;
			return EOF;
		}

		if (w == 0 && (--retry_count) <= 0) {
			stream->__error = 1;
			return (errno = EIO), EOF;
		}

		if (w > 0)
			retry_count = INTERNAL_FFLUSH_RETRY_COUNT;

		stream->__buffer_start = start + (int)w;
	}

	stream->__state &= ~__DANCY_FILE_WRITTEN_BYTES;
	stream->__buffer_start = 0;
	stream->__buffer_end = 0;

	return r;
}
