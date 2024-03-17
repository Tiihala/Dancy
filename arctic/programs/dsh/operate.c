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

static int state = 1;
static int exit_code = 0;

static void welcome(void)
{
	fputs("\tWelcome to the Dancy Operating System!\n\n", stdout);
}

static int dsh_chdir(char **argv)
{
	const char *path = argv[1] ? argv[1] : "";

	if (path[0] == '\0')
		return 1;

	if ((errno = 0, chdir(path)) == -1) {
		const char *enoent = "no such file or directory";
		const char *enotdir = "not a directory";

		if (errno == ENOENT)
			fprintf(stderr, "chdir: %s\n", enoent);
		else if (errno == ENOTDIR)
			fprintf(stderr, "chdir: %s\n", enotdir);
		else
			perror("chdir");

		return 1;
	}

	return 0;
}

static int dsh_clear(void)
{
	fputs("\033c", stdout);

	return 0;
}

static int dsh_echo(char **argv)
{
	size_t i;

	for (i = 1; argv[i] != NULL; i++) {
		const char *a = argv[i];

		if (a[0] == '\0')
			continue;

		if (i > 1)
			fputs(" ", stdout);

		if (!strcmp(a, "$?")) {
			fprintf(stdout, "%d", exit_code);
			continue;
		}

		if (!strcmp(a, "$$")) {
			fprintf(stdout, "%lld", (long long)getpid());
			continue;
		}

		fputs(a, stdout);
	}

	fputs("\n", stdout);

	return 0;
}

static void dsh_execute_spawn(const char *path, char **argv)
{
	extern char **environ;
	pid_t pid, wpid;
	int r;

	r = posix_spawn(&pid, path, NULL, NULL, argv, environ);

	if (r == ENOENT) {
		fputs("dsh: command not found...\n", stderr);
		return;
	}

	if (r != 0) {
		fprintf(stderr, "dsh: %s\n", strerror(r));
		return;
	}

	for (;;) {
		int status = 0;

		errno = 0;
		wpid = waitpid(pid, &status, 0);

		if (wpid == -1) {
			perror("waitpid");
			exit_code = 1;
			break;
		}

		if (wpid != pid)
			continue;

		if (WIFEXITED(status)) {
			exit_code = WEXITSTATUS(status);
			break;
		}

		if (WIFSIGNALED(status)) {
			exit_code = 128 + WTERMSIG(status);
			break;
		}
	}
}

static void dsh_execute(char **argv)
{
	const char *path = argv[0];
	char cmd[512];

	exit_code = 1;

	if (path[0] != '.' && path[0] != '/') {
		if (strlen(path) > 255) {
			fputs("dsh: command not found...\n", stderr);
			return;
		}
		strcpy(&cmd[0], "/bin/");
		strcat(&cmd[0], path);
		path = &cmd[0];
	}

	dsh_execute_spawn(path, argv);
}

int operate(struct options *opt)
{
	int r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	welcome();

	while (state != 0) {
		char *buffer, **new_argv;
		char prompt[2048];
		char cwd[256];
		const size_t cwdmax = 32;
		size_t offset;

		if ((errno = 0, getcwd(&cwd[0], sizeof(cwd))) == NULL)
			return perror("getcwd"), EXIT_FAILURE;

		if ((offset = strlen(&cwd[0])) > cwdmax) {
			offset = offset - cwdmax + 3;
			memmove(&cwd[0], &cwd[offset], cwdmax - 2);
			offset = cwdmax;

			r = snprintf(&prompt[0], sizeof(prompt),
				"\033[1;32m[\033[1;90m..."
				"\033[1;32m%s]$\033[0m ", &cwd[0]);
		} else {
			r = snprintf(&prompt[0], sizeof(prompt),
				"\033[1;32m[%s]$\033[0m ", &cwd[0]);
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

		if (new_argv[0] != NULL) {
			if (!strcmp(new_argv[0], "exit"))
				state = 0;
			else if (!strcmp(new_argv[0], "cd"))
				exit_code = dsh_chdir(new_argv);
			else if (!strcmp(new_argv[0], "chdir"))
				exit_code = dsh_chdir(new_argv);
			else if (!strcmp(new_argv[0], "clear"))
				exit_code = dsh_clear();
			else if (!strcmp(new_argv[0], "cls"))
				exit_code = dsh_clear();
			else if (!strcmp(new_argv[0], "echo"))
				exit_code = dsh_echo(new_argv);
			else
				dsh_execute(new_argv);
		}

		free(new_argv);
		free(buffer);
	}

	return 0;
}
