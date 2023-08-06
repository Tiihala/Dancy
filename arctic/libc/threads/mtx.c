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
 * libc/threads/mtx.c
 *      Mutex functions
 */

#include <threads.h>

static int not_implemented = 1;

#define null_mxt (mtx == NULL || mtx->_m == NULL)

void mtx_destroy(mtx_t *mtx)
{
	if (null_mxt)
		return;

	mtx->_m = NULL;
}

int mtx_init(mtx_t *mtx, int type)
{
	if (mtx == NULL || type != mtx_plain)
		return thrd_error;

	mtx->_m = &not_implemented;

	return thrd_success;
}

int mtx_lock(mtx_t *mtx)
{
	if (null_mxt)
		return thrd_error;

	return thrd_success;
}

int mtx_trylock(mtx_t *mtx)
{
	if (null_mxt)
		return thrd_error;

	return thrd_success;
}

int mtx_unlock(mtx_t *mtx)
{
	if (null_mxt)
		return thrd_error;

	return thrd_success;
}
