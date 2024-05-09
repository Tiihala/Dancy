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
 * dsh/token.c
 *      The Dancy Shell
 */

#include "main.h"

static int token_error(const char *message)
{
	fprintf(stderr, "dsh: error: %s\n", message);

	if (!dsh_interactive)
		dsh_operate_state = 0;

	return -1;
}

void dsh_token_init(struct token *token, const char *input)
{
	memset(token, 0, sizeof(*token));
	token->data = &token->_data[0];
	token->type = token_type_null;
	token->_input = input;
}

void dsh_token_reset(struct token *token)
{
	if (token->data && token->data != &token->_data[0])
		free(token->data);

	token->data = &token->_data[0];
	token->data[0] = '\0';
	token->type = token_type_null;
	token->value = 0;
}

int dsh_token_read(struct token *token)
{
	const char *input = token->_input;
	size_t new_i = token->_i;
	int size, empty_allowed = 0;
	int glob_mode = 0;
	int quote_mode = 0;
	char c;

	char _b[512];
	char *b = &_b[0];
	char *e = &_b[sizeof(_b) - 1];

	dsh_token_reset(token);
	*b = '\0';

	while ((c = input[new_i]) == ' ' || c == '\t')
		new_i += 1;

	token->_i = new_i;

	while ((c = input[new_i]) != '\0') {
		new_i += 1;

		if (b >= e)
			return token_error("buffer overflow");

		if (glob_mode && b + 1 >= e)
			return token_error("buffer overflow");

		if (quote_mode == '"') {
			if (c == '"') {
				quote_mode = 0;
				continue;
			}

			if (c == '\\') {
				char next_c = input[new_i];

				if (next_c == '\n') {
					new_i += 1;
					continue;

				} else if (next_c == '"' || next_c == '$') {
					c = next_c, new_i += 1;

				} else if (next_c == '\\' || next_c == '`') {
					c = next_c, new_i += 1;
				}
			}

			if (glob_mode == 1 && strchr("*?[", c))
				*b++ = '\\';

			*b++ = c, *b = '\0';
			continue;
		}

		if (quote_mode == '\'') {
			if (c == '\'') {
				quote_mode = 0;
				continue;
			}

			if (glob_mode == 1 && strchr("*?[", c))
				*b++ = '\\';

			*b++ = c, *b = '\0';
			continue;
		}

		if (c == '\\') {
			if (input[new_i] == '\0')
				break;

			c = input[new_i++];

			if (c == '\n')
				continue;

			if (glob_mode == 1 && strchr("*?[", c))
				*b++ = '\\';

			*b++ = c, *b = '\0';
			continue;
		}

		if (c == '"' || c == '\'') {
			quote_mode = (int)c;
			empty_allowed = 1;
			continue;
		}

		if (glob_mode == 0 && strchr("*?[", c)) {
			empty_allowed = 0;
			glob_mode = 1;
			new_i = token->_i;

			b = &_b[0], *b = '\0';
			continue;
		}

		if (glob_mode == 1 && c == '[') {
			glob_mode = 2;

			if (input[new_i] == ']') {
				*b++ = c;
				c = input[new_i++];
			}

			*b++ = c, *b = '\0';
			continue;
		}

		if (glob_mode == 2 && c == ']') {
			glob_mode = 1;

			*b++ = c, *b = '\0';
			continue;
		}

		if (strchr(" \t\n\v\f\r", c))
			break;

		if (strchr("&;<>|", c)) {
			long value = -1;

			size = (int)(b - &_b[0]);

			if (c == '<' || c == '>') {
				const char *p = &input[token->_i];

				while (p < &input[new_i - 1]) {
					if (*p < '0' || *p > '9') {
						value = -1;
						break;
					}

					if (value < 0)
						value = 0;

					value *= 10;
					value += (long)((int)*p - '0');

					if (value > 0xFFFF)
						value = 0xFFFF;

					p += 1;
				}
			}

			if (size > 0 && value < 0) {
				new_i -= 1;
				break;
			}

			if (c == '&') {
				strcpy(token->data, "&");
				if (input[new_i] == '&') {
					strcpy(token->data, "&&");
					new_i += 1;
				} else if (input[new_i] == '>') {
					strcpy(token->data, "&>");
					new_i += 1;
					if (input[new_i] == '>') {
						strcpy(token->data, "&>>");
						new_i += 1;
					}
				}
				token->type = token_type_op;
				return (token->_i = new_i), 0;
			}

			if (c == ';') {
				strcpy(token->data, ";");
				token->type = token_type_op;
				return (token->_i = new_i), 0;
			}

			if (c == '<') {
				strcpy(token->data, "<");
				if (input[new_i] == '<') {
					strcpy(token->data, "<<");
					new_i += 1;
				} else if (input[new_i] == '&') {
					strcpy(token->data, "<&");
					new_i += 1;
				} else if (input[new_i] == '>') {
					strcpy(token->data, "<>");
					new_i += 1;
				}
				token->type = token_type_op;
				token->value = value;
				return (token->_i = new_i), 0;
			}

			if (c == '>') {
				strcpy(token->data, ">");
				if (input[new_i] == '>') {
					strcpy(token->data, ">>");
					new_i += 1;
				} else if (input[new_i] == '&') {
					strcpy(token->data, ">&");
					new_i += 1;
				}
				token->type = token_type_op;
				token->value = value;
				return (token->_i = new_i), 0;
			}

			if (c == '|') {
				strcpy(token->data, "|");
				if (input[new_i] == '|') {
					strcpy(token->data, "||");
					new_i += 1;
				}
				token->type = token_type_op;
				return (token->_i = new_i), 0;
			}
		}

		*b++ = c, *b = '\0';
	}

	if (quote_mode == '"')
		return token_error("matching double-quote not found");

	if (quote_mode == '\'')
		return token_error("matching single-quote not found");

	token->_i = new_i;
	size = (int)(b - &_b[0]);

	if (!empty_allowed && size == 0)
		return 0;

	token->type = token_type_arg;

	if (glob_mode)
		token->type = token_type_glob;

	if (size < TOKEN_DATA_SIZE) {
		strcpy(token->data, &_b[0]);
	} else {
		token->data = strdup(&_b[0]);
		if (token->data == NULL)
			return token_error("out of memory");
	}

	return 0;
}
