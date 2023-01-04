/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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
	clockid_t id = va_arg(va, clockid_t);
	struct timespec *tp = va_arg(va, struct timespec *);
	long long r = -EINVAL;

	struct timespec t;
	memset(&t, 0, sizeof(t));

	if (id == CLOCK_REALTIME) {
		r = (long long)epoch_read();
		t.tv_sec = (time_t)r;

	} else if (id == CLOCK_MONOTONIC) {
		uint64_t ms64 = timer_read();
		uint32_t ms32 = (uint32_t)ms64;

		r = (long long)(ms64 / 1000);
		t.tv_sec = (time_t)r;
		t.tv_nsec = (long)(ms32 % 1000) * 1000000L;
	}

	if (r >= 0 && tp != NULL) {
		if (pg_check_user_write(tp, sizeof(*tp)))
			return -EFAULT;
		memcpy(tp, &t, sizeof(t));
	}

	return r;
}

static long long dancy_syscall_execve(va_list va)
{
	const char *path = va_arg(va, const char *);
	const void *argv = va_arg(va, const void *);
	const void *envp = va_arg(va, const void *);

	addr_t user_ip, user_sp;
	struct vfs_node *node;
	void *arg_state;
	int i, r;

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

	{
		const uint32_t fd_cloexec = 0x80000000;
		struct task *task = task_current();

		for (i = 0; i < (int)task->fd.state; i++) {
			if ((task->fd.table[i] & fd_cloexec) != 0)
				file_close(i);
		}
	}

	task_jump(user_ip, user_sp);

	return 0;
}

static long long dancy_syscall_spawn(va_list va)
{
	const char *path = va_arg(va, const char *);
	const void *argv = va_arg(va, const void *);
	const void *envp = va_arg(va, const void *);
	const void *options = va_arg(va, const void *);

	struct vfs_node *node;
	void *arg_state;
	uint64_t id;
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

	/*
	 * The new task calls arg_delete(arg_state) if there are no errors.
	 */
	if ((r = spawn_task(&id, node, arg_state, options)) != 0) {
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
		if (((addr_t)status % (addr_t)sizeof(int)) != 0)
			return -EFAULT;

		if (pg_check_user_write(status, sizeof(int)))
			return -EFAULT;
	}

	if (task_wait_descendant(&id, status))
		return -ECHILD;

	return (long long)id;
}

static long long dancy_syscall_waitpid(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	int *status = va_arg(va, int *);
	int options = va_arg(va, int);

	if (status) {
		if (((addr_t)status % (addr_t)sizeof(int)) != 0)
			return -EFAULT;

		if (pg_check_user_write(status, sizeof(int)))
			return -EFAULT;
	}

	if (pid < -1)
		return -ECHILD;

	if (pid == -1 || pid == 0) {
		uint64_t id;

		if ((options & WNOHANG) != 0) {
			int r = task_trywait_descendant(&id, status);
			if (r == DE_RETRY)
				return 0;
			return -ECHILD;
		} else {
			if (task_wait_descendant(&id, status))
				return -ECHILD;
		}

		return (long long)id;
	}

	if ((options & WNOHANG) != 0) {
		int r = task_trywait((uint64_t)pid, status);
		if (r == DE_RETRY)
			return 0;
		return -ECHILD;
	} else {
		if (task_wait((uint64_t)pid, status))
			return -ECHILD;
	}

	return (long long)pid;
}

static long long dancy_syscall_open(va_list va)
{
	const char *path = va_arg(va, const char *);
	int flags = va_arg(va, int);
	mode_t mode = va_arg(va, mode_t);
	int count, fd, r;

	if (pg_check_user_string(path, &count))
		return -EFAULT;

	if ((r = file_open(&fd, path, flags, mode)) != 0) {
		if (r == DE_ARGUMENT)
			return -EINVAL;
		if (r == DE_NAME)
			return -ENOENT;
		if (r == DE_PATH)
			return -ENOENT;
		if (r == DE_MEMORY)
			return -ENOMEM;
		if (r == DE_OVERFLOW)
			return -EMFILE;
		if (r == DE_FILE)
			return -ENOTDIR;
		if (r == DE_BUSY)
			return -EBUSY;
		if (r == DE_FULL)
			return -ENOSPC;
		return -EIO;
	}

	if (fd < 0 || fd >= TASK_FD_STATIC_COUNT)
		kernel->panic("__dancy_syscall_open: unexpected behavior");

	return (long long)fd;
}

