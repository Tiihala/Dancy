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
 * libc/spawn/actions.c
 *      The file action functions
 */

#include <spawn.h>
#include <stdlib.h>
#include <string.h>

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *actions)
{
	memset(actions, 0x00, sizeof(*actions));

	return 0;
}

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *actions)
{
	size_t m = sizeof(actions->_actions) / sizeof(actions->_actions[0]);
	unsigned int i;

	if (actions->_count <= (unsigned int)m) {
		for (i = 0; i < actions->_count; i++) {
			void *path = (void *)actions->_actions[i]._path;
			if (path != NULL)
				free(path);
		}
	}

	memset(actions, 0xFF, sizeof(*actions));

	return 0;
}
