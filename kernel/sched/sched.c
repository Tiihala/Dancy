/*
 * Copyright (c) 2021, 2024 Antti Tiihala
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

static void yield(void);

int sched_init(void)
{
	static int run_once;
	void (**yield_pointer)(void);

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	yield_pointer = &kernel->scheduler.yield;

	spin_unlock(&sched_lock);
	*yield_pointer = yield;

	return 0;
}

static void yield(void)
{
	static uint32_t count;

	struct task *current = task_current(), *next, *start;
	int current_priority = current->sched.priority;
	int current_runnable = !task_check_event(NULL);
	int search_task = 1;

	if (!spin_trylock(&current->sched.lock))
		return;

	/*
	 * Make sure that all tasks will get a chance to run.
	 */
	if ((cpu_add32(&count, 1) & 0x1F) == 0) {
		struct task *next_starved = NULL;

		next = start = current;

		while ((next = task_read_next(next)) && next != start) {
			uint32_t state = next->sched.state;

			if (!task_check_event(next)) {
				if (!next_starved)
					next_starved = next;
				if (next_starved->sched.state < state)
					next_starved = next;
			}
		}

		if (next_starved) {
			next_starved->sched.state = 0;

			if (!task_switch(next_starved)) {
				spin_unlock(&current->sched.lock);
				return;
			}
		}
	}

	/*
	 * The loop for finding the next task. There are four priority levels.
	 */
	while ((search_task & 1) != 0 && search_task < 8) {
		struct task *next_kernel = NULL;
		struct task *next_high   = NULL;
		struct task *next_normal = NULL;
		struct task *next_low    = NULL;

		next = start = current;

		while ((next = task_read_next(next)) && next != start) {
			int priority;
			uint32_t state;

			if (next->stopped) {
				next->sched.state = 0;
				continue;
			}

			if (next->uniproc) {
				next->sched.state = 0;
				if (!task_switch(next)) {
					spin_unlock(&current->sched.lock);
					return;
				}
			}

			priority = next->sched.priority;
			state = cpu_add32(&next->sched.state, 1);

			if (priority == sched_priority_kernel) {
				if (!task_check_event(next)) {
					if (!next_kernel)
						next_kernel = next;
					if (next_kernel->sched.state < state)
						next_kernel = next;
				}
			}

			if (next_kernel)
				continue;

			if (priority == sched_priority_high) {
				if (!task_check_event(next)) {
					if (!next_high)
						next_high = next;
					if (next_high->sched.state < state)
						next_high = next;
				}
			}

			if (next_high)
				continue;

			if (priority == sched_priority_normal) {
				if (!task_check_event(next)) {
					if (!next_normal)
						next_normal = next;
					if (next_normal->sched.state < state)
						next_normal = next;
				}
			}

			if (next_normal)
				continue;

			if (priority == sched_priority_low) {
				if (!task_check_event(next)) {
					if (!next_low)
						next_low = next;
					if (next_low->sched.state < state)
						next_low = next;
				}
			}
		}

		search_task += 1;

		/*
		 * Case: sched_priority_kernel
		 */
		if (next_kernel) {
			next_kernel->sched.state = 0;

			if (!task_switch(next_kernel)) {
				spin_unlock(&current->sched.lock);
				return;
			}

			search_task += 1;
			continue;
		}

		if (current_runnable) {
			if (current_priority == sched_priority_kernel)
				break;
		}

		/*
		 * Case: sched_priority_high
		 */
		if (next_high) {
			next_high->sched.state = 0;

			if (!task_switch(next_high)) {
				spin_unlock(&current->sched.lock);
				return;
			}

			search_task += 1;
			continue;
		}

		if (current_runnable) {
			if (current_priority == sched_priority_high)
				break;
		}

		/*
		 * Case: sched_priority_normal
		 */
		if (next_normal) {
			next_normal->sched.state = 0;

			if (!task_switch(next_normal)) {
				spin_unlock(&current->sched.lock);
				return;
			}

			search_task += 1;
			continue;
		}

		if (current_runnable) {
			if (current_priority == sched_priority_normal)
				break;
		}

		/*
		 * Case: sched_priority_low
		 */
		if (next_low) {
			next_low->sched.state = 0;

			if (!task_switch(next_low)) {
				spin_unlock(&current->sched.lock);
				return;
			}

			search_task += 1;
			continue;
		}

		if (current_runnable) {
			if (current_priority == sched_priority_low)
				break;
		}
	}

	/*
	 * No other runnable tasks were found. Call the idle function if
	 * this current task can't be run either.
	 */
	if (!current_runnable)
		task_idle();

	spin_unlock(&current->sched.lock);
}
