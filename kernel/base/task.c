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
static int task_ap_sync;

static uint8_t task_default_fstate[512];
static void *task_uniproc_tss;

static int task_lock;
static struct task *task_head;
static struct task *task_tail;

static int task_pool_count;
static struct task *task_pool_head;

static int task_struct_count;
static int task_struct_limit;

static void task_append(struct task *new_task)
{
	void *lock_local = &task_lock;

	task_write_next(new_task, task_head);

	spin_enter(&lock_local);
	task_tail = task_write_next(task_tail, new_task);
	spin_leave(&lock_local);
}

static uint64_t task_create_id(void)
{
	static int task_id_lock;
	static uint64_t task_id;

	void *lock_local = &task_id_lock;
	uint64_t id;

	spin_enter(&lock_local);

	task_id = (id = cpu_read64(&task_id) + 1);

	if (id == 0 || (id & 0x8000000000000000ull) != 0)
		panic("Error: task ID");

	spin_leave(&lock_local);

	return id;
}

static struct task *task_create_from_pool(void)
{
	struct task *new_task = NULL;
	void *lock_local = &task_lock;

	spin_enter(&lock_local);

	if (task_pool_head) {
		new_task = task_pool_head;

		task_pool_count -= 1;
		task_pool_head = task_read_next(task_pool_head);

		task_write_next(new_task, task_head);
		task_tail = task_write_next(task_tail, new_task);

		if (!new_task->active)
			panic("Error: inconsistent task structures");

		new_task->id = task_create_id();
		new_task->id_owner = task_current()->id;
	}

	spin_leave(&lock_local);

	return new_task;
}

static int task_null_func(uint64_t *data)
{
	(void)data;
	return 0;
}

static void task_schedule_default(void)
{
	struct task *current = task_current();
	struct task *next = task_read_next(current);

	while (task_switch(next)) {
		if (next == current)
			break;
		next = !next ? task_read_next(current) : task_read_next(next);
	}
}

static int task_caretaker(void *arg)
{
	void *lock_local = &task_lock;

	while (arg == NULL) {
		struct task *t = task_head;

		while (t != NULL) {
			struct task *t0 = t;
			struct task *t1 = task_read_next(t);
			struct task *t2;

			int error_assumption;
			unsigned char *p;
			size_t offset, size;

			if ((t = task_read_next(t)) == task_head)
				t = NULL;

			/*
			 * Do a quick test without locking the structure.
			 */
			if (!t1 || !t1->detached || !t1->stopped)
				continue;

			if (!spin_trylock(&t1->active))
				continue;

			if (!t1->detached || !t1->stopped) {
				spin_unlock(&t1->active);
				continue;
			}

			t2 = task_read_next(t1);
			error_assumption = 0;

			/*
			 * Acquire task_lock before doing anything for real.
			 */
			spin_enter(&lock_local);

			if (t1 == task_head)
				error_assumption = 1;

			if (task_read_next(t0) != t1)
				error_assumption = 1;

			if (task_read_next(t1) != t2)
				error_assumption = 1;

			if (t0 == t1 || t0 == t2 || t1 == t2 || t2 == NULL)
				error_assumption = 1;

			if (error_assumption) {
				spin_leave(&lock_local);
				spin_unlock(&t1->active);
				continue;
			}

			/*
			 * Change the task_tail if needed.
			 */
			if (t1 == task_tail)
				task_tail = t0;

			/*
			 * Remove the middle structure from the list.
			 */
			task_write_next(t0, t2);

			t1->esp = 0;
			t1->cr3 = 0;
			t1->id = 0;

			t1->retval = 0;
			t1->stopped  = 0;
			t1->ndisable = 0;

			/*
			 * Release task_lock so that interrupts are not
			 * disabled when doing the memset for the structure.
			 */
			spin_leave(&lock_local);

			p = (unsigned char *)&t1->next;
			p += sizeof(t1->next);

			offset = (size_t)(p - (unsigned char *)t1);
			size = 0x2000 - offset;

			p = (unsigned char *)t1 + offset;
			memset(p, 0, size);

			/*
			 * Add to the head of the pool list. The task
			 * structure has its t1->active locked.
			 */
			spin_enter(&lock_local);

			task_write_next(t1, task_pool_head);

			task_pool_count += 1;
			task_pool_head = t1;

			spin_leave(&lock_local);

			/*
			 * The next structure for the while loop.
			 */
			t = t2;
		}

		task_sleep(1000);
	}

	return 0;
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
	current->event.func = task_null_func;

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;

	fstate = (const uint8_t *)current + 0x0C00;
	memcpy(&task_default_fstate[0], fstate, 512);

	task_head = (task_tail = current);
	task_struct_count = 1;

	kernel->schedule = task_schedule_default;

	ap_count = (uint32_t)kernel->smp_ap_count;
	cpu_write32((uint32_t *)&task_ap_sync, 1);

	while (cpu_read32((const uint32_t *)&task_ap_count) != ap_count)
		delay(1000000);

	if (!task_create(task_caretaker, NULL, task_normal))
		return DE_MEMORY;

	cpu_write32((uint32_t *)&task_ready, 1);

	return 0;
}

