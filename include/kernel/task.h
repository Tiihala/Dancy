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

enum task_type {
	task_normal   = 0x00,
	task_detached = 0x01,
	task_uniproc  = 0x02
};

struct task {
	uint32_t esp;       /* Offset: 0 */
	uint32_t cr3;       /* Offset: 4 */
	uint64_t id;        /* Offset: 8 */
	int active;         /* Offset: 16 + 0 * sizeof(int) */
	int retval;         /* Offset: 16 + 1 * sizeof(int) */
	int stopped;        /* Offset: 16 + 2 * sizeof(int) */
	int ndisable;       /* Offset: 16 + 3 * sizeof(int) */
	struct task *next;  /* Offset: 16 + 4 * sizeof(int) */

	uint64_t id_owner;

	cpu_native_t iret_frame[5];
	int iret_lock;
	int iret_num;

	uint32_t pg_cr3;
	uint32_t pg_state;

	int detached;
	int uniproc;
};

int task_init(void);
int task_init_ap(void);

struct task *task_current(void);
uint64_t task_create(int (*func)(void *), void *arg, int type);

void task_exit(int retval);
void task_jump(addr_t user_ip, addr_t user_sp);

int task_switch(struct task *next);
void task_switch_disable(void);
void task_switch_enable(void);

int task_trywait(uint64_t id, int *retval);
int task_wait(uint64_t id, int *retval);
void task_yield(void);

#endif
