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
 * libc/stdio/fclose.c
 *      Close a stream
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

int fclose(FILE *stream)
{
	int r = 0;

	if ((mtx_lock(&__dancy_io_array_mtx)) == thrd_success) {
		if (__dancy_io_array[stream->__i] == NULL)
			return mtx_unlock(&__dancy_io_array_mtx), 0;

		__dancy_io_array[stream->__i] = NULL;
		mtx_unlock(&__dancy_io_array_mtx);
	}

	if (mtx_lock(&stream->__mtx) == thrd_success) {
		unsigned int state = stream->__state;

		r = __dancy_internal_fflush(stream);

		if (close(stream->__fd) == -1)
			r = EOF;

		if ((state & __DANCY_FILE_STATIC_BUFFER) == 0)
			free(stream->__buffer), stream->__buffer = NULL;

		mtx_unlock(&stream->__mtx);
		mtx_destroy(&stream->__mtx);

		memset(stream, 0, sizeof(FILE));
		stream->__fd = -1;

		if ((state & __DANCY_FILE_MALLOC_STRUCT) != 0)
			free(stream);
	}

	return r;
}