int task_init_ap(void)
{
	struct task *current;

	while (cpu_read32((const uint32_t *)&task_ap_sync) == 0)
		delay(1000000);

	current = memset((void *)task_current(), 0, 0x1000);
	current->cr3 = (uint32_t)pg_kernel;
	current->id = task_create_id();
	current->event.func = task_null_func;

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;
	task_append(current);

	spin_lock(&task_lock);

	task_ap_count += 1;
	task_struct_count += 1;

	if (task_ap_count > kernel->smp_ap_count)
		panic("task_init_ap: called too many times");

	spin_unlock(&task_lock);

	while (cpu_read32((const uint32_t *)&task_ready) == 0)
		delay(1000000);

	return 0;
}

struct task *task_find(uint64_t id)
{
	struct task *t = task_head;
	struct task *r = NULL;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		panic("Enumerating task structs while interrupts disabled.");

	if (id != 0 && t != NULL) {
		/*
		 * Do not disable interrupts. Task switching must be
		 * temporarily suspended.
		 */
		task_switch_disable();
		spin_lock(&task_lock);

		/*
		 * The circular linked list must be fully valid when the
		 * task lock is acquired. No other code can modify task
		 * structure identifications or the linked list.
		 */
		do {
			if (t->id == id) {
				r = t;
				break;
			}

			t = task_read_next(t);

		} while (t != task_head);

		spin_unlock(&task_lock);
		task_switch_enable();
	}

	/*
	 * The returned task structure is either NULL or the one that
	 * has or had the specific identification. The caller must lock
	 * the structure and check it again.
	 */
	return r;
}

