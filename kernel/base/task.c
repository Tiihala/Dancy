/*
 * Copyright (c) 2021, 2022, 2023, 2024 Antti Tiihala
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

static void *task_append(struct task *new_task)
{
	void *lock_local = &task_lock;
	void *r = new_task;

	spin_enter(&lock_local);

	task_write_next(new_task, task_head);

	if (!kernel->rebooting)
		task_tail = task_write_next(task_tail, new_task);
	else
		r = NULL;

	spin_leave(&lock_local);

	return r;
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

	if (task_pool_head && !kernel->rebooting) {
		new_task = task_pool_head;

		task_pool_count -= 1;
		task_pool_head = task_read_next(task_pool_head);

		task_write_next(new_task, task_head);
		task_tail = task_write_next(task_tail, new_task);

		if (!new_task->active)
			panic("Error: inconsistent task structures");

		new_task->id = task_create_id();
		new_task->id_owner = task_current()->id;
		new_task->id_group = task_current()->id_group;
		new_task->id_session = task_current()->id_session;
		new_task->owner = task_current();
	}

	spin_leave(&lock_local);

	return new_task;
}

static int task_null_func(uint64_t *data)
{
	(void)data;
	return 0;
}

static int task_caretaker(void *arg)
{
	void *lock_local = &task_lock;

	task_current()->sched.priority = sched_priority_low;
	task_set_cmdline(task_current(), NULL, "[caretaker]");

	while (arg == NULL) {
		struct task *t = task_head;

		while (t != NULL) {
			struct task *owner = t->owner;
			struct task *t0 = t;
			struct task *t1 = task_read_next(t);
			struct task *t2;

			int error_assumption;
			unsigned char *p;
			size_t offset, size;

			/*
			 * Check that the owner task is still available. If
			 * not, set the default owner. The first quick test
			 * is done without acquiring the main task lock.
			 */
			if (owner && owner->id != t->id_owner) {
				spin_enter(&lock_local);

				if (t->owner && t->owner->id != t->id_owner) {
					t->id_owner = 1;
					t->owner = task_head;
				}

				spin_leave(&lock_local);
			}

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

			t1->sp = 0;
			t1->cr3 = 0;

			t1->asm_data1 = 0;
			t1->asm_data2 = 0;
			t1->asm_data3 = 0;

			t1->id = 0;
			t1->id_owner = 0;
			t1->id_group = 0;
			t1->id_session = 0;
			t1->owner = NULL;

			t1->detached = 0;
			t1->stopped = 0;

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

#ifdef DANCY_32
	task_struct_limit = 16384;
	pages = (int)mm_available_pages(mm_addr32);
#else
	task_struct_limit = 131072;
	pages = (int)mm_available_pages(mm_addr36);
#endif
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
	current->cr3 = (uint64_t)pg_kernel;
	current->id = task_create_id();
	current->id_group = 1;
	current->id_session = 1;
	current->event.func = task_null_func;

	if (current->id != 1 || current->id_owner != 0)
		return DE_UNEXPECTED;

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;

	fstate = (const uint8_t *)current + 0x0C00;
	memcpy(&task_default_fstate[0], fstate, 512);

	task_head = (task_tail = current);
	task_struct_count = 1;

	ap_count = (uint32_t)kernel->smp_ap_count;
	cpu_write32((uint32_t *)&task_ap_sync, 1);

	while (cpu_read32((const uint32_t *)&task_ap_count) != ap_count)
		delay(1000000);

	if (!task_create(task_caretaker, NULL, task_normal))
		return DE_MEMORY;

	kernel->scheduler.task_lock = &task_lock;
	kernel->scheduler.task_head = task_head;

	cpu_write32((uint32_t *)&task_ready, 1);
	task_set_cmdline(current, NULL, "/?");

	return 0;
}

