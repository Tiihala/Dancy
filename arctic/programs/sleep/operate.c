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
 * sleep/operate.c
 *      Suspend execution for an interval
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

static long long parse_time_to_milliseconds(const char *input)
{
	const char *p = input;
	char *endptr = NULL;
	long long t;

	while (*p == '\t' || *p == ' ')
		p += 1;

	if (*p < '0' || *p > '9') {
		fprintf(stderr, MAIN_CMDNAME
			": unknown interval \'%s\'\n", input);
		return -1;
	}

	t = (long long)(errno = 0, strtol(p, &endptr, 0));

	if (errno == ERANGE || t < 0 || t > 0x7FFFFFFF) {
		fprintf(stderr, MAIN_CMDNAME
			": interval range error \'%s\'\n", input);
		return -1;
	}

	p = endptr;

	while (*p == '\t' || *p == ' ')
		p += 1;

	if (*p == '\0' || !strcmp(p, "s"))
		return (t * 1000LL);

	if (!strcmp(p, "ms"))
		return (t * 1LL);

	if (!strcmp(p, "m"))
		return (t * 1000LL * 60LL);

	if (!strcmp(p, "h"))
		return (t * 1000LL * 60LL * 60LL);

	if (!strcmp(p, "d"))
		return (t * 1000LL * 60LL * 60LL * 24LL);

	fprintf(stderr, MAIN_CMDNAME
		": unknown interval format \'%s\'\n", input);

	return -1;
}

int operate(struct options *opt)
{
	unsigned long long total_ms = 0;
	long long ms, tv_sec_ll;
	long tv_nsec;
	int i;

	if (opt->operands[0] == NULL) {
		opt->error = "time must be specified";
		return EXIT_FAILURE;
	}

	for (i = 0; opt->operands[i] != NULL; i++) {
		const char *input = opt->operands[i];

		ms = parse_time_to_milliseconds(input);

		if (ms < 0)
			return EXIT_FAILURE;

		if (total_ms > ULLONG_MAX - (unsigned long long)ms) {
			total_ms = ULLONG_MAX;
			break;
		}

		total_ms += (unsigned long long)ms;
	}

	{
		volatile unsigned long long *p = &total_ms;

		tv_sec_ll = (long long)(*p / 1000ULL);
		tv_nsec = (long)(*p % 1000ULL) * 1000L * 1000L;
	}

	if (tv_sec_ll > 0x7FFFFFFF) {
		fputs(MAIN_CMDNAME ": interval range error\n", stderr);
		return EXIT_FAILURE;
	}

	return call_sleep((time_t)tv_sec_ll, tv_nsec);
}
