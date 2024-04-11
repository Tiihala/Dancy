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
 * dsh/execute.c
 *      The Dancy Shell
 */

#include "main.h"

static int valid_command(const char *arg)
{
	if (arg[0] == '\0' || arg[0] == '/')
		return 0;

	if (arg[0] == '.') {
		size_t i = 1;

		while (arg[i] == '.')
			i += 1;

		if (arg[i] == '/')
			return 0;
	}

	return 1;
}

static void execute_spawn(const char *path, char **argv)
{
	extern char **environ;
	pid_t tc_pgrp = tcgetpgrp(STDIN_FILENO);
	pid_t pid, wpid;
	int r;

	posix_spawn_file_actions_t actions;
	posix_spawnattr_t attr;

	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_addtcsetpgrp_np(&actions, STDIN_FILENO);

	posix_spawnattr_init(&attr);
	posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
	posix_spawnattr_setpgroup(&attr, 0);

	r = posix_spawn(&pid, path, &actions, &attr, argv, environ);

	posix_spawn_file_actions_destroy(&actions);
	posix_spawnattr_destroy(&attr);

	if (r != 0) {
		const char *e;

		tcsetpgrp(STDIN_FILENO, tc_pgrp);

		if (r == ENOENT && valid_command(argv[0]))
			e = "command not found...";
		else
			e = strerror(r);

		fprintf(stderr, "dsh: %s: %s\n", argv[0], e);
		if (!dsh_interactive)
			dsh_operate_state = 0;
		return;
	}

	for (;;) {
		int status = 0;

		errno = 0;
		wpid = waitpid(pid, &status, 0);

		if (wpid != pid)
			continue;

		if (WIFEXITED(status)) {
			dsh_exit_code = WEXITSTATUS(status);
			break;
		}

		if (WIFSIGNALED(status)) {
			dsh_exit_code = 128 + WTERMSIG(status);
			break;
		}
	}

	tcsetpgrp(STDIN_FILENO, tc_pgrp);
}

void dsh_execute(char **argv)
{
	const char *path = argv[0];
	char cmd[512];
	size_t i;

	dsh_exit_code = 1;

	for (i = 0; path[i] != '\0'; i++) {
		char c = path[i];

		if ((c >= 0x01 && c <= 0x1F) || c == 0x7F) {
			fputs("dsh: unexpected characters...\n", stderr);
			if (!dsh_interactive)
				dsh_operate_state = 0;
			return;
		}
	}

	if (valid_command(path)) {
		if (strlen(path) > 255) {
			fputs("dsh: command name overflow...\n", stderr);
			if (!dsh_interactive)
				dsh_operate_state = 0;
			return;
		}
		strcpy(&cmd[0], "/bin/");
		strcat(&cmd[0], path);
		path = &cmd[0];
	}

	execute_spawn(path, argv);

	if (dsh_exit_code >= 128)
		fputs("\n", stderr);
}
