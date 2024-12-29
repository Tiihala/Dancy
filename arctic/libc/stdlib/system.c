/*
 * Copyright (c) 2024 Antti Tiihala
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
 * libc/stdlib/system.c
 *      The C Standard Library
 */

#include <errno.h>
#include <spawn.h>
#include <stdlib.h>
#include <sys/wait.h>

int system(const char *command)
{
	const char *a[] = { "sh", "-c", command, NULL };
	int saved_errno = errno;
	pid_t p = -1;
	int r;

	if (command == NULL)
		return 1;

	r = posix_spawn(&p, "/bin/sh", NULL, NULL, (char **)a, NULL);

	if (r) {
		a[0] = "dsh";
		r = posix_spawn(&p, "/bin/dsh", NULL, NULL, (char **)a, NULL);
	}

	if (r) {
		errno = r;
		return -1;
	}

	if (p < 0) {
		errno = EIO;
		return -1;
	}

	for (;;) {
		int status = 0;
		pid_t w = waitpid(p, &status, 0);

		if (w == p && WIFEXITED(status)) {
			errno = saved_errno;
			return status;
		}

		if (w < 0 && errno != EINTR)
			break;
	}

	return -1;
}
