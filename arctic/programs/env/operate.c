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
 * env/operate.c
 *      Set the environment for command invocation
 */

#include "main.h"

extern char **environ;

static char *new_environ[0x20000];
static size_t new_environ_count;

static void append_new_environ(const char *v)
{
	size_t m = sizeof(new_environ) / sizeof(new_environ[0]);

	if (new_environ_count >= m - 1) {
		fputs("env: too many environment variables\n", stderr);
		exit(EXIT_FAILURE);
	}

	new_environ[new_environ_count++] = (char *)v;
}

static void call_exec(char **utility)
{
	char buffer[1024];
	const char *path = utility[0];
	int my_errno;

	if (path[0] == '\0')
		return;

	if (!strchr(path, '/') && strlen(path) < sizeof(buffer) - 16) {
		strcpy(&buffer[0], "/bin/");
		strcat(&buffer[0], path);
		path = &buffer[0];
	}

	errno = 0;
	execve(path, utility, &new_environ[0]);
	my_errno = errno;

	fprintf(stderr, "env: \'%s\': %s\n", utility[0], strerror(my_errno));

	if (my_errno == ENOENT || my_errno == ENOTDIR)
		exit(127);

	exit(126);
}

int operate(struct options *opt)
{
	char **utility = NULL;
	size_t i, j;

	if (!opt->ignore_environ) {
		for (i = 0; environ[i] != NULL; i++)
			append_new_environ(environ[i]);
	}

	for (i = 0; opt->operands[i] != NULL; i++) {
		const char *s = opt->operands[i];
		const char *e = strchr(s, '=');
		int state = 0;

		if (e == NULL) {
			utility = &opt->operands[i];
			break;
		}

		for (j = 0; j < new_environ_count; j++) {
			if (!strncmp(new_environ[j], s, (size_t)(e - s))) {
				new_environ[j] = (char *)s;
				state = 1;
			}
		}

		if (state == 0)
			append_new_environ(s);
	}

	if (utility == NULL) {
		for (i = 0; new_environ[i] != NULL; i++)
			printf("%s\n", new_environ[i]);
		return 0;
	}

	call_exec(utility);

	return 0;
}
