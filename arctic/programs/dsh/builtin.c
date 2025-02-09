/*
 * Copyright (c) 2024, 2025 Antti Tiihala
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
	const char *path = (argc > 1) ? argv[1] : "";

	if (path[0] == '\0')
		return 1;

	if ((errno = 0, chdir(path)) == -1) {
		perror("chdir");
		return 1;
	}

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

		if (!strcmp(a, "$?")) {
			fprintf(stdout, "%d", dsh_exit_code);
			continue;
		}

		if (!strcmp(a, "$$")) {
			fprintf(stdout, "%lld", (long long)getpid());
			continue;
		}

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
	{ cmd_rename, "rename" },
	{ cmd_unlink, "unlink" },
	{ cmd_default, NULL }
};
