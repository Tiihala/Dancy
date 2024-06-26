/*
 * Copyright (c) 2022, 2024 Antti Tiihala
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
 * libc/spawn/addopen.c
 *      The spawn file actions
 */

#include <errno.h>
#include <spawn.h>
#include <string.h>

int posix_spawn_file_actions_addopen(
	posix_spawn_file_actions_t *actions, int fd,
	const char *path, int flags, mode_t mode)
{
	size_t m = sizeof(actions->_actions) / sizeof(actions->_actions[0]);
	void *p = NULL;
	unsigned int i;

	if (fd < 0 || fd >= __DANCY_OPEN_MAX)
		return (errno = EBADF), EBADF;

	if (actions->_count >= (unsigned int)m)
		return (errno = ENOMEM), ENOMEM;

	if (path != NULL && (p = strdup(path)) == NULL)
		return (errno = ENOMEM), ENOMEM;

	i = actions->_count++;

	actions->_actions[i]._type = __DANCY_SPAWN_ADD_OPEN;
	actions->_actions[i]._args[0] = fd;
	actions->_actions[i]._args[1] = flags;
	actions->_actions[i]._args[2] = (int)mode;
	actions->_actions[i]._path = p;

	return 0;
}
