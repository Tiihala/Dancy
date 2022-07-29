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
 * threads.h
 *      The C Standard Library
 */

#ifndef ARCTIC_DANCY_THREADS_H
#define ARCTIC_DANCY_THREADS_H

typedef void *mtx_t;

enum dancy_mtx_constants {
	mtx_plain,
	mtx_timed,
	mtx_recursive
};

enum dancy_thrd_constants {
	thrd_success,
	thrd_busy,
	thrd_error,
	thrd_nomem,
	thrd_timedout
};

void mtx_destroy(mtx_t *mtx);
int mtx_init(mtx_t *mtx, int type);
int mtx_lock(mtx_t *mtx);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);

#endif
