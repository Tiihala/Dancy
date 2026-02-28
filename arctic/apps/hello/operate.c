/*
 * Copyright (c) 2023 Antti Tiihala
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
 * hello/operate.c
 *      The "hello, world" application
 */

#include "main.h"

static int call_sleep(time_t tv_sec, long tv_nsec)
{
	struct timespec request = { tv_sec, tv_nsec };

	for (;;) {
		struct timespec remain = { 0, 0 };
		int r;

		r = clock_nanosleep(CLOCK_MONOTONIC, 0, &request, &remain);

		if (r == 0)
			break;

		if (r != EINTR) {
			fprintf(stderr, MAIN_CMDNAME
				": clock_nanosleep: %s\n", strerror(r));
			return EXIT_FAILURE;
		}

		if (remain.tv_sec == 0 && remain.tv_nsec == 0)
			break;

		memcpy(&request, &remain, sizeof(struct timespec));
	}

	return 0;
}

int operate(struct options *opt)
{
	int i;

	if (opt->sleep)
		call_sleep(10, 0);

	if (opt->upcase)
		fputs("HELLO, WORLD\n", stdout);
	else
		fputs("hello, world\n", stdout);

	for (i = 0; opt->operands[i] != NULL; i++)
		printf("%s%d: \"%s\"\n", i ? "" : "\n", i, opt->operands[i]);

	return 0;
}
