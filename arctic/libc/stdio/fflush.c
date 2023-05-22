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
 * libc/stdio/fflush.c
 *      Flush a stream
 */

#include <stdio.h>
#include <threads.h>

int fflush(FILE *stream)
{
	int i, r = 0;

	if (stream != NULL) {
		if (mtx_lock(&stream->__mtx) == thrd_success) {
			r = __dancy_internal_fflush(stream);
			mtx_unlock(&stream->__mtx);
		}
		return r;
	}

	for (i = 0; i < FOPEN_MAX; i++) {
		if ((stream = __dancy_io_array[i]) == NULL)
			continue;

		if (mtx_lock(&stream->__mtx) == thrd_success) {
			r = __dancy_internal_fflush(stream);
			mtx_unlock(&stream->__mtx);
		}
	}

	return r;
}
