/*
 * Copyright (c) 2023, 2024 Antti Tiihala
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
 * dsh/operate.c
 *      The Dancy Shell
 */

#include "main.h"

int dsh_exit_code = 0;
int dsh_operate_state = 1;
int dsh_interactive = 1;

pid_t dsh_tcpgrp = 0;
sigset_t dsh_sigmask;

static void welcome(void)
{
	fputs("\tWelcome to the Dancy Operating System!\n\n", stdout);
}

static void create_safe_cwd(void *out, const void *in)
{
	unsigned char *o = out;
	const unsigned char *i = in;

	for (;;) {
		unsigned int c = *i++;
		char b[8];

		*o++ = (unsigned char)c;

		if (c == 0)
			break;

		if (c <= 0x20 || c >= 0x7F) {
			if (snprintf(&b[0], sizeof(b), "\\x%02X", c) != 4)
				memcpy(&b[0], "\\x00", 4);

			memcpy(&o[-1], &b[0], 4), o += 3;
		}
	}
}

int operate(struct options *opt)
{
	pid_t pgrp = getpgrp();
	int r;

	sigset_t set;
	sigemptyset(&set);

	if (opt->operands[0] != NULL && opt->command_string == NULL) {
		FILE *stream = (errno = 0, fopen(opt->operands[0], "rb"));

		if (stream == NULL) {
			fprintf(stderr, "dsh: %s: %s\n",
				opt->operands[0], strerror(errno));
			return EXIT_FAILURE;
		}

		opt->input_stream = stream;
	}

	if (opt->command_string != NULL || opt->input_stream != NULL)
		dsh_interactive = 0;

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	if (tcgetpgrp(0) == pgrp && pgrp > 0) {
		dsh_tcpgrp = pgrp;
		sigaddset(&set, SIGTTIN);
		sigaddset(&set, SIGTTOU);
	}

	if (dsh_interactive) {
		sigaddset(&set, SIGINT);
		sigaddset(&set, SIGQUIT);
	}

	if ((errno = 0, sigprocmask(SIG_BLOCK, &set, &dsh_sigmask)) == -1) {
		fprintf(stderr, "dsh: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if (dsh_interactive)
		welcome();

	while (dsh_operate_state != 0) {
		char prompt[2048];
		char cwd[2048], raw_cwd[256];
		const size_t cwdmax = 32;
		size_t offset;
		char *buffer;

		if (getcwd(&raw_cwd[0], sizeof(raw_cwd)) == NULL)
			raw_cwd[0] = '\0';

		create_safe_cwd(&cwd[0], &raw_cwd[0]);

		if ((offset = strlen(&cwd[0])) > cwdmax) {
			offset = offset - cwdmax + 3;
			memmove(&cwd[0], &cwd[offset], cwdmax - 2);
			offset = cwdmax;

			r = snprintf(&prompt[0], sizeof(prompt),
				"\r\033[2K\033[1;32m[\033[1;90m..."
				"\033[1;32m%s]$\033[0m ", &cwd[0]);
		} else {
			r = snprintf(&prompt[0], sizeof(prompt),
				"\r\033[2K\033[1;32m[%s]$\033[0m ", &cwd[0]);
		}

		offset += 4;

		if (r <= 0) {
			dsh_exit_code = EXIT_FAILURE;
			break;
		}

		if ((buffer = dsh_get_input(opt, &prompt[0], offset)) == NULL)
			break;

		if (dsh_interactive)
			fprintf(stdout, "\n");

		dsh_parse_input(buffer);
		free(buffer);
	}

	if (opt->input_stream != NULL)
		fclose(opt->input_stream);

	if (dsh_exit_code != 0)
		exit(dsh_exit_code);

	return 0;
}
