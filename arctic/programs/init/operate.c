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

static void write_console_message(int i)
{
	int fd, r;

	if (i >= 0 && i < 6) {
		char console[24];
		char message[32];

		r = snprintf(&console[0], sizeof(console),
			"/dev/dancy-console-%d", i + 1);

		if (r != 20)
			return;

		r = snprintf(&message[0], sizeof(message),
			"\033c\033[4mDancy Console #%d\033[0m\n\n", i + 1);

		if (r != 28)
			return;

		if ((fd = open(&console[0], O_WRONLY)) >= 0) {
			write(fd, &message[0], (size_t)r);
			close(fd);
		}
	}
}

static pid_t start_dsh(char *console, char *number)
{
	const char *path = "/bin/terminal";
	const char *a[] = { path, number, "/bin/dsh", NULL };

	pid_t new_pid = -1;
	int r;

	posix_spawn_file_actions_t ac;
	posix_spawnattr_t at;

	posix_spawn_file_actions_init(&ac);
	posix_spawn_file_actions_addopen(&ac, 0, "/dev/null", O_RDWR, 0);
	posix_spawn_file_actions_addopen(&ac, 1, console, O_RDWR, 0);
	posix_spawn_file_actions_addopen(&ac, 2, console, O_RDWR, 0);

	posix_spawnattr_init(&at);
	posix_spawnattr_setflags(&at, POSIX_SPAWN_SETSID);

	r = posix_spawn(&new_pid, path, &ac, &at, (char **)a, NULL);

	if (r != 0 || new_pid < 0)
		return -1;

	return new_pid;
}

static void process_consoles(void)
{
	pid_t state[6] = { -1, -1, -1, -1, -1, -1 };
	int i = 0;

	struct timespec t;
	memset(&t, 0, sizeof(t));

	for (;;) {
		char console[24];
		char number[8];
		int j, r;

		for (;;) {
			pid_t pid = waitpid(-1, NULL, WNOHANG);

			if (pid <= 0)
				break;

			for (j = 0; j < 6; j++) {
				if (state[j] == pid) {
					write_console_message(j);
					state[j] = -1;
				}
			}
		}

		if (state[i] < 0) {
			r = snprintf(&console[0], sizeof(console),
				"/dev/dancy-console-%d", i + 1);

			if (r != 20)
				break;

			r = snprintf(&number[0], sizeof(number),
				"--%d", i + 1);

			if (r != 3)
				break;

			state[i] = start_dsh(&console[0], &number[0]);

			t.tv_sec = 0, t.tv_nsec = 100000000;
			clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
		}

		i = ((i + 1) % 6);

		if (state[0] < 0 || state[1] < 0 || state[2] < 0)
			continue;

		if (state[3] < 0 || state[4] < 0 || state[5] < 0)
			continue;

		t.tv_sec = 1, t.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &t, NULL);
	}
}

static const char *msg_begin = "\n\n\033[97m", *msg_end = "\033[0m\n\n";

int operate(struct options *opt)
{
	int i, r;

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
		const char *a[] = { path, NULL };
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

	for (i = 0; i < 6; i++)
		write_console_message(i);

	process_consoles();

	return 0;
}
