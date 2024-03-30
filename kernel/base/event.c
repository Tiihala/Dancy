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
 * base/event.c
 *      Kernel event services
 */

#include <dancy.h>

struct event_instance {
	int ready;
	int lock;
	int manual_reset;
	int signaled;
	cpu_native_t task;
};

#define EVENT_READY (0x00657665)

#define null_event (event == NULL)
#define this_event ((struct event_instance *)(event))

static cpu_native_t yield_task = 0;

event_t event_create(int type)
{
	const size_t size = sizeof(struct event_instance);
	void *event;

	if ((event = malloc(size)) != NULL) {
		memset(event, 0, size);

		this_event->ready = EVENT_READY;

		if ((type & event_type_manual_reset) != 0)
			this_event->manual_reset = 1;

		if ((type & event_type_set_signaled) != 0)
			this_event->signaled = 1;
	}

	return (event_t)event;
}

void event_delete(event_t event)
{
	if (null_event || this_event->ready != EVENT_READY)
		return;

	this_event->ready = 0;
	free((void *)event);
}

void event_reset(event_t event)
{
	void *lock_local;

	if (null_event || this_event->ready != EVENT_READY)
		return;

	lock_local = &this_event->lock;

	spin_enter(&lock_local);
	this_event->signaled = 0;
	spin_leave(&lock_local);
}

void event_signal(event_t event)
{
	cpu_native_t task = 0;
	void *lock_local;

	if (null_event || this_event->ready != EVENT_READY)
		return;

	lock_local = &this_event->lock;

	spin_enter(&lock_local);

	this_event->signaled = 1;
	task = cpu_xchg(&this_event->task, task);
	cpu_xchg(&yield_task, task);

	spin_leave(&lock_local);
}

int event_wait(event_t event, uint16_t milliseconds)
{
	event_t events[1] = { event };

	return event_wait_array(1, &events[0], milliseconds);
}

struct event_wait_array_data {
	int count;
	event_t *events;
	uint32_t ticks[2];
};

static int event_wait_array_func(uint64_t *data)
{
	struct event_wait_array_data *d = (void *)((cpu_native_t)data[0]);
	int i;

	for (i = 0; i < d->count; i++) {
		event_t event = d->events[i];

		if (this_event->signaled == 1)
			return 0;
	}

	if (d->ticks[1] != 0xFFFF) {
		if ((timer_ticks - d->ticks[0]) >= d->ticks[1])
			return 0;
	}

	return 1;
}

int event_wait_array(int count, event_t *events, uint16_t milliseconds)
{
	cpu_native_t task = (cpu_native_t)task_current();
	struct event_wait_array_data data;
	int i, r = -1;

	if (count <= 0 || count > 256)
		return -2;

	data.count = count;
	data.events = events;
	data.ticks[0] = timer_ticks;
	data.ticks[1] = milliseconds;

	for (i = 0; i < count; i++) {
		event_t event = events[i];

		if (null_event || this_event->ready != EVENT_READY)
			return -3;
	}

	for (;;) {
		uint64_t data0, data1;

		for (i = 0; i < count && r < 0; i++) {
			event_t event = events[i];
			void *lock_local = &this_event->lock;

			spin_enter(&lock_local);

			if (this_event->signaled) {
				if (!this_event->manual_reset)
					this_event->signaled = 0;
				r = (int)i;
			}

			cpu_xchg(&this_event->task, task);
			spin_leave(&lock_local);
		}

		if (r >= 0 || milliseconds == 0)
			break;

		if (milliseconds != 0xFFFF) {
			if ((timer_ticks - data.ticks[0]) >= data.ticks[1])
				break;
		}

		data0 = (uint64_t)((addr_t)&data), data1 = 0;

		task_write_event(event_wait_array_func, data0, data1);

		do {
			task_yield();
		} while (task_read_event());
	}

	return r;
}

void event_yield(void)
{
	cpu_native_t task = 0;

	if ((task = cpu_xchg(&yield_task, task)) != 0) {
		int i;

		if ((struct task *)task == task_current())
			return;

		if (!task_switch((struct task *)task))
			return;

		if (!kernel->smp_ap_count)
			return;

		for (i = -1; i < kernel->smp_ap_count; i++) {
			const uint32_t event_vector = 0x5F;
			uint32_t id, icr_low, icr_high;

			if (i < 0)
				id = kernel->apic_bsp_id;
			else
				id = kernel->smp_ap_id[i];

			if (id > 0xFE)
				continue;

			icr_low = 0x00004000 | event_vector;
			icr_high = id << 24;

			apic_send(icr_low, icr_high);
		}
	}
}