static long long dancy_syscall_close(va_list va)
{
	int fd = va_arg(va, int);
	int r;

	if ((r = file_close(fd)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		return -EIO;
	}

	return 0;
}

static long long dancy_syscall_read(va_list va)
{
	int fd = va_arg(va, int);
	void *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int r;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if (size > 0x7FFFF000)
		size = 0x7FFFF000;

	if ((r = file_read(fd, &size, buffer)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		if (r == DE_RETRY)
			return -EAGAIN;
		if (r == DE_DIRECTORY)
			return -EISDIR;
		return -EIO;
	}

	return (long long)size;
}

static long long dancy_syscall_write(va_list va)
{
	int fd = va_arg(va, int);
	const void *buffer = va_arg(va, const void *);
	size_t size = va_arg(va, size_t);
	int r;

	if (pg_check_user_read(buffer, size))
		return -EFAULT;

	if (size > 0x7FFFF000)
		size = 0x7FFFF000;

	if ((r = file_write(fd, &size, buffer)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		if (r == DE_RETRY)
			return -EAGAIN;
		if (r == DE_DIRECTORY)
			return -EISDIR;
		if (r == DE_FULL)
			return -ENOSPC;
		if (r == DE_READ_ONLY)
			return -EINVAL;
		return -EIO;
	}

	return (long long)size;
}

static long long dancy_syscall_pipe(va_list va)
{
	int *out_fd = va_arg(va, int *);
	int flags = va_arg(va, int);
	int fd[2], r;

	if (((addr_t)out_fd % (addr_t)sizeof(int)) != 0)
		return -EFAULT;

	if (pg_check_user_write(out_fd, sizeof(int) + sizeof(int)))
		return -EFAULT;

	if ((r = file_pipe(fd, flags)) != 0) {
		if (r == DE_OVERFLOW)
			return -EMFILE;
		return -ENFILE;
	}

	out_fd[0] = fd[0];
	out_fd[1] = fd[1];

	return 0;
}

static long long dancy_syscall_dup(va_list va)
{
	int fd = va_arg(va, int);
	int min_fd = va_arg(va, int);
	int max_fd = va_arg(va, int);
	int flags = va_arg(va, int);
	int new_fd, r;

	if ((r = file_dup(fd, &new_fd, min_fd, max_fd, flags)) != 0) {
		if (r == DE_OVERFLOW)
			return -EMFILE;
		return -EBADF;
	}

	return new_fd;
}

static long long dancy_syscall_lseek(va_list va)
{
	int fd = va_arg(va, int);
	off_t offset = va_arg(va, off_t);
	int whence = va_arg(va, int);
	uint64_t new_offset;
	int r;

	if ((r = file_lseek(fd, offset, &new_offset, whence)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		if (r == DE_ACCESS)
			return -ESPIPE;
		return -EINVAL;
	}

	if (new_offset > (uint64_t)(LLONG_MAX))
		new_offset = (uint64_t)(LLONG_MAX);

	return (long long)new_offset;
}

static long long dancy_syscall_fcntl(va_list va)
{
	int fd = va_arg(va, int);
	int cmd = va_arg(va, int);
	int arg = va_arg(va, int);
	int r, retval;

	if ((r = file_fcntl(fd, cmd, arg, &retval)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		return -EINVAL;
	}

	return (long long)((retval >= 0) ? retval : 0);
}

static long long dancy_syscall_getcwd(va_list va)
{
	void *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int r;

	if (size == 0)
		return -EINVAL;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if ((r = file_getcwd(buffer, size)) != 0) {
		if (r == DE_OVERFLOW)
			return -ERANGE;
		return -EACCES;
	}

	return 0;
}

static long long dancy_syscall_getdents(va_list va)
{
	int fd = va_arg(va, int);
	void *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int count = va_arg(va, int);
	int flags = va_arg(va, int);
	int r;

	if (((addr_t)buffer % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if ((r = file_getdents(fd, buffer, size, &count, flags)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		if (r == DE_BUFFER)
			return -EOVERFLOW;
		return -ENOENT;
	}

	return count;
}

static long long dancy_syscall_chdir(va_list va)
{
	const char *path = va_arg(va, const char *);
	int count, r;

	if (pg_check_user_string(path, &count))
		return -EFAULT;

	if ((r = file_chdir(path)) != 0) {
		if (r == DE_FILE)
			return -ENOTDIR;
		if (r == DE_TYPE)
			return -ENOTDIR;
		return -ENOENT;
	}

	return 0;
}

static long long dancy_syscall_rmdir(va_list va)
{
	const char *path = va_arg(va, const char *);
	int count, r;

	if (pg_check_user_string(path, &count))
		return -EFAULT;

	if ((r = vfs_rmdir(path)) != 0) {
		if (r == DE_FILE)
			return -ENOTDIR;
		if (r == DE_TYPE)
			return -ENOTDIR;
		if (r == DE_BUSY)
			return -EBUSY;
		if (r == DE_READ_ONLY)
			return -EACCES;
		return -ENOENT;
	}

	return 0;
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
	{ dancy_syscall_waitpid },
	{ dancy_syscall_open },
	{ dancy_syscall_close },
	{ dancy_syscall_read },
	{ dancy_syscall_write },
	{ dancy_syscall_pipe },
	{ dancy_syscall_dup },
	{ dancy_syscall_lseek },
	{ dancy_syscall_fcntl },
	{ dancy_syscall_getcwd },
	{ dancy_syscall_getdents },
	{ dancy_syscall_chdir },
	{ dancy_syscall_rmdir },
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
