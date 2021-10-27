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
 * sched/sched.c
 *      Task scheduler
 */

#include <dancy.h>

static int sched_lock = 1;

static void (*default_yield)(void);
static void yield(void);

int sched_init(void)
{
	static int run_once;
	void (**yield_pointer)(void);

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	yield_pointer = &kernel->scheduler.yield;
	default_yield = *yield_pointer;

	spin_unlock(&sched_lock);
	*yield_pointer = yield;

	return 0;
}

static void yield(void)
{
	default_yield();
}
