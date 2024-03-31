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
 * dsh/parser.c
 *      The Dancy Shell
 */

#include "main.h"

enum token_type {
	token_type_null,
	token_type_arg,
	token_max_type
};

#define FIXED_DATA_SIZE 8

struct token {
	void (*release)(struct token *token);
	char *data;
	char _data[FIXED_DATA_SIZE];
	int type;
};

struct state {
	struct token token;
	const char *input;
	size_t i;
};

static void release_default(struct token *token)
{
	token->data = &token->_data[0];
	*token->data = '\0';
	token->type = token_type_null;
}

static void release_malloc(struct token *token)
{
	token->release = release_default;
	free(token->data), release_default(token);
}

static int parsing_error(const char *message)
{
	fprintf(stderr, "dsh: parsing error: %s\n", message);
	if (!dsh_interactive)
		dsh_operate_state = 0;
	return 1;
}

static int read_token(struct state *state)
{
	const char *input = state->input;
	size_t new_i = state->i;
	int size, empty_allowed = 0;
	int quote_mode = 0;
	char c;

	char _b[512];
	char *b = &_b[0];
	char *e = &_b[sizeof(_b) - 1];

	state->token.release(&state->token);
	*b = '\0';

	while ((c = input[new_i]) == ' ' || c == '\t')
		new_i += 1;

	while ((c = input[new_i]) != '\0') {
		new_i += 1;

		if (b >= e)
			return parsing_error("buffer overflow");

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

			*b++ = c, *b = '\0';
			continue;
		}

		if (quote_mode == '\'') {
			if (c == '\'') {
				quote_mode = 0;
				continue;
			}

			*b++ = c, *b = '\0';
			continue;
		}

		if (c == '\\') {
			c = input[new_i++];

			if (c == '\n')
				continue;

			*b++ = c, *b = '\0';
			continue;
		}

		if (c == '"' || c == '\'') {
			quote_mode = (int)c;
			empty_allowed = 1;
			continue;
		}

		if (c == ' ' || c == '\t')
			break;

		*b++ = c, *b = '\0';
	}

	if (quote_mode == '"')
		return parsing_error("matching double-quote not found");

	if (quote_mode == '\'')
		return parsing_error("matching single-quote not found");

	state->i = new_i;
	size = (int)(b - &_b[0]);

	if (!empty_allowed && size == 0)
		return 0;

	state->token.type = token_type_arg;

	if (size < FIXED_DATA_SIZE) {
		strcpy(state->token.data, &_b[0]);
	} else {
		state->token.data = strdup(&_b[0]);
		state->token.release = release_malloc;

		if (state->token.data == NULL)
			return parsing_error("out of memory");
	}

	return 0;
}

static void parse_input(struct state *state)
{
	int i, argc = 0, status = 0;
	char *argv[4096];
	char *p;

	const int argc_limit = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

	for (;;) {
		int type;

		if (read_token(state)) {
			status = 1;
			break;
		}

		type = state->token.type;

		if (type == token_type_null)
			break;

		if (type == token_type_arg) {
			if (argc >= argc_limit) {
				parsing_error("argv overflow");
				status = 2;
				break;
			}

			if ((p = strdup(state->token.data)) == NULL) {
				parsing_error("out of memory");
				status = 3;
				break;
			}

			argv[argc++] = p;
		}
	}

	argv[argc] = NULL;

	for (i = 0; status == 0 && argc > 0; i++) {
		const char *name = dsh_builtin_array[i].name;
		int (*cmd)(int, char **);

		if (name == NULL) {
			dsh_execute(argv);
			break;
		}

		if (!strcmp(name, argv[0])) {
			cmd = dsh_builtin_array[i].execute;
			dsh_exit_code = cmd(argc, argv);
			break;
		}
	}

	for (i = 0; i < argc; i++)
		free(argv[i]);
}

void dsh_parse_input(const char *input)
{
	struct state state;

	memset(&state, 0, sizeof(state));
	state.token.release = release_default;
	state.input = input;

	parse_input(&state);
	state.token.release(&state.token);
}
