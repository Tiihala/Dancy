/*
 * Copyright (c) 2018 Antti Tiihala
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
 * cmd/program.c
 *      Template program for command-line argument parsing
 */

#include <stdio.h>
#include <string.h>

#include "program.h"

#define NR_MESSAGES 4
static const char *messages[NR_MESSAGES];

int optfnc_m(struct options *opt, const char *arg)
{
	if (opt->nr_m >= NR_MESSAGES) {
		opt->error = "too many additional messages";
		return 1;
	}
	messages[opt->nr_m++] = arg;
	return 0;
}

int program(struct options *opt)
{
	int i;
	FILE *out = stdout;

	if (opt->arg_o && strcmp(opt->arg_o, "-"))
		if ((out = fopen(opt->arg_o, "w")) == NULL)
			return perror("Error"), 1;

	fprintf(out, "Option A: %s\n", (opt->a) ? "enabled" : "disabled");
	fprintf(out, "Option B: %s\n", (opt->b) ? "enabled" : "disabled");

	for (i = 0; i < opt->nr_m; i++)
		fprintf(out, "Message[%i]: %s\n", i, messages[i]);

	if (opt->operands && *opt->operands && *(opt->operands + 1)) {
		fputs("\n", out);
		for (i = 1; opt->operands[i]; i++)
			fprintf(out, "%s ", opt->operands[i]);
		fputs("\n", out);
	}

	if (out != stdout)
		fclose(out);
	return 0;
}
