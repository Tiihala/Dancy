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
 * ld-dancy/operate.c
 *      The executable loader
 */

#include "main.h"

static void not_implemented(struct options *opt)
{
	int i;

	for (i = 0; opt->operands[i] != NULL; i++)
		fprintf(stderr, "%s%d: \"%s\"\n", i ? "" : "\n",
			i, opt->operands[i]);

	fprintf(stderr, "\nThe loader has not been implemented yet.\n");
}

int operate(struct options *opt)
{
	if (opt->operands[0] == NULL)
		return opt->error = "missing program", EXIT_FAILURE;

	not_implemented(opt);

	return 0;
}
