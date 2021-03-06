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
void task_switch_asm(struct task *next, void *tss);

static int task_ready;
static int task_ap_count;

static uint8_t task_default_fstate[512];
static void *task_uniproc_tss;

static int task_lock;
static struct task *task_head;
static struct task *task_tail;

static int task_struct_count;
static int task_struct_limit;

static int task_id_lock;
static uint64_t task_id;

static int task_append(struct task *new_task)
{
	void *lock_local = &task_lock;

	new_task->next = task_head;

	spin_enter(&lock_local);

	if (task_struct_count >= task_struct_limit)
		return spin_leave(&lock_local), 1;

	task_struct_count += 1;
	task_tail = (task_tail->next = new_task);

	spin_leave(&lock_local);

	return 0;
}

static uint64_t task_create_id(void)
{
	void *lock_local = &task_id_lock;
	uint64_t id;

	spin_enter(&lock_local);
	if ((id = ++task_id) == 0)
		panic("Error: task ID");
	spin_leave(&lock_local);

	return id;
}

static struct task *task_reuse(void)
{
	struct task *t = task_head;

	/*
	 * The "t->active" value is locked and the "t->next" pointer
	 * is preserved if a non-NULL task structure is returned.
	 */
	while (t != NULL) {
		if (!t->id && spin_trylock(&t->active)) {
			if (!t->id && t->detached && t->stopped) {
				unsigned char *p;
				size_t offset, size;

				t->retval = 0;
				t->stopped  = 0;
				t->ndisable = 0;

				p = (unsigned char *)&t->next;
				p += sizeof(t->next);

				offset = (size_t)(p - (unsigned char *)t);
				size = 0x2000 - offset;

				p = (unsigned char *)t + offset;
				memset(p, 0, size);

				return t;
			}
			spin_unlock(&t->active);
		}
		t = (t->next != task_head) ? t->next : NULL;
	}

	return NULL;
}

int task_init(void)
{
	static int run_once;
	struct task *current;
	const uint8_t *fstate;
	uint32_t ap_count;
	int pages;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	task_uniproc_tss = gdt_get_tss();

	task_struct_limit = 16384;
	pages = (int)mm_available_pages(mm_addr32);

	while (task_struct_limit > (pages / 16))
		task_struct_limit /= 2;

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
	current->cr3 = (uint32_t)pg_kernel;
	current->id = task_create_id();

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;

	fstate = (const uint8_t *)current + 0x0C00;
	memcpy(&task_default_fstate[0], fstate, 512);

	task_head = (task_tail = current);
	task_struct_count = 1;

	cpu_write32((uint32_t *)&task_ready, 1);

	ap_count = (uint32_t)kernel->smp_ap_count;

	while (cpu_read32((const uint32_t *)&task_ap_count) != ap_count)
		delay(1000000);

	return 0;
}

int task_init_ap(void)
{
	struct task *current;

	while (cpu_read32((const uint32_t *)&task_ready) == 0)
		delay(1000000);

	current = memset((void *)task_current(), 0, 0x1000);
	current->cr3 = (uint32_t)pg_kernel;
	current->id = task_create_id();

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;

	if (task_append(current))
		panic("task_init_ap: unexpected behavior");

	spin_lock(&task_lock);
	task_ap_count += 1;
	if (task_ap_count > kernel->smp_ap_count)
		panic("task_init_ap: called too many times");
	spin_unlock(&task_lock);

	return 0;
}

uint64_t task_create(int (*func)(void *), void *arg, int type)
{
	uint64_t id = 0;
	struct task *new_task = task_reuse();
	uint8_t *fstate;

	if (!new_task) {
		new_task = (struct task *)mm_alloc_pages(mm_kernel, 1);

		if (!new_task)
			return id;

		memset(new_task, 0, 0x2000);
		spin_trylock(&new_task->active);

		if (task_append(new_task)) {
			mm_free_pages((phys_addr_t)new_task, 1);
			return id;
		}
	}

	new_task->cr3 = (uint32_t)pg_kernel;
	new_task->id = (id = task_create_id());
	new_task->id_owner = task_current()->id;

	fstate = (uint8_t *)new_task + 0x0C00;
	memcpy(fstate, &task_default_fstate[0], 512);

	if ((type & task_detached) != 0)
		new_task->detached = 1;

	if ((type & task_uniproc) != 0)
		new_task->uniproc = 1;

	task_create_asm(new_task, func, arg);
	spin_unlock(&new_task->active);

	return id;
}

