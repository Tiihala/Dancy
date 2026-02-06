/*
 * Copyright (c) 2024, 2026 Antti Tiihala
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

static int valid_variable_char(char c, int digit)
{
	if (c == '_' || (digit && (c >= '0' && c <= '9')))
		return 1;

	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return 1;

	return 0;
}

static int expand(const char *input, size_t *new_i, char **b, char *e, char c)
{
	char next_c = input[*new_i];
	int add_i = 0;

	int buffer_i = 0;
	char buffer[256];

	do {
		if (c == '$' && next_c == '?') {
			int r = snprintf(&buffer[0], sizeof(buffer),
				"%d", dsh_exit_code);

			if (r <= 0 || r >= (int)sizeof(buffer))
				return token_error("unexpected behavior");

			buffer_i = r, add_i = 1;
			break;
		}

		if (c == '$' && next_c == '$') {
			int r = snprintf(&buffer[0], sizeof(buffer),
				"%lld", (long long)getpid());

			if (r <= 0 || r >= (int)sizeof(buffer))
				return token_error("unexpected behavior");

			buffer_i = r, add_i = 1;
			break;
		}

		if (c == '$' && valid_variable_char(next_c, 0)) {
			const char *p;

			do {
				buffer[buffer_i++] = next_c;
				buffer[buffer_i] = '\0';

				next_c = input[*new_i + (size_t)buffer_i];

				if (buffer_i + 1 >= (int)sizeof(buffer))
					return token_error("variable name");

			} while (valid_variable_char(next_c, 1));

			add_i = buffer_i;

			if ((p = dsh_var_read(&buffer[0])) == NULL)
				p = "";

			buffer_i = (int)strlen(p);

			if (buffer_i + 1 >= (int)sizeof(buffer))
				return token_error("variable value");

			memcpy(&buffer[0], p, (size_t)buffer_i);
			break;
		}

		if (c == '$' && next_c == '{') {
			int n = -1;
			const char *p;

			for (;;) {
				next_c = input[*new_i + (size_t)(++add_i)];

				if (n == -1 && (n = (next_c == '#')) > 0)
					continue;

				if (next_c == '\0') {
					add_i = 0, buffer_i = 0;
					break;
				}

				if (next_c == '}') {
					add_i += 1;
					break;
				}

				if (!valid_variable_char(next_c, (add_i > 1)))
					return token_error("unknown ${...}");

				buffer[buffer_i++] = next_c;
				buffer[buffer_i] = '\0';

				if (buffer_i + 1 >= (int)sizeof(buffer))
					return token_error("variable name");
			}

			if (buffer_i == 0) {
				if (n > 0)
					strcpy(&buffer[0], "0"), buffer_i = 1;
				break;
			}

			if ((p = dsh_var_read(&buffer[0])) == NULL)
				p = "";

			buffer_i = (int)strlen(p);

			if (n > 0) {
				int r = snprintf(&buffer[0], sizeof(buffer),
					"%d", buffer_i);

				if (r > 0 && r < (int)sizeof(buffer)) {
					buffer_i = r;
					break;
				}

				return token_error("unexpected behavior");
			}

			if (buffer_i + 1 >= (int)sizeof(buffer))
				return token_error("variable value");

			memcpy(&buffer[0], p, (size_t)buffer_i);
			break;
		}

	} while (0);

	if (buffer_i > 0) {
		if (*b >= e || (size_t)(e - *b) < (size_t)buffer_i)
			return token_error("expand buffer overflow");

		memcpy(*b, &buffer[0], (size_t)buffer_i);
	}

	(*new_i) += (size_t)add_i;
	(*b) += buffer_i;
	(*b)[0] = '\0';

	if (add_i > 0 && buffer_i == 0)
		return INT_MAX;

	return add_i;
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

			if (c == '$') {
				int r = expand(input, &new_i, &b, e, c);

				if (r < 0)
					return r;
				if (r > 0)
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

		if (c == '$') {
			int r = expand(input, &new_i, &b, e, c);

			if (r == INT_MAX)
				token->type = token_max_type;

			if (r < 0)
				return r;
			if (r > 0)
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