int task_init_ap(void)
{
	struct task *current;

	while (cpu_read32((const uint32_t *)&task_ap_sync) == 0)
		delay(1000000);

	current = memset((void *)task_current(), 0, 0x1000);
	current->cr3 = (uint64_t)pg_kernel;
	current->id = task_create_id();
	current->id_owner = 1;
	current->id_group = 1;
	current->id_session = 1;
	current->owner = task_head;
	current->event.func = task_null_func;

	task_switch_asm(current, gdt_get_tss());
	current->active = 1;
	current->detached = 1;
	task_append(current);

	spin_lock(&task_lock);

	task_ap_count += 1;
	task_struct_count += 1;

	if (task_ap_count > kernel->smp_ap_count)
		panic("task_init_ap: called too many times");

	spin_unlock(&task_lock);

	while (cpu_read32((const uint32_t *)&task_ready) == 0)
		delay(1000000);

	task_set_cmdline(current, NULL, "[cpu]");

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
	struct task *current = task_current();
	struct task *new_task = NULL;
	uint8_t *fstate;

	if (task_pool_head)
		new_task = task_create_from_pool();

	if (!new_task) {
		void *lock_local = &task_lock;
		int task_allowed = 0;

		spin_enter(&lock_local);

		if (!kernel->rebooting) {
			if (task_struct_count < task_struct_limit) {
				task_struct_count += 1;
				task_allowed = 1;
			}
		}

		spin_leave(&lock_local);

		if (!task_allowed)
			return id;
#ifdef DANCY_32
		new_task = (struct task *)mm_alloc_pages(mm_kernel, 1);
#else
		new_task = (struct task *)mm_alloc_pages(mm_addr36, 1);

		if (new_task) {
			const size_t size = 0x2000;
			const addr_t high_bit = 0x8000000000000000ull;

			phys_addr_t nt = (phys_addr_t)new_task;
			addr_t vaddr;

			pg_enter_kernel();
			vaddr = (addr_t)pg_map_kernel(nt, size, pg_extended);
			pg_leave_kernel();

			if ((vaddr & high_bit) == 0) {
				mm_free_pages((phys_addr_t)new_task, 1);
				vaddr = 0;
			}
			new_task = (struct task *)vaddr;
		}
#endif
		if (!new_task) {
			spin_enter(&lock_local);
			task_struct_count -= 1;
			spin_leave(&lock_local);
			return id;
		}

		memset(new_task, 0, 0x2000);
		spin_trylock(&new_task->active);

		new_task->id = task_create_id();

		spin_enter(&lock_local);
		new_task->id_owner = current->id;
		new_task->id_group = current->id_group;
		new_task->id_session = current->id_session;
		spin_leave(&lock_local);

		new_task->owner = current;

		if (!task_append(new_task)) {
			/*
			 * Losing the newly allocated task structure is
			 * acceptable here. It can happen only if the
			 * kernel->rebooting flag is set while running
			 * this task_create function.
			 */
			return id;
		}
	}

	id = new_task->id;

	new_task->cr3 = (uint64_t)pg_kernel;
	new_task->event.func = task_null_func;

	fstate = (uint8_t *)new_task + 0x0C00;
	memcpy(fstate, &task_default_fstate[0], 512);

	if ((type & task_detached) != 0)
		new_task->detached = 1;

	if ((type & task_uniproc) != 0)
		new_task->uniproc = 1;

	new_task->sched.priority = current->sched.priority;

	if (current->fd.state)
		current->fd.clone(current, new_task);

	task_create_asm(new_task, func, arg);
	spin_unlock(&new_task->active);

	return id;
}

void task_access(void (*func)(struct task *, void *), void *arg)
{
	struct task *current = task_current();
	void *lock_local = &task_lock;

	spin_enter(&lock_local);
	func(current, arg);
	spin_leave(&lock_local);
}

void task_foreach(int (*func)(struct task *, void *), void *arg)
{
	struct task *current = task_current();
	struct task *t = task_head;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		panic("Enumerating task structs while interrupts disabled.");

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
			if (t != current && func(t, arg) != 0)
				break;

			t = task_read_next(t);

		} while (t != task_head);

		/*
		 * The current task structure is always the last one
		 * to be processed by the input function.
		 */
		func(current, arg);

		spin_unlock(&task_lock);
		task_switch_enable();
	}
}

void task_set_cmdline(struct task *task, void *line, const char *cline)
{
	void *lock_local = &task_lock;
	void *prev = NULL;

	spin_enter(&lock_local);

	if (task->cmd.line != &task->cmd._line[0])
		prev = task->cmd.line;

	task->cmd.line = line;

	if (line == NULL && cline != NULL) {
		int i = 0;
		char c;

		while (i < (TASK_CMD_STATIC_SIZE - 2)) {
			if ((c = cline[i]) == 0)
				break;
			task->cmd._line[i++] = (uint8_t)c;
		}

		task->cmd._line[i + 0] = 0;
		task->cmd._line[i + 1] = 0;
		task->cmd.line = &task->cmd._line[0];
	}

	spin_leave(&lock_local);

	if (prev != NULL)
		free(prev);
}

void task_prepare_rebooting(void)
{
	void *lock_local = &task_lock;

	spin_enter(&lock_local);
	kernel->rebooting = 1;
	spin_leave(&lock_local);
}

void task_identify(uint64_t *id, uint64_t *id_owner,
	uint64_t *id_group, uint64_t *id_session)
{
	struct task *current = task_current();
	void *lock_local = &task_lock;

	spin_enter(&lock_local);

	if (id)
		*id = current->id;

	if (id_owner)
		*id_owner = current->id_owner;

	if (id_group)
		*id_group = current->id_group;

	if (id_session)
		*id_session = current->id_session;

	spin_leave(&lock_local);
}

