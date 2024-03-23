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
 * dsh/operate.c
 *      The Dancy Shell
 */

#include "main.h"

int dsh_exit_code = 0;
int dsh_operate_state = 1;

static void welcome(void)
{
	fputs("\tWelcome to the Dancy Operating System!\n\n", stdout);
}

static int dsh_valid_command(const char *arg)
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

static void dsh_execute_spawn(const char *path, char **argv)
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

	if (r != 0) {
		const char *e;

		tcsetpgrp(STDIN_FILENO, tc_pgrp);

		if (r == ENOENT && dsh_valid_command(argv[0]))
			e = "command not found...";
		else
			e = strerror(r);

		fprintf(stderr, "dsh: %s: %s\n", argv[0], e);
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

static void dsh_execute(char **argv)
{
	const char *path = argv[0];
	char cmd[512];
	size_t i;

	dsh_exit_code = 1;

	for (i = 0; path[i] != '\0'; i++) {
		char c = path[i];

		if ((c >= 0x01 && c <= 0x1F) || c == 0x7F) {
			fputs("dsh: unexpected characters...\n", stderr);
			return;
		}
	}

	if (dsh_valid_command(path)) {
		if (strlen(path) > 255) {
			fputs("dsh: command name overflow...\n", stderr);
			return;
		}
		strcpy(&cmd[0], "/bin/");
		strcat(&cmd[0], path);
		path = &cmd[0];
	}

	dsh_execute_spawn(path, argv);

	if (dsh_exit_code >= 128)
		fputs("\n", stderr);
}

static void create_safe_cwd(void *out, const void *in)
{
	unsigned char *o = out;
	const unsigned char *i = in;

	for (;;) {
		unsigned int c = *i++;
		char b[8];

		*o++ = (unsigned char)c;

		if (c == 0)
			break;

		if (c <= 0x20 || c >= 0x7F) {
			if (snprintf(&b[0], sizeof(b), "\\x%02X", c) != 4)
				memcpy(&b[0], "\\x00", 4);

			memcpy(&o[-1], &b[0], 4), o += 3;
		}
	}
}

int operate(struct options *opt)
{
	int i, r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	welcome();

	while (dsh_operate_state != 0) {
		char *buffer, **new_argv;
		char prompt[2048];
		char cwd[2048], raw_cwd[256];
		const size_t cwdmax = 32;
		size_t offset;

		if ((errno = 0, getcwd(&raw_cwd[0], sizeof(raw_cwd))) == NULL)
			return perror("getcwd"), EXIT_FAILURE;

		create_safe_cwd(&cwd[0], &raw_cwd[0]);

		if ((offset = strlen(&cwd[0])) > cwdmax) {
			offset = offset - cwdmax + 3;
			memmove(&cwd[0], &cwd[offset], cwdmax - 2);
			offset = cwdmax;

			r = snprintf(&prompt[0], sizeof(prompt),
				"\r\033[2K\033[1;32m[\033[1;90m..."
				"\033[1;32m%s]$\033[0m ", &cwd[0]);
		} else {
			r = snprintf(&prompt[0], sizeof(prompt),
				"\r\033[2K\033[1;32m[%s]$\033[0m ", &cwd[0]);
		}

		offset += 4;

		if (r <= 0)
			return EXIT_FAILURE;

		if ((buffer = dsh_get_input(&prompt[0], offset)) == NULL)
			break;

		if ((new_argv = dsh_create_argv(buffer)) == NULL) {
			free(buffer);
			break;
		}

		fprintf(stdout, "\n");

		if (new_argv[0] != NULL && new_argv[0][0] != '\0') {
			int argc = 1;

			while (new_argv[argc] != NULL)
				argc += 1;

			for (i = 0; /* void */; i++) {
				const char *name = dsh_builtin_array[i].name;
				int (*cmd)(int argc, char *argv[]);

				if (name == NULL) {
					dsh_execute(new_argv);
					break;
				}

				if (!strcmp(name, new_argv[0])) {
					cmd = dsh_builtin_array[i].execute;
					dsh_exit_code = cmd(argc, new_argv);
					break;
				}
			}
		}

		free(new_argv);
		free(buffer);
	}

	return 0;
}
