/*
 * Copyright (c) 2026 Antti Tiihala
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
 * libc/stdlib/setenv.c
 *      Add or change environment variable
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

static char **new_environ = NULL;
static char *empty_environ[] = { NULL };

static int check_name(const char *name)
{
	if (name == NULL || name[0] == '\0' || strchr(name, '='))
		return -1;

	return 0;
}

static char **find_entry(const char *name)
{
	size_t i, n = strlen(name);

	for (i = 0; environ[i] != NULL; i++) {
		char *e = environ[i];

		if (strncmp(e, name, n))
			continue;

		if (e[n] == '=')
			return &environ[i];
	}

	return NULL;
}

int setenv(const char *name, const char *value, int overwrite)
{
	char *b, **e;

	if (environ == NULL)
		environ = &empty_environ[0];

	if (check_name(name) || value == NULL)
		return (errno = EINVAL), -1;

	if ((b = malloc(strlen(name) + strlen(value) + 2)) == NULL)
		return (errno = ENOMEM), -1;

	strcpy(b, name), strcat(b, "="), strcat(b, value);

	if ((e = find_entry(name)) == NULL) {
		size_t n = 2;

		while (environ[n - 2] != NULL)
			n += 1;

		e = new_environ;

		if ((new_environ = malloc(n * sizeof(char *))) == NULL) {
			new_environ = e, free(b);
			return (errno = ENOMEM), -1;
		}

		new_environ[n - 1] = NULL;
		new_environ[n - 2] = b;

		while (--n >= 2)
			new_environ[n - 2] = environ[n - 2];

		environ = new_environ;

		return free(e), 0;
	}

	if (overwrite)
		return (*e = b), 0;

	return free(b), 0;
}

int unsetenv(const char *name)
{
	char **e;

	if (environ == NULL)
		environ = &empty_environ[0];

	if (check_name(name))
		return (errno = EINVAL), -1;

	if ((e = find_entry(name)) != NULL) {
		while (*e != NULL)
			*e = *(e + 1), e += 1;
	}

	return 0;
}
