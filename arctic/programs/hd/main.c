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
 * hd/main.c
 *      Execute /bin/hexdump -C
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;

int main(int argc, char *argv[])
{
	const size_t size = ((size_t)argc + 2) * sizeof(char *);
	char **new_argv;
	char new_argv1[] = "-C";
	int i;

	if (argc < 1 || argv[argc] != NULL)
		return EXIT_FAILURE;

	if ((new_argv = malloc(size)) == NULL)
		return fputs("hd: out of memory\n", stderr), EXIT_FAILURE;

	new_argv[0] = argv[0];
	new_argv[1] = &new_argv1[0];

	for (i = 2; /* void */; i++) {
		if ((new_argv[i] = argv[i - 1]) == NULL)
			break;
	}

	errno = 0;
	execve("/bin/hexdump", new_argv, environ);

	perror("hd: execve(\"/bin/hexdump\", ...)");
	free(new_argv);

	return EXIT_FAILURE;
}
