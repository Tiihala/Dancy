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
 * mkdir/operate.c
 *      Create directories
 */

#include "main.h"

static int call_mkdir(const char *path, int create_components)
{
	const mode_t mode = 0777;

	if (create_components) {
		char *p = strdup(path);
		char *e = p;

		if (p == NULL)
			return fputs("mkdir: not enough memory\n", stderr), 1;

		while ((e = strchr(&e[1], '/')) != NULL) {
			e[0] = '\0';

			if (mkdir(p, mode) != 0 && errno != EEXIST)
				return perror("mkdir"), free(p), 1;

			e[0] = '/';
		}

		free(p);
	}

	if (mkdir(path, mode) != 0)
		return perror("mkdir"), 1;

	return 0;
}

int operate(struct options *opt)
{
	int i, r = 0;

	if (opt->operands[0] == NULL) {
		opt->error = "specify the directories";
		return 1;
	}

	for (i = 0; opt->operands[i] != NULL && r == 0; i++)
		r = call_mkdir(opt->operands[i], opt->create_components);

	return r;
}
