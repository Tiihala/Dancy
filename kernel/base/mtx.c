/*
 * Copyright (c) 2021 Antti Tiihala
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
 * base/mtx.c
 *      Mutex implementation
 */

#include <dancy.h>

static int base_mtx_lock;

struct base_mtx {
	int init;
	int lock;
	int yield;
};

#define BASE_MTX_COUNT 128
static struct base_mtx base_mtx_array[BASE_MTX_COUNT];

#define null_mxt (mtx == NULL || *mtx == NULL)
#define this_mtx ((struct base_mtx *)(*mtx))

void mtx_destroy(mtx_t *mtx)
{
	if (null_mxt)
		return;

	this_mtx->init = 0;
}

int mtx_init(mtx_t *mtx, int type)
{
	void *lock_local = &base_mtx_lock;
	struct base_mtx *m = NULL;
	int i;

	if (mtx == NULL || type != mtx_plain)
		return thrd_error;

	spin_enter(&lock_local);

	for (i = 0; i < BASE_MTX_COUNT; i++) {
		if (base_mtx_array[i].init == 0) {
			m = &base_mtx_array[i];
			m->init = 1;
			m->yield = 0;
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
	if (null_mxt)
		return thrd_error;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		return thrd_error;

	while (!spin_trylock(&this_mtx->lock)) {
		this_mtx->yield = 1;
		task_yield();
	}

	return thrd_success;
}

int mtx_trylock(mtx_t *mtx)
{
	if (null_mxt)
		return thrd_error;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		return thrd_error;

	if (!spin_trylock(&this_mtx->lock))
		return thrd_busy;

	return thrd_success;
}

int mtx_unlock(mtx_t *mtx)
{
	int yield;

	if (null_mxt)
		return thrd_error;

	yield = this_mtx->yield;
	this_mtx->yield = 0;

	spin_unlock(&this_mtx->lock);

	if (yield)
		task_yield();

	return thrd_success;
}
