/*
 * Copyright (c) 2020, 2021 Antti Tiihala
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
 * init/mtx.c
 *      Mutex implementation for pre-kernel environment
 */

#include <boot/init.h>

struct init_mtx {
	int init;
	int lock;
};

#define INIT_MTX_COUNT 8
static struct init_mtx init_mtx_array[INIT_MTX_COUNT];
static int init_mtx_lock;

void mtx_destroy(mtx_t *mtx)
{
	((struct init_mtx *)(*mtx))->init = 0;
}

int mtx_init(mtx_t *mtx, int type)
{
	void *lock_local = &init_mtx_lock;
	struct init_mtx *m = NULL;
	int i;

	if (type != mtx_plain)
		return thrd_error;

	spin_enter(&lock_local);

	for (i = 0; i < INIT_MTX_COUNT; i++) {
		if (init_mtx_array[i].init == 0) {
			m = &init_mtx_array[i];
			m->init = 1;
			spin_unlock(&m->lock);
			*mtx = m;
			break;
		}
	}

	spin_leave(&lock_local);

	return (m == NULL) ? thrd_error : thrd_success;
}

int mtx_lock(mtx_t *mtx)
{
	int r;

	while ((r = mtx_trylock(mtx)) != thrd_success) {
		if (r != thrd_busy)
			break;
	}

	return r;
}

int mtx_trylock(mtx_t *mtx)
{
	struct init_mtx *m = *mtx;

	if (!spin_trylock(&m->lock))
		return thrd_busy;

	return thrd_success;
}

int mtx_unlock(mtx_t *mtx)
{
	struct init_mtx *m = *mtx;

	spin_unlock(&m->lock);

	return thrd_success;
}
