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
 * ps2/8042.c
 *      PS/2 Controller
 */

#include <dancy.h>

event_t ps2_event_port1;
event_t ps2_event_port2;

static int ps2_lock;
static int ps2_ready;

#define PS2_BUFFER_SIZE (0x2000)

/*
 * Circular buffer. New data will overwrite
 * existing data if the buffer is full.
 */
static struct {
	int start;
	int end;
	uint8_t base[PS2_BUFFER_SIZE];
} ps2_buffer[2];

static void ps2_irq_func(int irq, void *arg)
{
	int start, end;

	if (!ps2_ready) {
		(void)arg;
		return;
	}

	if (irq == 1) {
		spin_lock(&ps2_lock);

		end = ps2_buffer[0].end;

		ps2_buffer[0].base[end] = cpu_in8(0x60);
		ps2_buffer[0].end = (end = (end + 1) % PS2_BUFFER_SIZE);

		if ((start = ps2_buffer[0].start) == end)
			ps2_buffer[0].start = (start + 1) % PS2_BUFFER_SIZE;

		event_signal(ps2_event_port1);
		spin_unlock(&ps2_lock);

		return;
	}

	if (irq == 12) {
		spin_lock(&ps2_lock);

		end = ps2_buffer[1].end;

		ps2_buffer[1].base[end] = cpu_in8(0x60);
		ps2_buffer[1].end = (end = (end + 1) % PS2_BUFFER_SIZE);

		if ((start = ps2_buffer[1].start) == end)
			ps2_buffer[1].start = (start + 1) % PS2_BUFFER_SIZE;

		event_signal(ps2_event_port2);
		spin_unlock(&ps2_lock);

		return;
	}
}

static void ps2_delay(void)
{
	delay(1000000);
}

static void ps2_clear_status(int bit)
{
	const int repeat = 512;
	int i;

	/*
	 * Output buffer status (bit 0).
	 */
	if (bit == 0) {
		for (i = 0; i < repeat; i++) {
			if ((cpu_in8(0x64) & 1u) == 0)
				return;

			(void)cpu_in8(0x60);
			ps2_delay();
		}
	}

	/*
	 * Input buffer status (bit 1).
	 */
	if (bit == 1) {
		for (i = 0; i < repeat; i++) {
			if ((cpu_in8(0x64) & 2u) == 0)
				return;

			ps2_delay();
		}
	}
}

static unsigned int ps2_read_config(void)
{
	const int repeat = 512;
	int i;

	ps2_clear_status(1), cpu_out8(0x64, 0x20), ps2_delay();

	for (i = 0; i < repeat; i++) {
		if ((cpu_in8(0x64) & 1u) != 0)
			return (unsigned int)cpu_in8(0x60);
		ps2_delay();
	}

	return UINT_MAX;
}

static int ps2_task(void *arg)
{
	event_t events[2] = { ps2_event_port1, ps2_event_port2 };

	task_set_cmdline(task_current(), NULL, "[ps2]");

	while (!arg) {
		int r = event_wait_array(2, &events[0], 2500);

		if (r < -1)
			panic("ps2_task: unexpected behavior");
		else if (r == -1)
			ps2_kbd_probe(), ps2_mse_probe();
		else if (r == 0)
			ps2_kbd_handler();
		else if (r == 1)
			ps2_mse_handler();
	}

	return 0;
}

