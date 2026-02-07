/*
 * Copyright (c) 2024, 2025, 2026 Antti Tiihala
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
 * dsh/builtin.c
 *      The Dancy Shell
 */

#include "main.h"

static int cmd_chdir(int argc, char *argv[])
{
	const char *path = (argc > 1) ? argv[1] : NULL;

	if (argc > 2)
		return fputs("chdir: too many arguments\n", stderr), 1;

	if (path == NULL && (path = dsh_var_read("HOME")) == NULL)
		return fputs("chdir: HOME not set\n", stderr), 1;

	if (path[0] != '\0' && (errno = 0, chdir(path)) == -1)
		return perror("chdir"), 1;

	return 0;
}

static int cmd_clear(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	fputs("\033c", stdout);

	return 0;
}

static int cmd_echo(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		const char *a = argv[i];

		if (a[0] == '\0')
			continue;

		if (i > 1)
			fputs(" ", stdout);

		fputs(a, stdout);
	}

	if (fputs("\n", stdout) == EOF) {
		fprintf(stderr, "echo: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return 0;
}

static int cmd_exit(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	dsh_operate_state = 0;

	return 0;
}

static int cmd_export(int argc, char *argv[])
{
	const char *usage = "Usage: export [NAME[=VALUE]...]\n";
	int i;

	if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-p"))) {
		char **e = dsh_environ();

		while (*e != NULL)
			fprintf(stdout, "%s\n", *e++);

		return 0;
	}

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (arg[0] != '-')
			continue;

		if (!strcmp(arg, "--help'"))
			return fputs(usage, stdout), 0;

		return fputs(usage, stderr), EXIT_FAILURE;
	}

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];
		size_t size = 0;

		char name[256];
		const char *value = strchr(arg, '=');
		const size_t flags = 1;

		if (arg[0] == '\0')
			continue;

		while (size + 1 < sizeof(name)) {
			char c = arg[size];
			int valid = 0;

			if (c == '\0' || c == '=')
				break;

			if ((c >= '0' && c <= '9' && size > 0) || c == '_')
				valid = 1;

			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
				valid = 1;

			if (!valid) {
				size = 0;
				break;
			}

			name[size++] = c;
			name[size] = '\0';
		}

		if (size == 0) {
			fprintf(stderr, "export: \"%s\" not valid\n", arg);
			return EXIT_FAILURE;
		}

		if (value != NULL)
			value += 1;

		if (!dsh_var_write(&name[0], value, flags)) {
			fprintf(stderr, "export: \"%s\" not written\n", arg);
			return EXIT_FAILURE;
		}
	}

	return 0;
}

static int cmd_kill(int argc, char *argv[])
{
	const char *usage = "Usage: kill [-SIGNAL_NUMBER] PID...\n";
	int i = 1, signal_number = SIGTERM;

	if (argc < 2)
		return fputs(usage, stderr), EXIT_FAILURE;

	if (argv[1][0] == '-') {
		signal_number = 0;

		while (argv[1][i] != '\0') {
			char c = argv[1][i];

			if (c < '0' || c > '9')
				return fputs(usage, stderr), EXIT_FAILURE;

			signal_number *= 10;
			signal_number += ((int)c - '0');

			if (signal_number >= 128)
				return fputs(usage, stderr), EXIT_FAILURE;

			i += 1;
		}

		i = 2;
	}

	while (argv[i] != NULL) {
		long long pid = (errno = 0, strtoll(argv[i], NULL, 10));

		i += 1;

		if (errno != 0) {
			fprintf(stderr, "kill: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

		if (kill((pid_t)pid, signal_number) == 0)
			continue;

		fprintf(stderr, "kill(%lld, %d): %s\n",
			pid, signal_number, strerror(errno));
	}

	return 0;
}

static int cmd_rename(int argc, char *argv[])
{
	const char *usage = "Usage: rename OLD_FILE NEW_FILE\n";

	if (argc != 3)
		return fputs(usage, stderr), EXIT_FAILURE;

	if (rename(argv[1], argv[2]) != 0) {
		fprintf(stderr, "rename: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	return 0;
}

static int cmd_unlink(int argc, char *argv[])
{
	const char *usage = "Usage: unlink FILE\n";

	if (argc != 2)
		return fputs(usage, stderr), EXIT_FAILURE;

	if (unlink(argv[1]) != 0) {
		fprintf(stderr, "unlink: %s: %s\n", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	return 0;
}

static int cmd_unset(int argc, char *argv[])
{
	const char *usage = "Usage: unset NAME...\n";
	int i;

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (arg[0] == '-') {
			if (!strcmp(arg, "--help'"))
				return fputs(usage, stdout), 0;
			else
				return fputs(usage, stderr), EXIT_FAILURE;
		}
	}

	for (i = 1; i < argc; i++) {
		const char *name = argv[i];
		const char *value = NULL;
		const size_t flags = 2;

		(void)dsh_var_write(name, value, flags);
	}

	return 0;
}

static int cmd_default(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	return 0;
}

struct dsh_builtin dsh_builtin_array[] = {
	{ cmd_chdir, "cd" },
	{ cmd_chdir, "chdir" },
	{ cmd_clear, "clear" },
	{ cmd_clear, "cls" },
	{ cmd_clear, "reset" },
	{ cmd_echo, "echo" },
	{ cmd_exit, "exit" },
	{ cmd_export, "export" },
	{ cmd_kill, "kill" },
	{ cmd_rename, "rename" },
	{ cmd_unlink, "unlink" },
	{ cmd_unset, "unset" },
	{ cmd_default, NULL }
};
