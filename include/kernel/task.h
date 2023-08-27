/*
 * Copyright (c) 2021, 2022, 2023 Antti Tiihala
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

#include <common/types.h>

enum task_type {
	task_normal   = 0x00,
	task_detached = 0x01,
	task_uniproc  = 0x02
};

#define TASK_CMD_STATIC_SIZE 32
#define TASK_FD_STATIC_COUNT 64

struct task {
	uint64_t sp;        /* Offset: 0 */
	uint64_t cr3;       /* Offset: 8 */
	int active;         /* Offset: 16 + 0 * sizeof(int) */
	int asm_data1;      /* Offset: 16 + 1 * sizeof(int) */
	int asm_data2;      /* Offset: 16 + 2 * sizeof(int) */
	int asm_data3;      /* Offset: 16 + 3 * sizeof(int) */
	addr_t next;        /* Offset: 16 + 4 * sizeof(int) */

	uint64_t id;
	uint64_t id_owner;
	uint64_t id_group;
	uint64_t id_session;
	struct task *owner;

	int detached;
	int retval;
	int stopped;
	int uniproc;

	struct {
		int (*func)(uint64_t *data);
		uint64_t data[2];
	} event;

	cpu_native_t iret_frame[5];
	int iret_lock;
	int iret_num;

	struct {
		uint32_t data[2];
	} descendant;

	uint32_t pg_cr3;
	uint32_t pg_alt_cr3;
	uint32_t pg_state;
	cpu_native_t pg_user_memory;

	struct {
		int lock;
		int priority;
		uint32_t state;
	} sched;

	struct {
		uint8_t *line;
		uint8_t _line[TASK_CMD_STATIC_SIZE];
	} cmd;

	struct {
		uint32_t state;
		void (*release)(struct task *task);
		void (*clone)(struct task *task, struct task *new_task);
		void *wd_node;
		uint32_t *table;
		uint32_t _table[TASK_FD_STATIC_COUNT];
	} fd;
};

struct task *task_read_next(const struct task *task);
struct task *task_write_next(struct task *task, struct task *next);

int task_init(void);
int task_init_ap(void);

struct task *task_current(void);
struct task *task_find(uint64_t id);

uint64_t task_create(int (*func)(void *), void *arg, int type);
void task_access(void (*func)(struct task *, void *), void *arg);
void task_foreach(int (*func)(struct task *, void *), void *arg);
void task_set_cmdline(struct task *task, void *line, const char *cline);

void task_identify(uint64_t *id, uint64_t *id_owner,
	uint64_t *id_group, uint64_t *id_session);

int task_check_event(struct task *task);
int task_read_event(void);
void task_write_event(int (*func)(uint64_t *data), uint64_t d0, uint64_t d1);

void task_exit(int retval);
void task_jump(addr_t user_ip, addr_t user_sp);
void task_sleep(uint64_t milliseconds);

int task_switch(struct task *next);
void task_switch_disable(void);
void task_switch_enable(void);

int task_trywait_descendant(uint64_t *id, uint64_t id_group, int *retval);
int task_wait_descendant(uint64_t *id, uint64_t id_group, int *retval);
void task_yield(void);

#endif
