/*
 * Copyright (c) 2022 Antti Tiihala
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
 * libc/stdlib/atexit.c
 *      The C Standard Library
 */

#include <stdlib.h>
#include <threads.h>

#define __DANCY_ATEXIT_ARRAY_COUNT (32)

int __dancy_atexit_idx;
mtx_t __dancy_atexit_mtx;

void (*__dancy_atexit_array[__DANCY_ATEXIT_ARRAY_COUNT])(void);

void __dancy_atexit_init(void)
{
	mtx_init(&__dancy_atexit_mtx, mtx_plain);
}

void __dancy_atexit_fini(void)
{
	if (mtx_lock(&__dancy_atexit_mtx) != thrd_success)
		return;

	while (__dancy_atexit_idx > 0) {
		void (*func)(void);

		func = __dancy_atexit_array[--__dancy_atexit_idx];

		mtx_unlock(&__dancy_atexit_mtx);

		func();

		if (mtx_lock(&__dancy_atexit_mtx) != thrd_success)
			return;
	}

	mtx_unlock(&__dancy_atexit_mtx);
}

int atexit(void (*func)(void))
{
	int r = 0;

	if (mtx_lock(&__dancy_atexit_mtx) != thrd_success)
		return -1;

	if (__dancy_atexit_idx < __DANCY_ATEXIT_ARRAY_COUNT)
		__dancy_atexit_array[__dancy_atexit_idx++] = func;
	else
		r = -1;

	mtx_unlock(&__dancy_atexit_mtx);

	return r;
}
