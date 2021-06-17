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

		if (ap_id > 0xFE)
			continue;

		icr_low = 0x00004400;
		icr_high = ap_id << 24;

		apic_send(icr_low, icr_high);
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
	con_panic("It is safe to reset the computer.");

	cpu_halt(0);
}

static int runlevel_task(void *arg)
{
	uint32_t id;

	while (!arg) {
		id = cpu_read32(&runlevel_id[1]);

		if (id == cpu_read32(&runlevel_id[0])) {
			task_yield();
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

		spin_unlock(&runlevel_lock);
	}

	return 0;
}

int runlevel_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (!task_create(runlevel_task, NULL, task_detached | task_uniproc))
		return DE_MEMORY;

	return 0;
}

int runlevel_current(void)
{
	return (int)cpu_read32(&runlevel_id[0]);
}

int runlevel_set(int id)
{
	if (!spin_trylock(&runlevel_lock))
		return DE_RETRY;

	/*
	 * Shut down the computer.
	 */
	if (id == 0) {
		cpu_write32(&runlevel_id[1], 0);
		return 0;
	}

	/*
	 * Normal mode.
	 */
	if (id == 1) {
		cpu_write32(&runlevel_id[1], 1);
		return 0;
	}

	/*
	 * Reset the computer.
	 */
	if (id == 6) {
		cpu_write32(&runlevel_id[1], 6);
		return 0;
	}

	return DE_ARGUMENT;
}
