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
 * kernel/task.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <dancy/types.h>

struct task {
	uint32_t esp;    /* Offset: 0 */
	uint32_t cr3;    /* Offset: 4 */
	uint64_t id;     /* Offset: 8 */
	int active;      /* Offset: 16 + 0 * sizeof(int) */
	int retval;      /* Offset: 16 + 1 * sizeof(int) */
	int stopped;     /* Offset: 16 + 2 * sizeof(int) */
	int ndisable;    /* Offset: 16 + 3 * sizeof(int) */

	uint64_t id_owner;
	struct task *next;
};

int task_init(void);
int task_init_ap(void);

struct task *task_current(void);
uint64_t task_create(int (*func)(void *), void *arg);
void task_exit(int retval);

int task_switch(struct task *next);
void task_switch_disable(void);
void task_switch_enable(void);
void task_yield(void);

#endif