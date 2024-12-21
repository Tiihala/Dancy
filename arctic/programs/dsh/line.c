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
 * dsh/line.c
 *      The Dancy Shell
 */

#include "main.h"

static char *get_from_command_string(struct options *opt)
{
	size_t i = 0;
	size_t size = strlen(&opt->command_string[opt->command_string_i]) + 1;
	char *buffer;

	if (size == 1)
		return NULL;

	if ((buffer = malloc(size)) == NULL) {
		fputs("dsh: out of memory\n", stderr);
		return NULL;
	}

	for (;;) {
		char c = opt->command_string[opt->command_string_i];

		if (c == '\0')
			break;

		opt->command_string_i += 1;

		if (c == '\n')
			break;

		if (c == '\r')
			continue;

		buffer[i++] = c;
	}

	buffer[i] = '\0';

	return buffer;
}

static char *get_from_file(struct options *opt)
{
	size_t i = 0;
	size_t size = 0x2000;
	char *buffer;

	if (opt->input_stream_eof)
		return NULL;

	if ((buffer = malloc(size)) == NULL) {
		fputs("dsh: out of memory\n", stderr);
		return NULL;
	}

	for (;;) {
		int c = fgetc(opt->input_stream);

		if (c == EOF) {
			opt->input_stream_eof = 1;
			break;
		}

		if (c == '\n' || (char)c == '\0')
			break;

		if (c == '\r')
			continue;

		if (i + 2 > size) {
			fputs("dsh: line length error", stderr);
			return free(buffer), NULL;
		}

		buffer[i++] = (char)c;
	}

	buffer[i] = '\0';

	return buffer;
}

char *dsh_get_input(struct options *opt, const char *prompt, size_t offset)
{
	if (opt->command_string != NULL)
		return get_from_command_string(opt);

	if (opt->input_stream != NULL)
		return get_from_file(opt);

	return dsh_prompt_read(opt->prompt_state, prompt, (int)offset);
}
