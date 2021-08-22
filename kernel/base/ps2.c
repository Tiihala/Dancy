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
 * base/ps2.c
 *      PS/2 Controller
 */

#include <dancy.h>

static int ps2_lock;
static int ps2_ready;

static void ps2_default_callback(uint8_t val)
{
	(void)val;
}

void (*ps2_receive_port1)(uint8_t val) = ps2_default_callback;
void (*ps2_receive_port2)(uint8_t val) = ps2_default_callback;

static void ps2_irq_func(int irq, void *arg)
{
	(void)arg;

	if (!ps2_ready)
		return;

	if (irq == 1) {
		ps2_receive_port1(cpu_in8(0x60));
		return;
	}

	if (irq == 12) {
		ps2_receive_port2(cpu_in8(0x60));
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

int ps2_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	/*
	 * The init module is responsible for initializing all the USB
	 * controllers and disabling the "USB Legacy Support" features
	 * before executing the kernel base module.
	 *
	 * The ACPI table (FADT) has an "IA-PC Boot Architecture Flags"
	 * which indicates the availability of an "8042 Controller". If
	 * there is no FADT, the controller should exist.
	 */
	if (kernel->acpi && kernel->acpi->fadt_addr) {
		const unsigned int flag_8042 = 2;

		/*
		 * The flag is set if the controller is available.
		 */
		if (!(kernel->acpi->iapc_boot_arch & flag_8042))
			return 0;
	}

	/*
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
	}

	return 0;
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
