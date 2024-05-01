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

static void execute_spawn(struct dsh_execute_state *state, const char *path)
{
	extern char **environ;
	int r;

	r = posix_spawn(&state->pid, path,
		&state->actions, &state->attr, state->argv, environ);

	if (r != 0) {
		const char *e;

		if (r == ENOENT && valid_command(state->argv[0]))
			e = "command not found...";
		else
			e = strerror(r);

		if (r == ENOEXEC)
			dsh_exit_code = 126;

		state->pid = 0;

		fprintf(stderr, "dsh: %s: %s\n", state->argv[0], e);
		if (!dsh_interactive)
			dsh_operate_state = 0;
		return;
	}

	if (state->pipe_fd != NULL) {
		if (state->pipe_fd[0] >= 0)
			close(state->pipe_fd[0]), state->pipe_fd[0] = -1;
		if (state->pipe_fd[1] >= 0)
			close(state->pipe_fd[1]), state->pipe_fd[1] = -1;
	}

	if (state->no_wait) {
		dsh_exit_code = 0;
		return;
	}

	for (;;) {
		int status = 0;
		pid_t wpid = waitpid(-1, &status, 0);

		if (wpid != state->pid)
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

	while (state->pgroup > 1) {
		if (waitpid(-state->pgroup, NULL, 0) <= 0)
			break;
	}
}

void dsh_execute(struct dsh_execute_state *state)
{
	const char *path = state->argv[0];
	char cmd[512];
	size_t i;

	dsh_exit_code = 127;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		/* void */;

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

	execute_spawn(state, path);

	if (dsh_tcpgrp)
		tcsetpgrp(0, dsh_tcpgrp);

	if (dsh_exit_code >= 128)
		fputs("\n", stderr);
}
