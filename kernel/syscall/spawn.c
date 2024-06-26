/*
 * Copyright (c) 2022, 2023, 2024 Antti Tiihala
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
	struct __dancy_spawn_attributes _attr;

	int setpgroup_retval;
};

static void func_setsid(struct task *current, void *arg)
{
	uint64_t id = current->id;

	(void)arg;
	current->id_group = id;
	current->id_session = id;
}

static void func_setsigmask(struct task *current, void *arg)
{
	uint32_t mask = *((uint32_t *)arg);

	mask &= ~(((uint32_t)1) << (SIGKILL - 1));
	mask &= ~(((uint32_t)1) << (SIGSTOP - 1));

	current->sig.mask = mask;
}

static int func_setpgroup(struct task *task, void *arg)
{
	struct task_arg *ta = arg;
	struct task *current = task_current();
	uint64_t pgroup = (uint64_t)ta->attrp->_pgroup;

	if (pgroup == 0)
		pgroup = current->id;

	if (task != current) {
		if (task->id_group == pgroup) {
			if (task->id_session != current->id_session)
				return (ta->setpgroup_retval = DE_ACCESS);
		}
		return 0;
	}

	if ((pgroup >> 63) != 0)
		return (ta->setpgroup_retval = DE_ARGUMENT);

	if (current->id_session == current->id && current->id_group != pgroup)
		return (ta->setpgroup_retval = DE_ACCESS);

	current->id_group = pgroup;

	return 0;
}

static int new_task(void *arg)
{
	struct task_arg *ta = arg;
	addr_t user_ip, user_sp;
	int i, r = 0;

	if (ta->attrp) {
		int flags = ta->attrp->_flags;

		if ((flags & __DANCY_SPAWN_USEVFORK) != 0)
			flags ^= __DANCY_SPAWN_USEVFORK;

		if ((flags & __DANCY_SPAWN_SETSID) != 0) {
			flags ^= __DANCY_SPAWN_SETSID;
			task_access(func_setsid, NULL);
		}

		if ((flags & __DANCY_SPAWN_SETPGROUP) != 0) {
			flags ^= __DANCY_SPAWN_SETPGROUP;
			task_foreach(func_setpgroup, arg);

			if (ta->setpgroup_retval != 0) {
				ta->retval = ta->setpgroup_retval;
				spin_unlock(&ta->lock);
				return 0;
			}
		}

		if ((flags & __DANCY_SPAWN_SETSIGDEF) != 0)
			flags ^= __DANCY_SPAWN_SETSIGDEF;

		if ((flags & __DANCY_SPAWN_SETSIGMASK) != 0) {
			flags ^= __DANCY_SPAWN_SETSIGMASK;
			task_access(func_setsigmask, &ta->attrp->_sigmask);
		}

		if (flags != 0)
			r = DE_ARGUMENT;
	}

	if (ta->actions) {
		int count = (int)ta->actions->_count;

		for (i = 0; r == 0 && i < count; i++) {
			int type = ta->actions->_actions[i]._type;
			int flags, mode, fd[3];
			const void *p;

			if (type == __DANCY_SPAWN_ADD_CLOSE) {
				fd[0] = ta->actions->_actions[i]._args[0];
				(void)file_close(fd[0]);

			} else if (type == __DANCY_SPAWN_ADD_DUP2) {
				fd[0] = ta->actions->_actions[i]._args[0];
				fd[1] = ta->actions->_actions[i]._args[1];

				if (fd[0] == fd[1]) {
					int cmd = F_GETFD;
					r = file_fcntl(fd[0], cmd, 0, &fd[2]);
					continue;
				}

				r = file_dup(fd[0], &fd[2], fd[1], fd[1], 0);

				if (fd[1] != fd[2])
					r = DE_ARGUMENT;

			} else if (type == __DANCY_SPAWN_ADD_OPEN) {
				fd[1] = ta->actions->_actions[i]._args[0];
				flags = ta->actions->_actions[i]._args[1];
				mode = ta->actions->_actions[i]._args[2];
				p = ta->actions->_actions[i]._path;

				r = file_open(&fd[0], p, flags, (mode_t)mode);

				if (r || fd[0] == fd[1])
					continue;

				r = file_dup(fd[0], &fd[2], fd[1], fd[1], 0);

				if (fd[1] != fd[2])
					r = DE_ARGUMENT;

				(void)file_close(fd[0]);

			} else if (type == __DANCY_SPAWN_ADD_TCSET) {
				int request = __DANCY_IOCTL_TIOCSPGRP;
				__dancy_pid_t *g;
				long long a;

				if ((g = malloc(sizeof(*g))) == NULL) {
					r = DE_MEMORY;
					continue;
				}

				fd[0] = ta->actions->_actions[i]._args[0];
				*g = (__dancy_pid_t)task_current()->id_group;
				a = (long long)((addr_t)g);

				r = file_ioctl(fd[0], request, a);
				free(g);

			} else {
				r = DE_ARGUMENT;
				break;
			}
		}
	}

	if (r != 0) {
		ta->retval = r;
		spin_unlock(&ta->lock);
		return 0;
	}

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

	if ((r = arg_set_cmdline(ta->node, user_sp)) != 0) {
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
		const struct __dancy_spawn_file_actions *src;
		struct __dancy_spawn_file_actions *dst;
		unsigned int i;

		if (pg_check_user_read(options->actions, size))
			return DE_ARGUMENT;

		if ((ta->actions = malloc(size)) == NULL)
			return DE_MEMORY;

		dst = ta->actions;
		src = options->actions;

		memset(dst, 0, size);
		dst->_state = src->_state;
		dst->_count = src->_count;

		{
			int t1 = (int)sizeof(dst->_actions);
			int t2 = (int)sizeof(*dst->_actions);

			if (dst->_count >= (unsigned int)(t1 / t2)) {
				dst->_state = 0, dst->_count = 0;
				return DE_ARGUMENT;
			}
		}

		for (i = 0; i < dst->_count; i++) {
			const char *path = src->_actions[i]._path;
			const char *dup_path = NULL;
			int value;

			value = src->_actions[i]._type;
			dst->_actions[i]._type = value;

			value = src->_actions[i]._args[0];
			dst->_actions[i]._args[0] = value;

			value = src->_actions[i]._args[1];
			dst->_actions[i]._args[1] = value;

			value = src->_actions[i]._args[2];
			dst->_actions[i]._args[2] = value;

			if (path) {
				if (pg_check_user_string(path, &value))
					return DE_ARGUMENT;

				if ((dup_path = strdup(path)) == NULL)
					return DE_MEMORY;
			}

			dst->_actions[i]._path = dup_path;
		}
	}

	if (options->attrp) {
		size_t size = sizeof(*options->attrp);

		if (pg_check_user_read(options->attrp, size))
			return DE_ARGUMENT;

		ta->attrp = &ta->_attr;
		memcpy(ta->attrp, options->attrp, size);
	}

	return 0;
}

static void release_actions(struct task_arg *ta)
{
	unsigned int i;

	if (!ta->actions)
		return;

	for (i = 0; i < ta->actions->_count; i++) {
		if (ta->actions->_actions[i]._path)
			free((void *)(ta->actions->_actions[i]._path));
	}

	memset(ta->actions, 0, sizeof(*ta->actions));
	free(ta->actions);

	ta->actions = NULL;
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
		return release_actions(&ta), *id = 0, r;

	if ((*id = task_create(new_task, &ta, 0)) == 0)
		return release_actions(&ta), DE_MEMORY;

	while (!spin_trylock(&ta.lock))
		task_yield();

	release_actions(&ta);

	if (ta.retval)
		task_wait_descendant(id, 0, NULL);

	return ta.retval;
}
