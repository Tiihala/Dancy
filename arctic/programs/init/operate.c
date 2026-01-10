/*
 * Copyright (c) 2023, 2024, 2026 Antti Tiihala
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
 * init/operate.c
 *      The init program
 */

#include "main.h"

static const char *msg_begin = "\n\n\033[97m", *msg_end = "\033[0m\n\n";

int operate(struct options *opt)
{
	int r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	if (getpid() != 1) {
		opt->error = "process ID is not 1";
		return EXIT_FAILURE;
	}

	{
		const char *path = "/bin/terminal";
		const char *a[] = { path, "/bin/dsh", NULL };
		pid_t new_pid = -1;

		posix_spawnattr_t at;
		posix_spawnattr_init(&at);
		posix_spawnattr_setflags(&at, POSIX_SPAWN_SETSID);

		r = posix_spawn(&new_pid, path, NULL, &at, (char **)a, NULL);

		if (r != 0 || new_pid < 0) {
			fprintf(stderr, "%sinit: \'%s\' failed (%s)\n%s",
				msg_begin, path, strerror(r), msg_end);
			return 0;
		}

		for (;;) {
			pid_t pid = waitpid(-1, NULL, 0);

			if (pid == new_pid)
				break;
		}

		for (;;) {
			pid_t pid = waitpid(-1, NULL, WNOHANG);

			if (pid <= 0)
				break;
		}
	}

	return 0;
}
