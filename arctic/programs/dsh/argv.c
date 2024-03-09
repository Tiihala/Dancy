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
 * dsh/argv.c
 *      The Dancy Shell
 */

#include "main.h"

char **dsh_create_argv(char *buffer)
{
	size_t size = (strlen(buffer) * sizeof(char *)) + sizeof(char *);
	char **new_argv = malloc(size);
	size_t i = 0, j = 0;

	if (!new_argv)
		return NULL;

	while (buffer[i] != '\0') {
		size_t backslashes = 0;
		int quotation_mark = 0;
		char c, *p;

		while ((c = buffer[i]) == ' ' || c == '\t')
			buffer[i++] = '\0';

		if (buffer[i] == '\0')
			break;

		new_argv[j++] = p = &buffer[i];

		while ((c = buffer[i]) != '\0') {
			char prev_c = (char)((i > 0) ? buffer[i - 1] : '\0');
			char next_c = buffer[i + 1];

			if (!quotation_mark && (c == ' ' || c == '\t'))
				break;

			i += 1;

			if (prev_c == '\\')
				backslashes += 1;
			else
				backslashes = 0;

			if (backslashes > 0 && c == '"') {
				if ((backslashes & 1) == 0) {
					quotation_mark = !quotation_mark;
					c = '\\';
				}
				p -= (backslashes / 2);
				p[-1] = c;
				continue;
			}

			if (c == '"') {
				if (quotation_mark && next_c == '"')
					*p++ = c, i += 1;
				else
					quotation_mark = !quotation_mark;
				continue;
			}

			*p++ = c;
		}

		*p = (char)((p != &buffer[i]) ? '\0' : c);
	}

	new_argv[j] = NULL;

	return new_argv;
}
