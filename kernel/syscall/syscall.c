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
 * syscall/syscall.c
 *      System calls
 */

#include <dancy.h>
#include <errno.h>

static long long dancy_syscall_exit(va_list va)
{
	int retval = va_arg(va, int);

	task_exit((retval & 0xFF) << 8);

	return kernel->panic("__dancy_syscall_exit: unexpected behavior"), 0;
}

static long long dancy_syscall_time(va_list va)
{
	return (void)va, (long long)epoch_read();
}

static long long dancy_syscall_execve(va_list va)
{
	const char *path = va_arg(va, const char *);
	const void *argv = va_arg(va, const void *);
	const void *envp = va_arg(va, const void *);

	addr_t user_ip, user_sp;
	struct vfs_node *node;
	void *arg_state;
	int r;

	if (pg_check_user_string(path, &r))
		return -EFAULT;

	if ((r = vfs_open(path, &node, vfs_type_regular, 0)) != 0) {
		if (r == DE_NAME)
			return -ENOENT;
		if (r == DE_TYPE)
			return -ENOEXEC;
		return -EINVAL;
	}

	if ((r = arg_create(&arg_state, argv, envp)) != 0) {
		node->n_release(&node);

		if (r == DE_MEMORY)
			return -ENOMEM;
		return -EFAULT;
	}

	if (pg_alt_create()) {
		arg_delete(arg_state);
		node->n_release(&node);

		return -ENOMEM;
	}

	if ((r = coff_load_executable(node, &user_ip)) != 0) {
		pg_alt_delete();
		arg_delete(arg_state);
		node->n_release(&node);

		if (r == DE_MEMORY)
			return -ENOMEM;
		return -ENOEXEC;
	}

	if (arg_copy(arg_state, &user_sp)) {
		pg_alt_delete();
		arg_delete(arg_state);
		node->n_release(&node);

		return -ENOMEM;
	}

	pg_alt_accept();
	arg_delete(arg_state);
	node->n_release(&node);

	file_map_descriptors(0, NULL);
	task_jump(user_ip, user_sp);

	return 0;
}

static long long dancy_syscall_spawn(va_list va)
{
	const char *path = va_arg(va, const char *);
	int fd_count = va_arg(va, int);
	const int *fd_map = va_arg(va, const int *);
	const void *argv = va_arg(va, const void *);
	const void *envp = va_arg(va, const void *);

	struct vfs_node *node;
	void *arg_state;
	uint64_t id;
	int r;

	if (pg_check_user_string(path, &r))
		return -EFAULT;

	if (fd_count < 0 || fd_count > TASK_FD_STATIC_COUNT)
		return -EINVAL;

	if (fd_count > 0) {
		size_t size = (size_t)fd_count * sizeof(int);

		if (((addr_t)fd_map % (addr_t)sizeof(int)) != 0)
			return -EFAULT;

		if (pg_check_user_read(fd_map, size))
			return -EFAULT;
	}

	if ((r = vfs_open(path, &node, vfs_type_regular, 0)) != 0) {
		if (r == DE_NAME)
			return -ENOENT;
		if (r == DE_TYPE)
			return -ENOEXEC;
		return -EINVAL;
	}

	if ((r = arg_create(&arg_state, argv, envp)) != 0) {
		node->n_release(&node);

		if (r == DE_MEMORY)
			return -ENOMEM;
		return -EFAULT;
	}

	/*
	 * The new task calls arg_delete(arg_state) if there are no errors.
	 */
	if ((r = spawn_task(&id, node, fd_count, fd_map, arg_state)) != 0) {
		arg_delete(arg_state);
		node->n_release(&node);

		if (r == DE_MEMORY)
			return -ENOMEM;
		if (r == DE_ARGUMENT)
			return -EINVAL;
		return -ENOEXEC;
	}

	node->n_release(&node);

	return (long long)id;
}

static long long dancy_syscall_wait(va_list va)
{
	int *status = va_arg(va, int *);
	uint64_t id;

	if (status) {
		if (((addr_t)status % (addr_t)sizeof(int *)) != 0)
			return -EFAULT;

		if (pg_check_user_write(status, sizeof(int *)))
			return -EFAULT;
	}

	if (task_wait_descendant(&id, status))
		return -ECHILD;

	return (long long)id;
}

static long long dancy_syscall_reserved(va_list va)
{
	return (void)va, -EINVAL;
}

static struct { long long (*handler)(va_list va); } handler_array[] = {
	{ dancy_syscall_exit },
	{ dancy_syscall_time },
	{ dancy_syscall_execve },
	{ dancy_syscall_spawn },
	{ dancy_syscall_wait },
	{ dancy_syscall_reserved }
};

int syscall_init(void)
{
	static int run_once;
	extern void syscall_init_asm(void);
	uint32_t t0, t1, t2, t3;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	syscall_init_asm();

	t0 = (uint32_t)(sizeof( handler_array));
	t1 = (uint32_t)(sizeof(*handler_array));
	t2 = (uint32_t)(__dancy_syscall_arg0__);
	t3 = (uint32_t)(__dancy_syscall_argn__);

	if (cpu_read32(&t0) / t1 != t3 - t2)
		return DE_UNEXPECTED;

	return 0;
}

long long syscall_handler(int arg0, ...)
{
	long long r = -EINVAL;

	va_list va;
	va_start(va, arg0);

	if (arg0 > __dancy_syscall_arg0__ && arg0 < __dancy_syscall_argn__) {
		int i = arg0 - __dancy_syscall_arg0__ - 1;
		r = handler_array[i].handler(va);
	}

	va_end(va);

	return r;
}
