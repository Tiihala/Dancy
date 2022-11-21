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
	int fd_count;
	const int *fd_map;
	void *arg_state;
};

static int new_task(void *arg)
{
	struct task_arg *ta = arg;
	addr_t user_ip, user_sp;
	int r;

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

	if ((r = file_map_descriptors(ta->fd_count, ta->fd_map)) != 0) {
		ta->retval = r;
		spin_unlock(&ta->lock);
		return 0;
	}

	arg_delete(ta->arg_state);

	ta->retval = 0;
	spin_unlock(&ta->lock);

	task_jump(user_ip, user_sp);

	return 0;
}

int spawn_task(uint64_t *id, struct vfs_node *node,
	int fd_count, const int *fd_map, void *arg_state)
{
	struct task_arg ta;
	int *fd_map_copy = NULL;

	memset(&ta, 0, sizeof(ta));

	if (fd_count > 0) {
		size_t size = (size_t)fd_count * sizeof(int);

		if ((fd_map_copy = malloc(size)) == NULL)
			return *id = 0, DE_MEMORY;

		memcpy(fd_map_copy, fd_map, size);
	}

	ta.lock = 1;
	ta.retval = DE_UNEXPECTED;

	ta.node = node;
	ta.fd_count = fd_count;
	ta.fd_map = fd_map_copy;
	ta.arg_state = arg_state;

	if ((*id = task_create(new_task, &ta, 0)) == 0) {
		free(fd_map_copy);
		return DE_MEMORY;
	}

	while (!spin_trylock(&ta.lock))
		task_yield();

	free(fd_map_copy);

	if (ta.retval)
		task_wait(*id, NULL);

	return ta.retval;
}