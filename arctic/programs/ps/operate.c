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
 * ps/operate.c
 *      Write information about processes
 */

#include "main.h"

static pid_t *pid_array;
static size_t pid_array_count;

static int get_width(pid_t pid, int min)
{
	char b[32];
	int w = snprintf(&b[0], sizeof(b), "%lld", (long long)pid);
	return (w < min) ? min : w;
}

static int write_information(struct options *opt)
{
	int cmdline = 0;
	int width;
	size_t i, j;

	for (i = 0; opt->operands[i] != NULL; i++) {
		if (!strcmp(opt->operands[i], "a")) {
			cmdline = 1;

		} else {
			opt->error = "unknown operands";
			return EXIT_FAILURE;
		}
	}

	if (pid_array_count == 0) {
		fputs(MAIN_CMDNAME ": no processes\n", stderr);
		return EXIT_FAILURE;
	}

	width = get_width(pid_array[pid_array_count - 1], 4);

	printf("%*s %s\n", width, "PID", cmdline ? "COMMAND" : "CMD");

	for (i = 0; i < pid_array_count; i++) {
		pid_t pid = pid_array[i];
		const int request = __DANCY_PROCINFO_CMDLINE;

		uint8_t cmd[0x10000];
		ssize_t size;

		size = __dancy_procinfo(pid, request, &cmd[0], sizeof(cmd));

		if (size < 0) {
			perror(MAIN_CMDNAME ":__dancy_procinfo");
			return EXIT_FAILURE;
		}

		printf("%*lld ", width, (long long)pid_array[i]);

		for (j = 0; j < (size_t)size; j++) {
			int c = (int)cmd[j];

			if (c == 0) {
				if (!cmdline)
					break;
				printf(" ");
				continue;
			}

			if (c <= 0x20 || c >= 0x7F) {
				printf("\\x%02X", c);
				continue;
			}

			printf("%c", c);
		}

		printf("\n");
	}

	return 0;
}

int operate(struct options *opt)
{
	size_t array_size = 64 * sizeof(pid_t);
	int r;

	for (;;) {
		ssize_t size;

		if ((pid_array = malloc(array_size)) == NULL) {
			fputs(MAIN_CMDNAME ": out of memory\n", stderr);
			return EXIT_FAILURE;
		}

		if ((size = __dancy_proclist(pid_array, array_size)) >= 0) {
			pid_array_count = (size_t)size / sizeof(pid_t);
			break;
		}

		free(pid_array);

		if (errno != ENOMEM) {
			perror(MAIN_CMDNAME ":__dancy_proclist");
			return EXIT_FAILURE;
		}

		if ((array_size <<= 1) > 0x10000000) {
			fputs(MAIN_CMDNAME ": unexpected failure\n", stderr);
			return EXIT_FAILURE;
		}
	}

	r = write_information(opt);
	free(pid_array);

	return r;
}
