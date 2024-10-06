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
		uint64_t ms64 = (uint64_t)epoch_read_ms();

		r = (long long)(ms64 / 1000);
		t.tv_sec = (time_t)r;
		t.tv_nsec = (long)(ms64 % 1000) * 1000000L;

	} else if (id == CLOCK_MONOTONIC) {
		uint64_t ms64 = timer_read();

		r = (long long)(ms64 / 1000);
		t.tv_sec = (time_t)r;
		t.tv_nsec = (long)(ms64 % 1000) * 1000000L;
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
		if (r == DE_PATH)
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

	if (arg_set_cmdline(node, user_sp)) {
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
		if (r == DE_PATH)
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
		if (r == DE_ACCESS)
			return -EPERM;
		return -ENOEXEC;
	}

	node->n_release(&node);

	return (long long)id;
}

static long long dancy_syscall_wait(va_list va)
{
	int *status = va_arg(va, int *);
	uint64_t id = 0, id_group = 0;
	int r;

	if (status) {
		if (((addr_t)status % (addr_t)sizeof(int)) != 0)
			return -EFAULT;

		if (pg_check_user_write(status, sizeof(int)))
			return -EFAULT;
	}

	if ((r = task_wait_descendant(&id, id_group, status)) != 0) {
		if (r == DE_INTERRUPT)
			return -EINTR;
		return -ECHILD;
	}

	return (long long)id;
}

