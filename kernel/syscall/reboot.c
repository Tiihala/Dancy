/*
 * Copyright (c) 2024, 2025 Antti Tiihala
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
 * syscall/reboot.c
 *      Reboot or power off the system
 */

#include <dancy.h>

static int f_send(struct task *task, void *arg)
{
	int sig = *((int *)arg) & 127;

	if ((*((int *)arg) & 128) == 0)
		task->uniproc = 1;

	if (sig > 0 && sig < 32) {
		void *address = &task->asm_data3;
		uint32_t value = (uint32_t)(sig - 1);

		cpu_bts32(address, value);
	}

	return 0;
}

static int f_check(struct task *task, void *arg)
{
	int *count = (int *)arg;

	task->uniproc = 1;

	if (!task->stopped) {
		uint8_t *line = task->cmd.line;

		if (line && line[0] == '/')
			*count += 1;
	}

	return 0;
}

static void check_all_tasks(void)
{
	int i;

	for (i = 0; i < 100; i++) {
		int count = -1;

		task_foreach(f_check, &count);

		if (count == 0)
			break;

		task_sleep(100);
	}
}

static uint32_t ap_halt_count = 0;

static int ap_halt_task(void *arg)
{
	task_set_cmdline(task_current(), NULL, "[cpu-halt]");

	while (arg == NULL) {
		int r = cpu_ints(0);
		void *tss = gdt_get_tss();

		if (tss != task_uniproc_tss) {
			cpu_add32(&ap_halt_count, 1);
			cpu_halt(0);
		}

		cpu_ints(r);
	}

	return 0;
}

static void operate(int power_off)
{
	int i, sig;

	for (i = 0; i < (kernel->smp_ap_count * 4); i++) {
		while (!task_create(ap_halt_task, NULL, task_detached)) {
			sig = SIGTERM | 128;
			task_foreach(f_send, &sig);
			sig = SIGKILL | 128;
			task_foreach(f_send, &sig);
		}
	}

	for (i = 0; i < 1000; i++) {
		if (ap_halt_count == (uint32_t)kernel->smp_ap_count)
			break;
		task_sleep(20);
	}

	task_sleep(250);
	task_prepare_rebooting();
	task_sleep(250);

	kernel->print("\033[?25l\n");

	kernel->print("\033[97mSending the TERM signal!\033[0m\n");
	sig = SIGTERM;
	task_foreach(f_send, &sig);
	check_all_tasks();

	kernel->print("\033[97mSending the KILL signal!\033[0m\n");
	sig = SIGKILL;
	task_foreach(f_send, &sig);
	check_all_tasks();

	if (kernel->smp_ap_count != 0) {
		if (ap_halt_count == (uint32_t)kernel->smp_ap_count) {
			kernel->print("\033[97m%s!\033[0m\n",
				"All application processors halted");
		}
	}

	kernel->print("\033[97m%s!\033[0m\n",
		(power_off) ? "Powering off" : "Rebooting");

	task_sleep(3000);

	runlevel_send_request((power_off) ? 0 : 6);
	cpu_halt(0);
}

int reboot_internal(int request, long long arg)
{
	if (request == __DANCY_REQUEST_RESTART && arg == 0)
		return operate(0), 0;

	if (request == __DANCY_REQUEST_POWER_OFF && arg == 0)
		return operate(1), 0;

	return DE_ARGUMENT;
}
