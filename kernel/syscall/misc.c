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
 * syscall/misc.c
 *      Miscellaneous internal functions
 */

#include <dancy.h>

struct id_internal_arg {
	uint64_t id;
	uint64_t *id_group;
	uint64_t *id_session;
};

static int id_internal_func(struct task *task, void *arg)
{
	struct id_internal_arg *a = arg;

	if (task->id == a->id) {
		if (a->id_group != NULL)
			*a->id_group = task->id_group;
		if (a->id_session != NULL)
			*a->id_session = task->id_session;
		return 1;
	}

	return 0;
}

int getpgid_internal(uint64_t id, uint64_t *id_group)
{
	struct id_internal_arg arg;

	if ((arg.id = id) == 0) {
		task_identify(NULL, NULL, id_group, NULL);
		return 0;
	}

	*(arg.id_group = id_group) = 0;
	arg.id_session = NULL;

	task_foreach(id_internal_func, &arg);

	if (*id_group == 0)
		return DE_SEARCH;

	return 0;
}

int getsid_internal(uint64_t id, uint64_t *id_session)
{
	struct id_internal_arg arg;

	if ((arg.id = id) == 0) {
		task_identify(NULL, NULL, NULL, id_session);
		return 0;
	}

	arg.id_group = NULL;
	*(arg.id_session = id_session) = 0;

	task_foreach(id_internal_func, &arg);

	if (*id_session == 0)
		return DE_SEARCH;

	return 0;
}