int task_check_event(struct task *task)
{
	struct task *current = task_current();
	int r = 0;

	if (!task || task == current) {
		if (current->event.func != task_null_func)
			r = current->event.func(&current->event.data[0]);
		return r;
	}

	if (task->active)
		return 1;

	if (task->event.func != task_null_func) {
		if (!spin_trylock(&task->active))
			return 1;

		r = task->event.func(&task->event.data[0]);
		spin_unlock(&task->active);
	}

	return r;
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

void task_idle(void)
{
	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) != 0)
		cpu_halt(1);
}

void task_exit(int retval)
{
	static const char *unexpected = "task_exit: unexpected behavior";
	struct task *current = task_current();

	if (current->fd.state)
		current->fd.release(current);

	if (current->sig.state)
		current->sig.send(current);

	pg_delete();

	if (current->id == 1) {
		int sig = (retval & 127);
		char buffer[64];

		while (!retval || sig == SIGKILL || sig == SIGTERM)
			task_wait_descendant(NULL, 0, NULL);

		if (sig)
			snprintf(&buffer[0], sizeof(buffer),
				"TASK ID 1, SIGNAL %d", sig);
		else
			snprintf(&buffer[0], sizeof(buffer),
				"TASK ID 1, ERROR %d", (retval >> 8) & 255);

		panic(&buffer[0]);
	}

	current->retval = retval;

	if (!current->id_owner || !spin_trylock(&current->stopped))
		panic(unexpected);

	cpu_add32(&current->owner->descendant.data[0], 1);

	while (current->stopped)
		task_yield();

	panic(unexpected);
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

static int task_wait_descendant_func(uint64_t *data)
{
	struct task *t = (struct task *)((addr_t)data[0]);
	unsigned int ticks0 = (unsigned int)data[1];
	unsigned int ticks1 = (unsigned int)timer_ticks;

	if (t->descendant.data[0] != t->descendant.data[1])
		return 0;

	if ((ticks1 - ticks0) >= 2000)
		return 0;

	return 1;
}

static int task_wait_descendant_shared(uint64_t *id, uint64_t id_group,
	int *retval, int mode_trywait)
{
	struct task *current = task_current();
	int descendant_state = 0;
	uint64_t in_id = 0;
	uint64_t out_id = 0;
	int out_retval = 0;
	int de_interrupt = 0;

	if (id)
		in_id = *id;

	for (;;) {
		struct task *t = task_head;

		current->descendant.data[1] = current->descendant.data[0];
		task_switch_disable();

		do {
			if (t->owner == current) {
				void *lock_local = &task_lock;
				int valid_candidate = 0;

				spin_enter(&lock_local);

				if (t->owner == current)
					valid_candidate = 1;

				if (in_id && in_id != t->id)
					valid_candidate = 0;

				if (id_group && id_group != t->id_group)
					valid_candidate = 0;

				descendant_state |= valid_candidate;

				if (valid_candidate && t->stopped) {
					descendant_state = 2;
					if (spin_trylock(&t->detached)) {
						out_id = t->id;
						out_retval = t->retval;
					}
				}

				spin_leave(&lock_local);
			}

			t = task_read_next(t);

		} while (out_id == 0 && t != NULL && t != task_head);

		task_switch_enable();

		if (out_id)
			break;

		if (t != NULL) {
			uint64_t d0, d1;

			if (!descendant_state || mode_trywait)
				break;

			if (current->asm_data3) {
				de_interrupt = 1;
				break;
			}

			d0 = (uint64_t)((addr_t)current);
			d1 = (uint64_t)timer_ticks;
			task_write_event(task_wait_descendant_func, d0, d1);

			do {
				task_yield();
			} while (task_read_event());
		}

		descendant_state = 0;
	}

	if (id)
		*id = out_id;

	if (retval)
		*retval = out_retval;

	if (de_interrupt)
		return DE_INTERRUPT;

	if (!out_id)
		return (descendant_state == 1) ? DE_RETRY : DE_EMPTY;

	return 0;
}

int task_trywait_descendant(uint64_t *id, uint64_t id_group, int *retval)
{
	return task_wait_descendant_shared(id, id_group, retval, 1);
}

int task_wait_descendant(uint64_t *id, uint64_t id_group, int *retval)
{
	return task_wait_descendant_shared(id, id_group, retval, 0);
}

void task_yield(void)
{
	int task_switch_disabled = (task_current()->asm_data1 > 1);

	if (task_switch_disabled)
		return;

	kernel->scheduler.yield();
}