static long long dancy_syscall_waitpid(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	int *status = va_arg(va, int *);
	int options = va_arg(va, int);
	uint64_t id, id_group;
	int r;

	if (status) {
		if (((addr_t)status % (addr_t)sizeof(int)) != 0)
			return -EFAULT;

		if (pg_check_user_write(status, sizeof(int)))
			return -EFAULT;
	}

	if (pid < -1) {
		id = 0;
		id_group = (uint64_t)(-pid);

	} else if (pid == -1) {
		id = 0;
		id_group = 0;

	} else if (pid == 0) {
		id = 0;
		task_identify(NULL, NULL, &id_group, NULL);

	} else {
		id = (uint64_t)pid;
		id_group = 0;
	}

	if ((options & WNOHANG) == 0) {
		if ((r = task_wait_descendant(&id, id_group, status)) != 0) {
			if (r == DE_INTERRUPT)
				return -EINTR;
			return -ECHILD;
		}
		return (long long)id;
	}

	if ((r = task_trywait_descendant(&id, id_group, status)) != 0) {
		if (r == DE_RETRY)
			return 0;
		return -ECHILD;
	}

	return (long long)id;
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
		if (r == DE_DIRECTORY)
			return -EISDIR;
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

	if ((r = file_read(fd, &size, buffer)) != 0 && size == 0) {
		if (r == DE_INTERRUPT)
			return -EINTR;
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

	if ((r = file_write(fd, &size, buffer)) != 0 && size == 0) {
		if (r == DE_INTERRUPT)
			return -EINTR;
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
		if (r == DE_PIPE)
			return -EPIPE;
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
		if (r == DE_NOT_EMPTY)
			return -ENOTEMPTY;
		return -ENOENT;
	}

	return 0;
}

static long long dancy_syscall_unlink(va_list va)
{
	const char *path = va_arg(va, const char *);
	int count, r;

	if (pg_check_user_string(path, &count))
		return -EFAULT;

	if ((r = vfs_unlink(path)) != 0) {
		if (r == DE_DIRECTORY)
			return -EPERM;
		if (r == DE_TYPE)
			return -EPERM;
		if (r == DE_BUSY)
			return -EBUSY;
		if (r == DE_READ_ONLY)
			return -EACCES;
		return -ENOENT;
	}

	return 0;
}

static long long dancy_syscall_rename(va_list va)
{
	const char *old_path = va_arg(va, const char *);
	const char *new_path = va_arg(va, const char *);
	int count, r;

	if (pg_check_user_string(old_path, &count))
		return -EFAULT;

	if (pg_check_user_string(new_path, &count))
		return -EFAULT;

	if ((r = vfs_rename(old_path, new_path)) != 0) {
		if (r == DE_BUSY)
			return -EBUSY;
		if (r == DE_UNSUPPORTED)
			return -EXDEV;
		return -EINVAL;
	}

	return 0;
}

static long long dancy_syscall_stat(va_list va)
{
	int fd = va_arg(va, int);
	const char *path = va_arg(va, const char *);
	struct stat *buffer = va_arg(va, struct stat *);
	int flags = va_arg(va, int);

	struct vfs_node *node = NULL;
	struct vfs_stat vstat;
	int count, r = 0;

	if (flags != 0 && flags != 1)
		return -EINVAL;

	if (((addr_t)buffer % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_write(buffer, sizeof(*buffer)))
		return -EFAULT;

	if (path) {
		if (pg_check_user_string(path, &count))
			return -EFAULT;

		if ((r = vfs_open(path, &node, 0, 0)) != 0) {
			if (r == DE_NAME)
				return -ENOENT;
			return -EINVAL;
		}

	} else {
		const uint32_t table_mask = 0x0FFFFFFF;
		struct task *task = task_current();
		struct file_table_entry *fte;
		uint32_t t;

		if (fd < 0 || fd >= (int)task->fd.state)
			return -EBADF;

		if ((t = task->fd.table[fd]) == 0)
			return -EBADF;

		fte = (void *)((addr_t)(t & table_mask));

		while (!spin_trylock(&fte->lock[1]))
			task_yield();

		node = fte->node;
		vfs_increment_count(node);

		spin_unlock(&fte->lock[1]);
	}

	if (node->n_stat(node, &vstat)) {
		node->n_release(&node);
		return -EIO;
	}

	memset(buffer, 0, sizeof(*buffer));

	if (node->type == vfs_type_regular)
		buffer->st_mode = 0x1FF | __DANCY_S_IFREG;

	else if (node->type == vfs_type_buffer)
		buffer->st_mode = 0x1B6 | __DANCY_S_IFIFO;

	else if (node->type == vfs_type_directory)
		buffer->st_mode = 0x1FF | __DANCY_S_IFDIR;

	else if (node->type == vfs_type_character)
		buffer->st_mode = 0x1B6 | __DANCY_S_IFCHR;

	else if (node->type == vfs_type_block)
		buffer->st_mode = 0x1B6 | __DANCY_S_IFBLK;

	else if (node->type == vfs_type_socket)
		buffer->st_mode = 0x1B6 | __DANCY_S_IFSOCK;

	node->n_release(&node);

	buffer->st_nlink = 1;
	buffer->st_size = (off_t)vstat.size;

	{
		buffer->st_atim.tv_sec  = vstat.access_time.tv_sec;
		buffer->st_atim.tv_nsec = vstat.access_time.tv_nsec;

		buffer->st_mtim.tv_sec  = vstat.write_time.tv_sec;
		buffer->st_mtim.tv_nsec = vstat.write_time.tv_nsec;

		buffer->st_ctim.tv_sec  = vstat.creation_time.tv_sec;
		buffer->st_ctim.tv_nsec = vstat.creation_time.tv_nsec;
	}

	return 0;
}

static long long dancy_syscall_sleep(va_list va)
{
	clockid_t id = va_arg(va, clockid_t);
	int flags = va_arg(va, int);
	const struct timespec *request = va_arg(va, const struct timespec *);
	struct timespec *remain = va_arg(va, struct timespec *);
	int r;

	if (((addr_t)request % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_read(request, sizeof(*request)))
		return -EFAULT;

	if (remain != NULL) {
		if (((addr_t)remain % (addr_t)sizeof(void *)) != 0)
			return -EFAULT;

		if (pg_check_user_write(remain, sizeof(*remain)))
			return -EFAULT;
	}

	if ((r = sleep_internal(id, flags, request, remain)) != 0) {
		if (r == DE_INTERRUPT)
			return -EINTR;
		if (r == DE_UNSUPPORTED)
			return -ENOTSUP;
		return -EINVAL;
	}

	return 0;
}

static long long dancy_syscall_mmap(va_list va)
{
	void *address = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	const struct __dancy_mmap_options *options = va_arg(va, const void *);

	unsigned int prot, flags;
	addr_t vaddr;

	if (((addr_t)options % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_read(options, sizeof(*options)))
		return -EFAULT;

	if (address != options->_address)
		return -EINVAL;

	if (size == 0 || size != options->_size)
		return -EINVAL;

	if (options->_prot < 0 || options->_flags < 0)
		return -EINVAL;

	prot = (unsigned int)options->_prot;
	prot &= (~((unsigned int)__DANCY_PROT_READ));
	prot &= (~((unsigned int)__DANCY_PROT_WRITE));
	prot &= (~((unsigned int)__DANCY_PROT_EXEC));

	if (prot != 0)
		return -EINVAL;

	flags = (unsigned int)options->_flags;
	flags ^= ((unsigned int)__DANCY_MAP_ANONYMOUS);
	flags ^= ((unsigned int)__DANCY_MAP_PRIVATE);
	flags ^= ((unsigned int)__DANCY_MAP_FIXED);

	if (flags != 0)
		return -ENOTSUP;

	if ((vaddr = (addr_t)address) < 0x80000000)
		return -ENOTSUP;

	if ((addr_t)pg_map_user(vaddr, size) != vaddr)
		return -ENOMEM;

	return (long long)vaddr;
}

static long long dancy_syscall_munmap(va_list va)
{
	void *address = va_arg(va, void *);
	size_t size = va_arg(va, size_t);

	if ((addr_t)address < 0x80000000 || size == 0)
		return -EINVAL;

	if (pg_unmap_user((addr_t)address, size))
		return -EINVAL;

	return 0;
}

static long long dancy_syscall_mprotect(va_list va)
{
	void *address = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	unsigned int prot = va_arg(va, unsigned int);

	if ((addr_t)address < 0x80000000)
		return -EINVAL;

	prot &= (~((unsigned int)__DANCY_PROT_READ));
	prot &= (~((unsigned int)__DANCY_PROT_WRITE));
	prot &= (~((unsigned int)__DANCY_PROT_EXEC));

	if (prot != 0)
		return -EINVAL;

	if (size && pg_check_user_read(address, size))
		return -ENOMEM;

	return 0;
}

static long long dancy_syscall_msync(va_list va)
{
	void *address = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	unsigned int flags = va_arg(va, unsigned int);

	int ms_async = (flags & __DANCY_MS_ASYNC) ? 1 : 0;
	int ms_sync = (flags & __DANCY_MS_SYNC) ? 1 : 0;

	if ((addr_t)address < 0x80000000)
		return -EINVAL;

	flags &= (~((unsigned int)__DANCY_MS_ASYNC));
	flags &= (~((unsigned int)__DANCY_MS_SYNC));
	flags &= (~((unsigned int)__DANCY_MS_INVALIDATE));

	if (flags != 0 || (ms_async != 0 && ms_sync != 0))
		return -EINVAL;

	if (size && pg_check_user_read(address, size))
		return -ENOMEM;

	return 0;
}

static long long dancy_syscall_getpid(va_list va)
{
	return (void)va, (long long)task_current()->id;
}

static long long dancy_syscall_getppid(va_list va)
{
	return (void)va, (long long)task_current()->id_owner;
}

static long long dancy_syscall_realpath(va_list va)
{
	const char *path = va_arg(va, const char *);
	void *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int r, count;

	if (pg_check_user_string(path, &count))
		return -EFAULT;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if ((r = file_realpath(path, buffer, size)) != 0) {
		if (r == DE_ARGUMENT)
			return -EINVAL;
		if (r == DE_NAME)
			return -ENOENT;
		if (r == DE_PATH)
			return -ENOENT;
		if (r == DE_MEMORY)
			return -ENOMEM;
		if (r == DE_OVERFLOW)
			return -EOVERFLOW;
		return -EIO;
	}

	return 0;
}

static long long dancy_syscall_kill(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	int sig = va_arg(va, int);
	int flags = va_arg(va, int);
	int r;

	if (pid == 1 && task_current()->id != 1)
		return -EPERM;

	if ((r = kill_internal(pid, sig, flags)) != 0) {
		if (r == DE_ACCESS)
			return -EPERM;
		if (r == DE_SEARCH)
			return -ESRCH;
		return -EINVAL;
	}

	return 0;
}

static long long dancy_syscall_poll(va_list va)
{
	struct pollfd *fds = va_arg(va, struct pollfd *);
	nfds_t nfds = va_arg(va, nfds_t);
	int timeout = va_arg(va, int);
	size_t size;
	int r, retval;

	if ((size_t)nfds > 0x7FFF)
		return -EINVAL;

	size = (size_t)nfds * sizeof(struct pollfd);

	if (((addr_t)fds % (addr_t)sizeof(int)) != 0)
		return -EFAULT;

	if (pg_check_user_write(fds, size))
		return -EFAULT;

	if ((r = file_poll(&fds[0], (int)nfds, timeout, &retval)) != 0) {
		if (r == DE_INTERRUPT)
			return -EINTR;
		if (r == DE_MEMORY)
			return -EAGAIN;
		return -EINVAL;
	}

	return (long long)retval;
}

static long long dancy_syscall_ioctl(va_list va)
{
	int fd = va_arg(va, int);
	int request = va_arg(va, int);
	long long arg = va_arg(va, long long);
	long long retval = 0;
	int r;

	if ((r = ioctl_internal(fd, request, arg, &retval)) != 0) {
		if (r == DE_ARGUMENT)
			return -EBADF;
		if (r == DE_ACCESS)
			return -EFAULT;
		if (r == DE_TYPE)
			return -ENOTTY;
		return -EINVAL;
	}

	if (retval < 0)
		return -EINVAL;

	return retval;
}

static long long dancy_syscall_getpgid(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	uint64_t id_group;
	int r;

	if (pid < 0)
		return -EINVAL;

	if ((r = getpgid_internal((uint64_t)pid, &id_group)) != 0) {
		if (r == DE_SEARCH)
			return -ESRCH;
		return -EINVAL;
	}

	return (long long)id_group;
}

static long long dancy_syscall_getsid(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	uint64_t id_session;
	int r;

	if (pid < 0)
		return -EINVAL;

	if ((r = getsid_internal((uint64_t)pid, &id_session)) != 0) {
		if (r == DE_SEARCH)
			return -ESRCH;
		return -EINVAL;
	}

	return (long long)id_session;
}

static long long dancy_syscall_openpty(va_list va)
{
	int *out_fd = va_arg(va, int *);
	char *name = va_arg(va, char *);
	const struct __dancy_termios *termios_p = va_arg(va, const void *);
	const struct __dancy_winsize *winsize_p = va_arg(va, const void *);
	int fd[2], r;

	if (((addr_t)out_fd % (addr_t)sizeof(int)) != 0)
		return -EFAULT;

	if (pg_check_user_write(out_fd, sizeof(int) + sizeof(int)))
		return -EFAULT;

	if (termios_p != NULL) {
		if (((addr_t)termios_p % (addr_t)sizeof(int)) != 0)
			return -EFAULT;
		if (pg_check_user_read(termios_p, sizeof(*termios_p)))
			return -EFAULT;
	}

	if (winsize_p != NULL) {
		if (((addr_t)winsize_p % (addr_t)sizeof(short)) != 0)
			return -EFAULT;
		if (pg_check_user_read(winsize_p, sizeof(*winsize_p)))
			return -EFAULT;
	}

	if (name == NULL) {
		r = file_openpty(fd, NULL, termios_p, winsize_p);

	} else {
		char buffer[16];

		if (pg_check_user_write(name, sizeof(buffer)))
			return -EFAULT;

		if ((r = file_openpty(fd, buffer, termios_p, winsize_p)) == 0)
			strcpy(name, &buffer[0]);
	}

	if (r != 0) {
		if (r == DE_OVERFLOW)
			return -EMFILE;
		if (r == DE_MEMORY)
			return -ENOMEM;
		return -ENOENT;
	}

	out_fd[0] = fd[0];
	out_fd[1] = fd[1];

	return 0;
}

static long long dancy_syscall_memusage(va_list va)
{
	int flags = va_arg(va, int);
	uint64_t r = task_current()->pg_user_memory;

	if (flags != 0)
		return -EINVAL;

	if ((unsigned long long)r > (unsigned long long)(LLONG_MAX))
		r = (uint64_t)(LLONG_MAX);

	return (long long)r;
}

static long long dancy_syscall_reboot(va_list va)
{
	int request = va_arg(va, int);
	long long arg = va_arg(va, long long);
	int r;

	if ((r = reboot_internal(request, arg)) != 0) {
		if (r == DE_ACCESS)
			return -EPERM;
		return -EINVAL;
	}

	kernel->panic("__dancy_syscall_reboot: unexpected behavior");

	return 0;
}

static long long dancy_syscall_sigpending(va_list va)
{
	__dancy_sigset_t *out = va_arg(va, void *);

	struct task *current = task_current();
	uint32_t asm_data3 = (uint32_t)current->asm_data3;
	uint32_t mask = current->sig.mask;

	if (((addr_t)out % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_write(out, sizeof(__dancy_sigset_t)))
		return -EFAULT;

	*out = (__dancy_sigset_t)(asm_data3 & mask);

	return 0;
}

static long long dancy_syscall_sigprocmask(va_list va)
{
	int how = va_arg(va, int);
	const __dancy_sigset_t *set = va_arg(va, const void *);
	__dancy_sigset_t *out = va_arg(va, void *);

	struct task *current = task_current();
	uint32_t mask = current->sig.mask;
	uint32_t s = 0;

	if (how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
		return -EINVAL;

	if (set) {
		if (((addr_t)set % (addr_t)sizeof(void *)) != 0)
			return -EFAULT;

		if (pg_check_user_read(set, sizeof(__dancy_sigset_t)))
			return -EFAULT;

		s = (uint32_t)(*set);
	}

	if (out) {
		if (((addr_t)out % (addr_t)sizeof(void *)) != 0)
			return -EFAULT;

		if (pg_check_user_write(out, sizeof(__dancy_sigset_t)))
			return -EFAULT;

		*out = (__dancy_sigset_t)mask;
	}

	if (set) {
		if (how == SIG_UNBLOCK) {
			current->sig.mask = (mask & (~s));
			return 0;
		}

		s &= ~(((uint32_t)1) << (SIGKILL - 1));
		s &= ~(((uint32_t)1) << (SIGSTOP - 1));

		if (how == SIG_BLOCK) {
			current->sig.mask = (mask | s);
			return 0;
		}

		if (how == SIG_SETMASK) {
			current->sig.mask = s;
			return 0;
		}
	}

	return 0;
}

static long long dancy_syscall_proclist(va_list va)
{
	__dancy_pid_t *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int r;

	if (size == 0)
		return -ENOMEM;

	if (((addr_t)buffer % (addr_t)sizeof(void *)) != 0)
		return -EFAULT;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if ((r = proclist_internal(&size, buffer)) != 0) {
		if (r == DE_MEMORY)
			return -ENOMEM;
		return -EACCES;
	}

	return (long long)size;
}

static long long dancy_syscall_procinfo(va_list va)
{
	__dancy_pid_t pid = va_arg(va, __dancy_pid_t);
	int request = va_arg(va, int);
	void *buffer = va_arg(va, void *);
	size_t size = va_arg(va, size_t);
	int r;

	if (size == 0)
		return -ENOMEM;

	if (pg_check_user_write(buffer, size))
		return -EFAULT;

	if ((r = procinfo_internal(pid, request, &size, buffer)) != 0) {
		if (r == DE_MEMORY)
			return -ENOMEM;
		if (r == DE_SEARCH)
			return -ESRCH;
		return -EACCES;
	}

	return (long long)size;
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
	{ dancy_syscall_unlink },
	{ dancy_syscall_rename },
	{ dancy_syscall_stat },
	{ dancy_syscall_sleep },
	{ dancy_syscall_mmap },
	{ dancy_syscall_munmap },
	{ dancy_syscall_mprotect },
	{ dancy_syscall_msync },
	{ dancy_syscall_getpid },
	{ dancy_syscall_getppid },
	{ dancy_syscall_realpath },
	{ dancy_syscall_kill },
	{ dancy_syscall_poll },
	{ dancy_syscall_ioctl },
	{ dancy_syscall_getpgid },
	{ dancy_syscall_getsid },
	{ dancy_syscall_openpty },
	{ dancy_syscall_memusage },
	{ dancy_syscall_reboot },
	{ dancy_syscall_sigpending },
	{ dancy_syscall_sigprocmask },
	{ dancy_syscall_proclist },
	{ dancy_syscall_procinfo },
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
