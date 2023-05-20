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
 * run/run.c
 *      Run the first user-space executable
 */

#include <dancy.h>

static const char *exe_name = "/bin/init";

static void build_arg_header(cpu_native_t *argh, cpu_native_t size)
{
	argh[0] = 1;
	argh[1] = (cpu_native_t)(&argh[4]);
	argh[2] = (cpu_native_t)(&argh[6]);
	argh[3] = size;
	argh[4] = (cpu_native_t)(&argh[8]);
	argh[5] = 0;
	argh[6] = 0;
	argh[7] = 0;

	strcpy((void *)(&argh[8]), exe_name);
}

static int open_file_descriptors(void)
{
	int fd, r;

	if ((r = file_open(&fd, "/dev/null", O_RDONLY, 0)) != 0)
		return r;

	if (fd != 0)
		return DE_UNEXPECTED;

	if ((r = file_open(&fd, "/dev/console", O_WRONLY, 0)) != 0)
		return r;

	if (fd != 1)
		return DE_UNEXPECTED;

	if ((r = file_open(&fd, "/dev/console", O_WRONLY, 0)) != 0)
		return r;

	if (fd != 2)
		return DE_UNEXPECTED;

	return 0;
}

int run_init(void)
{
	static int run_once;
	struct vfs_node *node;
	addr_t user_ip, user_sp;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (task_current()->id != 1)
		return DE_UNEXPECTED;

	if ((r = pg_create()) != 0)
		return r;

	if ((r = vfs_open(exe_name, &node, 0, 0)) != 0)
		return (r != DE_NAME) ? r : 0;

	if ((r = coff_load_executable(node, &user_ip)) != 0)
		return r;

	{
		const cpu_native_t top = 0x80000000;
		const cpu_native_t size = 0x100;

		cpu_native_t *argh = (void *)(top - size);

		user_sp = (addr_t)(argh - 1);
		build_arg_header(argh, size);
	}

	if ((r = open_file_descriptors()) != 0)
		return r;

	task_jump(user_ip, user_sp);

	return 0;
}
