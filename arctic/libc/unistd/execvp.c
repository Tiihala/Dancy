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
 * libc/unistd/execvp.c
 *      Execute a file
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern char **environ;

static int func(const char *dir_name, const char *file,
	int last_component, char *const argv[])
{
	struct stat s;
	char path[512];

	if (strlen(dir_name) >= 250 || strlen(file) >= 250)
		return (errno = ENOENT), -1;

	strcpy(&path[0], dir_name);
	strcat(&path[0], "/");
	strcat(&path[0], file);

	if (stat(&path[0], &s) == -1 && errno == ENOENT)
		return last_component ? -1 : 0;

	return execve(&path[0], argv, environ), -1;
}

int execvp(const char *file, char *const argv[])
{
	if (file[0] == '\0')
		return (errno = ENOENT), -1;

	if (!strchr(file, '/')) {
		const char *p1 = getenv("PATH");
		char *p2, *p3, *p4;

		if (p1 == NULL || *p1 == '\0')
			p1 = "/bin";

		if ((p2 = strdup(p1), p3 = p2) == NULL)
			return (errno = ENOMEM), -1;

		while (p3 && *p3 != '\0') {
			if ((p4 = strchr(p3, ':')) != NULL)
				*p4++ = '\0';
			else
				p4 = strchr(p3, '\0');

			if (*p3 == '/' && p4 != NULL) {
				if (func(p3, file, *p4 == '\0', argv)) {
					int my_errno = errno;
					free(p2), (errno = my_errno);
					return -1;
				}
			}

			p3 = p4;
		}

		free(p2);

	} else {
		return execve(file, argv, environ);
	}

	return (errno = ENOENT), -1;
}