int ps2_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	/*
	 * Create the events for special key combinations.
	 */
	if ((kernel->keyboard.ctrl_alt_del_event = event_create(0)) == NULL)
		return DE_MEMORY;
	if ((kernel->keyboard.console_switch_event = event_create(0)) == NULL)
		return DE_MEMORY;

	/*
	 * The init module is responsible for initializing all the USB
	 * controllers and disabling the "USB Legacy Support" features
	 * before executing this module.
	 *
	 * Install IRQ handlers for the both PS/2 ports.
	 */
	{
		const int port1 = 1;
		const int port2 = 12;

		if (!irq_install(port1, NULL, ps2_irq_func))
			return DE_UNEXPECTED;

		if (!irq_install(port2, NULL, ps2_irq_func))
			return DE_UNEXPECTED;

		irq_enable(port1);
		irq_enable(port2);
	}

	ps2_clear_status(0);

	/*
	 * Disable PS/2 devices.
	 */
	ps2_clear_status(1), cpu_out8(0x64, 0xAD), ps2_delay();
	ps2_clear_status(1), cpu_out8(0x64, 0xA7), ps2_delay();

	/*
	 * Clear both buffers and do one extra read from the data port.
	 */
	ps2_clear_status(1);
	ps2_clear_status(0);
	(void)cpu_in8(0x60);

	/*
	 * Modify the PS/2 Controller configuration byte.
	 */
	{
		unsigned int val;

		if ((val = ps2_read_config()) == UINT_MAX)
			return DE_UNEXPECTED;

		/*
		 * Enable IRQs and "port translation".
		 */
		val |= 0x43u;

		/*
		 * Write the modified configuration byte.
		 */
		ps2_clear_status(1), cpu_out8(0x64, 0x60);
		ps2_delay();
		ps2_clear_status(1), cpu_out8(0x60, (uint8_t)val);
		ps2_delay();

		/*
		 * Check that the modified byte has been written.
		 */
		if ((val = ps2_read_config()) == UINT_MAX)
			return DE_UNEXPECTED;

		if ((val & 0x43u) != 0x43u)
			return DE_UNEXPECTED;
	}

	/*
	 * Create the PS/2 event objects.
	 */
	ps2_event_port1 = event_create(event_type_manual_reset);
	ps2_event_port2 = event_create(event_type_manual_reset);

	if (!ps2_event_port1 || !ps2_event_port2)
		return DE_MEMORY;

	/*
	 * Enable PS/2 devices.
	 */
	ps2_clear_status(1), cpu_out8(0x64, 0xAE), ps2_delay();
	ps2_clear_status(1), cpu_out8(0x64, 0xA8), ps2_delay();

	cpu_write32((uint32_t *)&ps2_ready, 1);

	/*
	 * Clear both buffers and do a few extra reads from the data port.
	 */
	{
		void *lock_local = &ps2_lock;
		int i;

		spin_enter(&lock_local);

		ps2_clear_status(1);
		ps2_clear_status(0);

		for (i = 0; i < 4; i++)
			(void)cpu_in8(0x60);

		spin_leave(&lock_local);

		for (;;) {
			int r1 = ps2_receive_port1();
			int r2 = ps2_receive_port2();

			if (r1 < 0 && r2 < 0)
				break;
		}
	}

	/*
	 * Initialize PS/2 devices.
	 */
	(void)ps2_kbd_init();
	(void)ps2_mse_init();

	/*
	 * Create a task for handling the PS/2 devices.
	 */
	if (!task_create(ps2_task, NULL, task_detached))
		return DE_MEMORY;

	return 0;
}

int ps2_receive_port1(void)
{
	void *lock_local = &ps2_lock;
	int start, end;
	int r = -1;

	spin_enter(&lock_local);

	start = ps2_buffer[0].start;
	end = ps2_buffer[0].end;

	if (start != end) {
		r = (int)ps2_buffer[0].base[start];
		ps2_buffer[0].base[start] = 0;

		ps2_buffer[0].start = (start = (start + 1) % PS2_BUFFER_SIZE);
	}

	if (start == end)
		event_reset(ps2_event_port1);

	spin_leave(&lock_local);

	return r;
}

int ps2_receive_port2(void)
{
	void *lock_local = &ps2_lock;
	int start, end;
	int r = -1;

	spin_enter(&lock_local);

	start = ps2_buffer[1].start;
	end = ps2_buffer[1].end;

	if (start != end) {
		r = (int)ps2_buffer[1].base[start];
		ps2_buffer[1].base[start] = 0;

		ps2_buffer[1].start = (start = (start + 1) % PS2_BUFFER_SIZE);
	}

	if (start == end)
		event_reset(ps2_event_port2);

	spin_leave(&lock_local);

	return r;
}

int ps2_send_port1(uint8_t val)
{
	void *lock_local = &ps2_lock;

	if (!ps2_ready)
		return 1;

	spin_enter(&lock_local);

	ps2_clear_status(1), cpu_out8(0x60, val);

	spin_leave(&lock_local);

	return 0;
}

int ps2_send_port2(uint8_t val)
{
	void *lock_local = &ps2_lock;

	if (!ps2_ready)
		return 1;

	spin_enter(&lock_local);

	ps2_clear_status(1), cpu_out8(0x64, 0xD4), ps2_delay();
	ps2_clear_status(1), cpu_out8(0x60, val);

	spin_leave(&lock_local);

	return 0;
}