uint64_t task_create(int (*func)(void *), void *arg, int type)
{
	uint64_t id = 0;
	struct task *new_task = NULL;
	uint8_t *fstate;

	if (task_pool_head)
		new_task = task_create_from_pool();

	if (!new_task) {
		void *lock_local = &task_lock;
		int task_overflow = 0;

		new_task = (struct task *)mm_alloc_pages(mm_kernel, 1);

		if (!new_task)
			return id;

		memset(new_task, 0, 0x2000);
		spin_trylock(&new_task->active);

		spin_enter(&lock_local);

		if (task_struct_count < task_struct_limit)
			task_struct_count += 1;
		else
			task_overflow = 1;

		spin_leave(&lock_local);

		if (task_overflow) {
			mm_free_pages((phys_addr_t)new_task, 1);
			return id;
		}

		new_task->id = task_create_id();
		new_task->id_owner = task_current()->id;

		task_append(new_task);
	}

	id = new_task->id;

	new_task->cr3 = (uint32_t)pg_kernel;
	new_task->event.func = task_null_func;

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

int task_list(struct task_list_entry *buf, size_t buf_size)
{
	struct task *t = task_head;
	size_t max_count = buf_size / sizeof(*buf);
	int count = 0;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		panic("Enumerating task structs while interrupts disabled.");

	if (!buf || !buf_size) {
		void *lock_local = &task_lock;

		spin_enter(&lock_local);
		count = task_struct_count - task_pool_count;
		spin_leave(&lock_local);

		return count;
	}

	memset(buf, 0, buf_size);

	if (t != NULL) {
		/*
		 * Do not disable interrupts. Task switching must be
		 * temporarily suspended.
		 */
		task_switch_disable();
		spin_lock(&task_lock);

		/*
		 * The circular linked list must be fully valid when the
		 * task lock is acquired. No other code can modify task
		 * structure identifications or the linked list.
		 */
		do {
			if ((size_t)count < max_count) {
				buf[count].id = t->id;
				buf[count].id_owner = t->id_owner;

				if (t->event.func != task_null_func)
					buf[count].event_active = 1;

				buf[count].stopped = t->stopped;
			}

			count += 1;
			t = task_read_next(t);

		} while (t != task_head);

		spin_unlock(&task_lock);
		task_switch_enable();
	}

	return count;
}

int task_read_event(void)
{
	struct task *current = task_current();
	int r0, r1;

	r0 = cpu_ints(0);

	if ((r1 = current->event.func(&current->event.data[0])) == 0) {
		current->event.data[0] = 0;
		current->event.data[1] = 0;
		current->event.func = task_null_func;
	}

	cpu_ints(r0);

	return r1;
}

void task_write_event(int (*func)(uint64_t *data), uint64_t d0, uint64_t d1)
{
	struct task *current = task_current();
	int r;

	r = cpu_ints(0);

	current->event.data[0] = d0;
	current->event.data[1] = d1;
	current->event.func = func;

	cpu_ints(r);
}

void task_exit(int retval)
{
	struct task *current = task_current();
	struct task *next = task_read_next(current);

	if (!current->id_owner)
		panic("task_exit: system task stopped");

	current->retval = retval;
	current->stopped = 1;

	pg_delete();

	while (task_switch(next))
		next = !next ? task_read_next(current) : task_read_next(next);

	panic("task_exit: unexpected behavior");
}

void task_jump(addr_t user_ip, addr_t user_sp)
{
	extern void task_jump_asm(addr_t ip, addr_t cs, addr_t sp, addr_t ss);

	addr_t cs = gdt_user_code | 3;
	addr_t ss = gdt_user_data | 3;

	task_jump_asm(user_ip, cs, user_sp, ss);
}

static int task_sleep_func(uint64_t *data)
{
	return (data[0] > timer_read());
}

void task_sleep(uint64_t milliseconds)
{
	uint64_t data = timer_read() + milliseconds;

	if (data < milliseconds)
		data = (uint64_t)(ULLONG_MAX);

	task_write_event(task_sleep_func, data, 0);

	do {
		task_yield();
	} while (task_read_event());
}

int task_switch(struct task *next)
{
	void *tss;
	int r;

	/*
	 * The next argument can be NULL. Otherwise, do a quick test for
	 * the "next->active" and "next->stopped" members. These do not
	 * have to be "thread-safe".
	 */
	if (!next || next->active || next->stopped)
		return 1;

	/*
	 * Disable interrupts and save the previous status. This is very
	 * important and essential for many reasons.
	 */
	r = cpu_ints(0);

	tss = gdt_get_tss();

	/*
	 * This quick test does not have to be "thread-safe" either.
	 */
	if (next->uniproc && tss != task_uniproc_tss) {
		cpu_ints(r);
		return 1;
	}

	/*
	 * Try to acquire the "ownership" of the next task. If it succeeds,
	 * then this thread of execution "owns" two locked task structures.
	 */
	if (!spin_trylock(&next->active)) {
		cpu_ints(r);
		return 1;
	}

	if (next->stopped) {
		spin_unlock(&next->active);
		cpu_ints(r);
		return 1;
	}

	/*
	 * Typically all processors are allowed to execute the task, but
	 * some special tasks are allowed only on the bootstrap processor.
	 */
	if (next->uniproc && tss != task_uniproc_tss) {
		spin_unlock(&next->active);
		cpu_ints(r);
		return 1;
	}

	/*
	 * Call the event function if that has been set. If the function
	 * returns a non-zero value, this task switching can be skipped.
	 */
	if (next->event.func != task_null_func) {
		if (next->event.func(&next->event.data[0])) {
			spin_unlock(&next->active);
			cpu_ints(r);
			return 1;
		}

		next->event.data[0] = 0;
		next->event.data[1] = 0;
		next->event.func = task_null_func;
	}

	/*
	 * Call the assembly function, which will take care of the rest.
	 */
	task_switch_asm(next, tss);

	/*
	 * Restore the interrupts like they were. This step exactly here
	 * is more fundamental for multitasking than it might seem.
	 */
	cpu_ints(r);

	return 0;
}

int task_trywait(uint64_t id, int *retval)
{
	struct task *t;
	int r = 0;

	if (retval)
		*retval = r;

	if ((t = task_find(id)) == NULL)
		return DE_ARGUMENT;

	if (!t->stopped || !spin_trylock(&t->active))
		return DE_RETRY;

	if (!t->stopped) {
		spin_unlock(&t->active);
		return DE_RETRY;
	}

	if (t->id != id || t->detached != 0) {
		spin_unlock(&t->active);
		return DE_ARGUMENT;
	}

	r = t->retval;
	t->detached = 1;

	spin_unlock(&t->active);

	if (retval)
		*retval = r;

	return 0;
}

static int task_wait_func(uint64_t *data)
{
	struct task *t = (struct task *)((addr_t)data[0]);
	unsigned int ticks0 = (unsigned int)data[1];
	unsigned int ticks1 = (unsigned int)timer_ticks;

	if (!t->active && t->stopped)
		return 0;

	if ((ticks1 - ticks0) >= 2000)
		return 0;

	return 1;
}

int task_wait(uint64_t id, int *retval)
{
	struct task *t;
	int r = 0;

	if (retval)
		*retval = r;

	if ((t = task_find(id)) == NULL)
		return DE_ARGUMENT;

	for (;;) {
		void *lock_local;
		int id_changed;
		uint64_t d0, d1;

		if (t->stopped && spin_trylock(&t->active)) {
			if (t->stopped)
				break;
			spin_unlock(&t->active);
		}

		lock_local = &task_lock;
		spin_enter(&lock_local);

		id_changed = (t->id != id);

		spin_leave(&lock_local);

		if (id_changed)
			return DE_ARGUMENT;

		d0 = (uint64_t)((addr_t)t);
		d1 = (uint64_t)timer_ticks;
		task_write_event(task_wait_func, d0, d1);

		do {
			task_yield();
		} while (task_read_event());
	}

	if (t->id != id || t->detached != 0) {
		spin_unlock(&t->active);
		return DE_ARGUMENT;
	}

	r = t->retval;
	t->detached = 1;

	spin_unlock(&t->active);

	if (retval)
		*retval = r;

	return 0;
}

void task_yield(void)
{
	if (!task_ready)
		return;

	kernel->schedule();
}
