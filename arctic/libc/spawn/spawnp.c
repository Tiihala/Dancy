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
 * libc/spawn/spawnp.c
 *      The implementation of the spawnp function
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <spawn.h>

int posix_spawnp(pid_t *pid, const char *file,
	const posix_spawn_file_actions_t *actions,
	const posix_spawnattr_t *attrp,
	char *const argv[], char *const envp[])
{
	struct __dancy_spawn_options opts;
	const char *path;
	long long r;

	opts.actions = actions;
	opts.attrp = attrp;

	{
		size_t i = 0;

		while (file[i] != '/') {
			if (file[i++] == '\0')
				return (errno = ENOSYS), ENOSYS;
		}

		path = file;
	}

	r = __dancy_syscall4(__dancy_syscall_spawn, path, argv, envp, &opts);

	if (r < 0)
		errno = -((int)r), r = -((int)r);

	if (pid)
		*pid = (pid_t)r;

	return 0;
}
