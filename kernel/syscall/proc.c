/*
 * Copyright (c) 2024 Antti Tiihala
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
 * syscall/proc.c
 *      Internals of "proclist" and "procinfo" syscalls
 */

#include <dancy.h>

struct f1_arg {
	size_t size[2];
	__dancy_pid_t *out;
	int r;
};

static int f1(struct task *task, void *arg)
{
	struct f1_arg *a = arg;

	if ((a->size[0] + sizeof(__dancy_pid_t)) > a->size[1])
		return (a->r = DE_MEMORY), 1;

	if ((*a->out++ = (__dancy_pid_t)task->id) <= 0)
		kernel->panic("proclist: unexpected behavior");

	a->size[0] += sizeof(__dancy_pid_t);

	return 0;
}

static int f1_qsort(const void *p1, const void *p2)
{
	__dancy_pid_t id1 = *((const __dancy_pid_t *)p1);
	__dancy_pid_t id2 = *((const __dancy_pid_t *)p2);

	if (id1 == id2)
		kernel->panic("proclist: unexpected behavior");

	return (id1 < id2) ? -1 : 1;
}

int proclist_internal(size_t *size, __dancy_pid_t *out)
{
	struct f1_arg _a;
	struct f1_arg *a = &_a;
	size_t count;

	a->size[0] = 0;
	a->size[1] = *size;
	a->out = out;
	a->r = 0;

	if (a->size[1] > 0x80000000)
		a->size[1] = 0x80000000;

	task_foreach(f1, a);
	count = a->size[0] / sizeof(__dancy_pid_t);

	if (count == 0 && a->r == 0)
		kernel->panic("proclist: unexpected behavior");

	if (a->r != 0) {
		if (*size != 0)
			memset(out, 0, *size);

		return (*size = 0), a->r;
	}

	qsort(out, count, sizeof(__dancy_pid_t), f1_qsort);
	*size = a->size[0];

	return 0;
}

struct f2_arg {
	__dancy_pid_t id;
	int request;
	size_t size[2];
	void *out;
	int r;
};

static int f2(struct task *task, void *arg)
{
	struct f2_arg *a = arg;

	if (task->id != (uint64_t)a->id)
		return 0;

	if (a->request == __DANCY_PROCINFO_OWNER_ID) {
		__dancy_pid_t id = (__dancy_pid_t)task->id_owner;

		if (a->size[1] < sizeof(id))
			return (a->r = DE_MEMORY), 1;

		memcpy(a->out, &id, sizeof(id));
		a->size[0] +=  sizeof(id);

		return (a->r = 0), 1;
	}

	if (a->request == __DANCY_PROCINFO_GROUP_ID) {
		__dancy_pid_t id = (__dancy_pid_t)task->id_group;

		if (a->size[1] < sizeof(id))
			return (a->r = DE_MEMORY), 1;

		memcpy(a->out, &id, sizeof(id));
		a->size[0] +=  sizeof(id);

		return (a->r = 0), 1;
	}

	if (a->request == __DANCY_PROCINFO_SESSION_ID) {
		__dancy_pid_t id = (__dancy_pid_t)task->id_session;

		if (a->size[1] < sizeof(id))
			return (a->r = DE_MEMORY), 1;

		memcpy(a->out, &id, sizeof(id));
		a->size[0] +=  sizeof(id);

		return (a->r = 0), 1;
	}

	if (a->request == __DANCY_PROCINFO_CMDLINE) {
		uint8_t *line = task->cmd.line;
		uint8_t *out = a->out;
		size_t i = 0;

		if (line == NULL || line[0] == 0) {
			if (a->size[1] < 9)
				return (a->r = DE_MEMORY), 1;

			memcpy(out, "[unknown]", 9);
			a->size[0] = 9;

			return (a->r = 0), 1;
		}

		for (;;) {
			if (line[i + 0] == 0 && line[i + 1] == 0)
				break;

			if (a->size[0] >= a->size[1]) {
				if (a->size[0] < 16)
					return (a->r = DE_MEMORY), 1;

				memcpy(&out[-3], "...", 3);
				return (a->r = 0), 1;
			}

			*out++ = line[i++];
			a->size[0] += 1;
		}

		return (a->r = 0), 1;
	}

	return (a->r = DE_ARGUMENT), 1;
}

int procinfo_internal(__dancy_pid_t id, int request, size_t *size, void *out)
{
	struct f2_arg _a;
	struct f2_arg *a = &_a;

	a->id = id;
	a->request = request;
	a->size[0] = 0;
	a->size[1] = *size;
	a->out = out;
	a->r = DE_SEARCH;

	task_foreach(f2, a);

	if (a->r != 0) {
		if (*size != 0 && a->r != DE_ARGUMENT)
			memset(out, 0, *size);

		return (*size = 0), a->r;
	}

	*size = a->size[0];

	return 0;
}
