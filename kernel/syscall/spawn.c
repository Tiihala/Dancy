/*
 * Copyright (c) 2022 Antti Tiihala
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
 * syscall/spawn.c
 *      Create a new child process
 */

#include <dancy.h>

struct task_arg {
	int lock;
	int retval;

	struct vfs_node *node;
	void *arg_state;

	struct __dancy_spawn_file_actions *actions;
	struct __dancy_spawn_attributes *attrp;
	struct __dancy_spawn_attributes __attr;
};

static int new_task(void *arg)
{
	struct task_arg *ta = arg;
	addr_t user_ip, user_sp;
	int i, r;

	if ((r = pg_create()) != 0) {
		ta->retval = r;
		spin_unlock(&ta->lock);
		return 0;
	}

	if ((r = coff_load_executable(ta->node, &user_ip)) != 0) {
		ta->retval = r;
		spin_unlock(&ta->lock);
		return 0;
	}

	if ((r = arg_copy(ta->arg_state, &user_sp)) != 0) {
		ta->retval = r;
		spin_unlock(&ta->lock);
		return 0;
	}

	arg_delete(ta->arg_state);

	{
		const uint32_t fd_cloexec = 0x80000000;
		struct task *task = task_current();

		for (i = 0; i < (int)task->fd.state; i++) {
			if ((task->fd.table[i] & fd_cloexec) != 0)
				file_close(i);
		}
	}

	ta->retval = 0;
	spin_unlock(&ta->lock);

	task_jump(user_ip, user_sp);

	return 0;
}

static int handle_options(struct task_arg *ta,
	const struct __dancy_spawn_options *options)
{
	if (((addr_t)options % (addr_t)sizeof(void *)) != 0)
		return DE_ARGUMENT;

	if (pg_check_user_read(options, sizeof(*options)))
		return DE_ARGUMENT;

	if (options->actions) {
		size_t size = sizeof(*options->actions);

		if (pg_check_user_read(options->actions, size))
			return DE_ARGUMENT;

		if ((ta->actions = malloc(size)) == NULL)
			return DE_MEMORY;

		memcpy(ta->actions, options->actions, size);
	}

	if (options->attrp) {
		size_t size = sizeof(*options->attrp);

		if (pg_check_user_read(options->attrp, size))
			return DE_ARGUMENT;

		ta->attrp = &ta->__attr;
		memcpy(ta->attrp, options->attrp, size);
	}

	return 0;
}

int spawn_task(uint64_t *id, struct vfs_node *node,
	void *arg_state, const struct __dancy_spawn_options *options)
{
	struct task_arg ta;
	int r;

	memset(&ta, 0, sizeof(ta));

	ta.lock = 1;
	ta.retval = DE_UNEXPECTED;

	ta.node = node;
	ta.arg_state = arg_state;

	if (options && (r = handle_options(&ta, options)) != 0)
		return free(ta.actions), *id = 0, r;

	if ((*id = task_create(new_task, &ta, 0)) == 0)
		return free(ta.actions), DE_MEMORY;

	while (!spin_trylock(&ta.lock))
		task_yield();

	if (ta.actions)
		free(ta.actions);

	if (ta.retval)
		task_wait(*id, NULL);

	return ta.retval;
}
