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
 * base/task.c
 *      Task management
 */

#include <dancy.h>

void task_create_asm(struct task *new_task, int (*func)(void *), void *arg);
void task_switch_asm(struct task *next);

static int task_ready;

static uint32_t task_default_cr3;
static uint8_t task_default_fstate[512];

static int task_id_lock;
static uint64_t task_id;

static uint64_t task_get_id(void)
{
	void *lock_local = &task_id_lock;
	uint64_t id;

	spin_enter(&lock_local);
	id = task_id++;
	spin_leave(&lock_local);

	return id;
}

int task_init(void)
{
	static int run_once;
	struct task *current;
	const uint8_t *fstate;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	task_default_cr3 = (uint32_t)cpu_read_cr3();

#ifdef DANCY_32
	if ((cpu_read_cr4() & (1u << 9)) == 0) {
		extern uint8_t task_patch_fxsave[3];
		extern uint8_t task_patch_fxrstor[3];

		if (task_patch_fxsave[0] != 0x0F)
			return DE_UNEXPECTED;
		if (task_patch_fxsave[1] != 0xAE)
			return DE_UNEXPECTED;
		if (task_patch_fxsave[2] != 0x03)
			return DE_UNEXPECTED;

		/*
		 * Use fsave [ebx] instruction.
		 */
		task_patch_fxsave[0] = 0x9B;
		task_patch_fxsave[1] = 0xDD;
		task_patch_fxsave[2] = 0x33;

		if (task_patch_fxrstor[0] != 0x0F)
			return DE_UNEXPECTED;
		if (task_patch_fxrstor[1] != 0xAE)
			return DE_UNEXPECTED;
		if (task_patch_fxrstor[2] != 0x0B)
			return DE_UNEXPECTED;

		/*
		 * Use frstor [ebx] instruction.
		 */
		task_patch_fxrstor[0] = 0xDD;
		task_patch_fxrstor[1] = 0x23;
		task_patch_fxrstor[2] = 0x90;
	}
#endif
	current = memset((void *)task_current(), 0, 0x1000);
	task_default_cr3 = (uint32_t)cpu_read_cr3();
	current->cr3 = task_default_cr3;
	current->id = task_get_id();

	task_switch_asm(current);
	current->state = TASK_FLAG_RUNNING;

	fstate = (const uint8_t *)current + 0x0C00;
	memcpy(&task_default_fstate[0], fstate, 512);

	cpu_write32((uint32_t *)&task_ready, 1);

	return 0;
}

int task_init_ap(void)
{
	struct task *current;

	while (cpu_read32((const uint32_t *)&task_ready) == 0)
		delay(1000000);

	current = memset((void *)task_current(), 0, 0x1000);
	current->cr3 = task_default_cr3;
	current->id = task_get_id();

	task_switch_asm(current);
	current->state = TASK_FLAG_RUNNING;

	return 0;
}

struct task *task_create(int (*func)(void *), void *arg)
{
	struct task *new_task = aligned_alloc(0x2000, 0x2000);
	uint8_t *fstate;

	if (new_task) {
		memset(new_task, 0, 0x2000);
		new_task->cr3 = task_default_cr3;
		new_task->id = task_get_id();

		fstate = (uint8_t *)new_task + 0x0C00;
		memcpy(fstate, &task_default_fstate[0], 512);

		task_create_asm(new_task, func, arg);
	}

	return new_task;
}

void task_switch(struct task *next)
{
	void *lock_local;
	int state;

	if (next == NULL)
		return;

	lock_local = &next->lock;

	spin_enter(&lock_local);
	state = next->state;
	next->state = (state | TASK_FLAG_RUNNING);
	spin_leave(&lock_local);

	if ((state & TASK_FLAG_RUNNING) == 0)
		task_switch_asm(next);
}

void task_yield(void)
{

}
