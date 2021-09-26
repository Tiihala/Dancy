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
 * base/event.c
 *      Kernel event services
 */

#include <dancy.h>

struct event_instance {
	int ready;
	int lock;
	int manual_reset;
	int signaled;
};

#define EVENT_READY (0x00657665)

#define null_event (event == NULL)
#define this_event ((struct event_instance *)(event))

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
	void *lock_local;

	if (null_event || this_event->ready != EVENT_READY)
		return;

	lock_local = &this_event->lock;

	spin_enter(&lock_local);
	this_event->signaled = 1;
	spin_leave(&lock_local);
}

static int event_wait_func(uint64_t *data)
{
	void *event = (void *)data[0];

	if (this_event->signaled == 1 || data[1] <= timer_read())
		return 0;

	return 1;
}

int event_wait(event_t event, uint16_t milliseconds)
{
	struct task *current = task_current();
	uint64_t data0 = (uint64_t)event;
	uint64_t data1 = timer_read() + (uint64_t)milliseconds;
	int r = 1;

	if (null_event || this_event->ready != EVENT_READY)
		return 1;

	for (;;) {
		void *lock_local = &this_event->lock;

		spin_enter(&lock_local);

		if (this_event->signaled) {
			if (!this_event->manual_reset)
				this_event->signaled = 0;
			r = 0;
		}

		spin_leave(&lock_local);

		if (r == 0 || data1 <= timer_read())
			break;

		cpu_write64(&current->event.data[0], data0);
		cpu_write64(&current->event.data[1], data1);
		current->event.func = event_wait_func;

		do {
			task_yield();
		} while (current->event.func(&current->event.data[0]));
	}

	return r;
}
