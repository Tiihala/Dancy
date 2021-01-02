/*
 * Copyright (c) 2020 Antti Tiihala
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
 * init/thrd.c
 *      Threads for pre-kernel environment
 */

#include <init.h>

struct init_thrd *init_thrd_current = NULL;

static int init_thrd_lock = 0;
static struct init_thrd *init_thrd_first = NULL;
static struct init_thrd *init_thrd_last = NULL;

const size_t init_thrd_size = 0x2000;

static int check_caller(void)
{
	if (apic_bsp_id != apic_id())
		return 1;

	if (init_thrd_current) {
		if (init_thrd_current != init_thrd_first)
			return 1;
	}

	return 0;
}

static uint32_t get_current_id(void)
{
	uint32_t id = apic_id();

	if (id == apic_bsp_id)
		id += (uint32_t)((addr_t)init_thrd_current);

	return id;
}

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	void *init_thrd_lock_local = &init_thrd_lock;
	struct init_thrd *thr_new;

	/*
	 * This simple implementation allows calling this
	 * function only if running the main thread.
	 */
	if (check_caller())
		return thrd_error;

	if (!init_thrd_first) {
		size_t size = sizeof(struct init_thrd);
		void *s;

		s = malloc(size);
		if (!s)
			return thrd_nomem;

		memset(s, 0, size);
		init_thrd_first = (struct init_thrd *)s;

		init_thrd_first->next = init_thrd_first;
		init_thrd_last = init_thrd_first;
	}

	thr_new = init_thrd_first;

	while (thr_new) {
		if (thr_new->detached && thr_new->terminated)
			break;
		if (thr_new != init_thrd_last)
			thr_new = thr_new->next;
		else
			thr_new = NULL;
	}

	if (!thr_new) {
		void *s = aligned_alloc(init_thrd_size, init_thrd_size);

		if (!s)
			return thrd_nomem;

		memset(s, 0, init_thrd_size);
		thr_new = s;
		thr_new->next = NULL;
	}

	init_thrd_create(thr_new, func, arg);

	spin_enter(&init_thrd_lock_local);

	if (!thr_new->next) {
		thr_new->next = init_thrd_first;

		init_thrd_last->next = thr_new;
		init_thrd_last = thr_new;
	}

	thr_new->detached = 0;
	thr_new->res = 0;
	thr_new->terminated = 0;

	init_thrd_current = init_thrd_first;
	*thr = (thrd_t)thr_new;

	spin_leave(&init_thrd_lock_local);

	return thrd_success;
}

int thrd_join(thrd_t thr, int *res)
{
	struct init_thrd *p = init_thrd_first;
	struct init_thrd *t = (struct init_thrd *)thr;

	/*
	 * This simple implementation allows calling this
	 * function only if running the main thread.
	 */
	if (check_caller())
		return thrd_error;

	while (p != NULL) {
		if (p == t) {
			while (!t->terminated)
				thrd_yield();
			if (res)
				*res = t->res;

			cpu_write32(&t->detached, 1);
			return thrd_success;
		}

		if (p == init_thrd_last)
			break;
		p = p->next;
	}

	return thrd_error;
}

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

	if (type < 0 || (type & mtx_timed) != 0)
		return thrd_error;

	spin_enter(&lock_local);

	for (i = 0; i < INIT_MTX_COUNT; i++) {
		if (init_mtx_array[i].init == 0) {
			m = &init_mtx_array[i];
			m->init = 1;
			m->lock = 0;
			m->type = type;
			m->count = 0;
			break;
		}
	}

	spin_leave(&lock_local);

	if (!m)
		return thrd_error;

	*mtx = m;

	return thrd_success;
}

int mtx_lock(mtx_t *mtx)
{
	int r;

	while ((r = mtx_trylock(mtx)) != thrd_success) {
		if (r != thrd_busy)
			break;
		thrd_yield();
	}

	return r;
}

int mtx_trylock(mtx_t *mtx)
{
	void *lock_local = &init_mtx_lock;
	struct init_mtx *m = *mtx;
	int r = thrd_success;

	spin_enter(&lock_local);

	if (m->count == 0) {
		m->count += 1;
		m->id = get_current_id();
	} else {
		if (m->id != get_current_id())
			r = thrd_busy;
		else if ((m->type & mtx_recursive) == 0)
			r = thrd_error;
		else if (m->count == UINT_MAX)
			r = thrd_error;
		else
			m->count += 1;
	}

	spin_leave(&lock_local);

	return r;
}

int mtx_unlock(mtx_t *mtx)
{
	void *lock_local = &init_mtx_lock;
	struct init_mtx *m = *mtx;
	int r = thrd_success;

	spin_enter(&lock_local);

	if (m->count == 0 || m->id != get_current_id())
		r = thrd_error;
	else
		m->count -= 1;

	spin_leave(&lock_local);

	return r;
}
