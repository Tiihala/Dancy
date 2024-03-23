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
	int i, r;

	if (opt->operands[0] != NULL) {
		opt->error = "operands are not allowed";
		return EXIT_FAILURE;
	}

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	welcome();

	while (dsh_operate_state != 0) {
		char *buffer, **new_argv;
		char prompt[2048];
		char cwd[2048], raw_cwd[256];
		const size_t cwdmax = 32;
		size_t offset;

		if ((errno = 0, getcwd(&raw_cwd[0], sizeof(raw_cwd))) == NULL)
			return perror("getcwd"), EXIT_FAILURE;

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

		if (r <= 0)
			return EXIT_FAILURE;

		if ((buffer = dsh_get_input(&prompt[0], offset)) == NULL)
			break;

		if ((new_argv = dsh_create_argv(buffer)) == NULL) {
			free(buffer);
			break;
		}

		fprintf(stdout, "\n");

		if (new_argv[0] != NULL && new_argv[0][0] != '\0') {
			int argc = 1;

			while (new_argv[argc] != NULL)
				argc += 1;

			for (i = 0; /* void */; i++) {
				const char *name = dsh_builtin_array[i].name;
				int (*cmd)(int argc, char *argv[]);

				if (name == NULL) {
					dsh_execute(new_argv);
					break;
				}

				if (!strcmp(name, new_argv[0])) {
					cmd = dsh_builtin_array[i].execute;
					dsh_exit_code = cmd(argc, new_argv);
					break;
				}
			}
		}

		free(new_argv);
		free(buffer);
	}

	return 0;
}
