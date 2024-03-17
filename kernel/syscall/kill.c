/*
 * Copyright (c) 2023 Antti Tiihala
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
 * syscall/kill.c
 *      Send signal
 */

#include <dancy.h>

struct f_arg {
	struct task *current;
	__dancy_pid_t pid;
	int sig;
	int r;
};

static int f(struct task *task, void *arg)
{
	struct f_arg *a = arg;

	if (a->r != DE_SEARCH) {
		int sig = a->sig;

		if (a->current == task || task->id == 1)
			return 0;

		if (a->pid < -1 && task->id_group != (uint64_t)(-(a->pid)))
			return 0;

		if (a->pid == 0 && task->id_group != a->current->id_group)
			return 0;

		if (task->cmd.line == NULL || task->cmd.line[0] != '/')
			return 0;

		if (sig > 0 && sig < 32) {
			void *address = &task->asm_data3;
			uint32_t value = (uint32_t)(sig - 1);

			cpu_bts32(address, value);
		}

		return 0;
	}

	if ((uint64_t)a->pid == task->id) {
		int sig = a->sig;

		if (task->cmd.line == NULL || task->cmd.line[0] != '/')
			return (a->r = DE_ACCESS), 1;

		if (sig > 0 && sig < 32) {
			void *address = &task->asm_data3;
			uint32_t value = (uint32_t)(sig - 1);

			cpu_bts32(address, value);
		}

		return (a->r = 0), 1;
	}

	return 0;
}

static int allowed_signal(int sig)
{
	switch (sig) {
		case 0:
			return 1;
		case SIGABRT:
			return 1;
		case SIGINT:
			return 1;
		case SIGKILL:
			return 1;
		case SIGQUIT:
			return 1;
		case SIGTERM:
			return 1;
		case SIGUSR1:
			return 1;
		case SIGUSR2:
			return 1;
		default:
			break;
	};

	return 0;
}

int kill_internal(__dancy_pid_t pid, int sig, int flags)
{
	struct f_arg a;

	a.current = task_current();
	a.pid = pid;
	a.sig = sig;
	a.r = DE_SEARCH;

	if (pid <= 0)
		a.r = 0;

	if (allowed_signal(sig) && flags == 0) {
		task_foreach(f, &a);
		return a.r;
	}

	return DE_ARGUMENT;
}