void task_exit(int retval)
{
	struct task *current = task_current();
	struct task *next = current->next;

	if (!current->id_owner)
		panic("task_exit: system task stopped");

	current->retval = retval;
	current->stopped = 1;

	pg_delete();

	if (current->detached)
		current->id = 0;

	while (task_switch(next))
		next = (!next) ? task_head : next->next;

	panic("task_exit: unexpected behavior");
}

void task_jump(addr_t user_ip, addr_t user_sp)
{
	extern void task_jump_asm(addr_t ip, addr_t cs, addr_t sp, addr_t ss);

	addr_t cs = gdt_user_code | 3;
	addr_t ss = gdt_user_data | 3;

	task_jump_asm(user_ip, cs, user_sp, ss);
}

static int task_switch_generic(struct task *next, void *tss)
{
	int r;

	if (!spin_trylock(&next->active))
		return 1;

	if (next->stopped) {
		spin_unlock(&next->active);
		return 1;
	}

	if (!tss) {
		/*
		 * This variable has already been checked in the
		 * task_switch function without locking the task
		 * structure. Check that it has not changed.
		 */
		if (next->uniproc) {
			spin_unlock(&next->active);
			return 1;
		}

		/*
		 * Disable interrupts. The TSS must not change before
		 * completing the task_switch_asm function. The original
		 * interrupt flag state will be restored.
		 */
		r = cpu_ints(0);

		tss = gdt_get_tss();
		task_switch_asm(next, tss);

		cpu_ints(r);

	} else {
		/*
		 * The caller has already disabled interrupts.
		 */
		task_switch_asm(next, tss);
	}

	return 0;
}

int task_switch(struct task *next)
{
	int r0, r1 = 1;

	if (!next)
		return 1;

	/*
	 * The typical case is to allow execution on all processors.
	 */
	if (!next->uniproc)
		return task_switch_generic(next, NULL);

	/*
	 * Allow execution on the bootstrap processor only.
	 */
	r0 = cpu_ints(0);

	if (gdt_get_tss() == task_uniproc_tss)
		r1 = task_switch_generic(next, task_uniproc_tss);

	cpu_ints(r0);

	return r1;
}

int task_trywait(uint64_t id, int *retval)
{
	struct task *current = task_current();
	struct task *next = current->next;
	int r = 1;

	if (retval)
		*retval = 0;

	if (!id || current->id == id)
		return 1;

	while (current != next && next) {
		if (next->id == id) {
			while (!next->stopped || !spin_trylock(&next->active))
				return 1;

			if (next->id == id && next->stopped) {
				if (retval)
					*retval = next->retval;
				next->detached = 1;
				next->id = 0;
				r = 0;
			}

			spin_unlock(&next->active);
			return r;
		}
		next = next->next;
	}

	return 1;
}

int task_wait(uint64_t id, int *retval)
{
	struct task *current = task_current();
	struct task *next = current->next;
	int r = 1;

	if (retval)
		*retval = 0;

	if (!id || current->id == id)
		return 1;

	while (current != next && next) {
		if (next->id == id) {
			while (!next->stopped || !spin_trylock(&next->active))
				task_yield();

			if (next->id == id && next->stopped) {
				if (retval)
					*retval = next->retval;
				next->detached = 1;
				next->id = 0;
				r = 0;
			}

			spin_unlock(&next->active);
			return r;
		}
		next = next->next;
	}

	return 1;
}

void task_yield(void)
{
	struct task *current = task_current();
	struct task *next = current->next;

	while (task_switch(next)) {
		if (!next || next == current)
			break;
		next = next->next;
	}
}
