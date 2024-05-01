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
 * date/operate.c
 *      Write the date and time
 */

#include "main.h"

static struct tm *lt;
static const char *format;
static char buffer[2048];

static void set_default_format(void)
{
	if (strftime(&buffer[0], sizeof(buffer), "%Z", lt) > 0)
		format = "%a %b %e %H:%M:%S %Z %Y";
	else
		format = "%a %b %e %H:%M:%S %Y";
}

static int write_date_time(void)
{
	size_t s = strftime(&buffer[0], sizeof(buffer), format, lt);

	if (s > 0 && s < sizeof(buffer))
		fprintf(stdout, "%s\n", &buffer[0]);
	else
		fputs("\n", stdout);

	return 0;
}

int operate(struct options *opt)
{
	const char *p = opt->operands[0];
	time_t t = time(NULL);

	lt = localtime(&t);
	set_default_format();

	if (p != NULL) {
		if (opt->operands[1] != NULL) {
			opt->error = "too many operands";
			return EXIT_FAILURE;
		}

		if (*p != '+') {
			opt->error = "format must start with a '+' character";
			return EXIT_FAILURE;
		}

		format = p + 1;
	}

	return write_date_time();
}
