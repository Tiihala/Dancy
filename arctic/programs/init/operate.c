/*
 * Copyright (c) 2023, 2024 Antti Tiihala
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

	for (;;) {
		const char *a[] = { "/bin/terminal", "/bin/dsh", NULL };
		pid_t new_pid = -1;
		int status = -1;

		r = posix_spawn(&new_pid, a[0], NULL, NULL, (char **)a, NULL);

		if (r != 0 || new_pid < 0)
			break;

		for (;;) {
			pid_t pid = waitpid(-1, &status, 0);

			if (pid == new_pid)
				break;
		}

		printf("\033cinit: \'%s\' exited with return code %d\n"
			"init: restarting the Dancy Shell\n\n", a[0], status);
	}

	return 0;
}
