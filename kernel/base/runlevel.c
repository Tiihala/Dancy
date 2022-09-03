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
 * base/runlevel.c
 *      Mode of operation
 */

#include <dancy.h>

static uint32_t runlevel_id[2] = { 1, 1 };

static int runlevel_lock = 0;
static event_t runlevel_event = NULL;

static void runlevel_prepare_shutdown(void)
{
	int i;

	task_switch_disable();

	if (apic_id() != kernel->apic_bsp_id)
		cpu_halt(0);

	if (!spin_trylock(&panic_lock))
		cpu_halt(0);

	/*
	 * Send a non-maskable interrupt to every
	 * application processor.
	 */
	for (i = 0; i < kernel->smp_ap_count; i++) {
		uint32_t ap_id = kernel->smp_ap_id[i];
		uint32_t icr_low, icr_high;
		int wait_ms = 10000;

		if (ap_id > 0xFE)
			continue;

		kernel->smp_ap_state[ap_id] = 1;

		icr_low = 0x00004400;
		icr_high = ap_id << 24;

		apic_send(icr_low, icr_high);

		while (kernel->smp_ap_state[ap_id] != 2) {
			if (--wait_ms == 0) {
				spin_unlock(&panic_lock);
				panic("runlevel_prepare_shutdown: watchdog");
			}

			if ((wait_ms % 1000) == 0)
				apic_send(icr_low, icr_high);

			delay(1000000);
		}
	}

	apic_wait_delivery();
	delay(1000000);

	spin_unlock(&panic_lock);

	task_switch_enable();
	task_yield();
}

static void runlevel_shutdown(void)
{
	int i;
	int (*func)(void);

#ifdef DANCY_32
	const char *name = "_acpios_shutdown";
#else
	const char *name = "acpios_shutdown";
#endif

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strcmp(kernel->symbol[i].name, name)) {
			addr_t a = (addr_t)kernel->symbol[i].value;

			func = (int (*)(void))a;
			func();
			break;
		}
	}

	task_switch_disable();
	con_panic("It is safe to shut down the computer.");

	cpu_halt(0);
}

static void runlevel_reset(void)
{
	int i;
	int (*func)(void);

#ifdef DANCY_32
	const char *name = "_acpios_reset";
#else
	const char *name = "acpios_reset";
#endif

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strcmp(kernel->symbol[i].name, name)) {
			addr_t a = (addr_t)kernel->symbol[i].value;

			func = (int (*)(void))a;
			func();
			break;
		}
	}

	task_switch_disable();

	/*
	 * Try to use the ACPI reset register "manually" if the
	 * acpios_reset function was not available or did not work.
	 */
	if (kernel->acpi_enabled && kernel->acpi->fadt_addr) {
		phys_addr_t addr = kernel->acpi->fadt_addr;
		uint64_t length = pg_read_memory(addr + 4, 4);

		if (length > 128) {
			phys_addr_t reset_reg_offset = addr + 116;
			phys_addr_t reset_val_offset = addr + 128;
			uint64_t addr_space_id, reset_val;
			uint64_t a_lo, a_hi;

			addr_space_id = pg_read_memory(reset_reg_offset, 1);
			reset_val = pg_read_memory(reset_val_offset, 1);

			a_lo = pg_read_memory(reset_reg_offset + 4, 4);
			a_hi = pg_read_memory(reset_reg_offset + 8, 4);

			addr = (phys_addr_t)(((a_hi << 16) << 16) | a_lo);

			/*
			 * System memory space.
			 */
			if (addr_space_id == 0)
				pg_write_memory(addr, reset_val, 1);

			/*
			 * System I/O space.
			 */
			if (addr_space_id == 1)
				cpu_out8((uint16_t)addr, (uint8_t)reset_val);
		}
	}

	/*
	 * Disable interrupts and load a zero-length IDT.
	 */
	{
		static uint32_t null_idt[3];

		cpu_ints(0);
		idt_load(&null_idt[0]);
	}

	/*
	 * Try to use the "8042" method. The existence of the controller
	 * is not checked because the state of the system is ready for a
	 * triple fault.
	 */
	for (i = 0; i < 16; i++) {
		cpu_out8(0x64, 0xFE);
		delay(10000000);
	}

	/*
	 * Enable interrupts and let a timer interrupt (or some other)
	 * to triple fault the CPU.
	 */
	cpu_ints(1);

	con_panic("It is safe to reset the computer.");
	cpu_halt(0);
}

static int runlevel_task(void *arg)
{
	uint32_t id;

	while (!arg) {
		id = cpu_read32(&runlevel_id[1]);

		if (id == cpu_read32(&runlevel_id[0])) {
			event_wait(runlevel_event, 0xFFFF);
			continue;
		}

		cpu_write32(&runlevel_id[0], id);

		if (id == 0) {
			runlevel_prepare_shutdown();
			runlevel_shutdown();
		}

		if (id == 6) {
			runlevel_prepare_shutdown();
			runlevel_reset();
		}
	}

	return 0;
}

int runlevel_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	runlevel_event = event_create(0);

	if (!runlevel_event)
		return DE_MEMORY;

	if (!task_create(runlevel_task, NULL, task_detached | task_uniproc))
		return DE_MEMORY;

	return 0;
}

int runlevel_current(void)
{
	return (int)cpu_read32(&runlevel_id[0]);
}

int runlevel_send_request(int id)
{
	/*
	 * Run levels from 1 to 5.
	 */
	if (id >= 1 && id <= 5) {
		while (!spin_trylock(&runlevel_lock))
			task_yield();

		cpu_write32(&runlevel_id[1], (uint32_t)id);
		event_signal(runlevel_event);

		while (runlevel_current() != id)
			task_yield();

		spin_unlock(&runlevel_lock);
		return 0;
	}

	/*
	 * Shut down or reset the computer.
	 */
	if (id == 0 || id == 6) {
		const uint64_t forever = (uint64_t)ULLONG_MAX;

		while (!spin_trylock(&runlevel_lock))
			task_yield();

		cpu_write32(&runlevel_id[1], (uint32_t)id);
		event_signal(runlevel_event);

		task_sleep(forever);
		panic("runlevel_send_request: unexpected behavior");
	}

	return DE_ARGUMENT;
}
