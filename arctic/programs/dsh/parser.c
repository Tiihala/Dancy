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

struct command {
	char **argv;
	size_t state[2];
};

static void command_init(struct command *command)
{
	memset(command, 0, sizeof(*command));
	command->argv = NULL;
}

static void command_release(struct command *command)
{
	size_t i;

	if (!command->argv)
		return;

	for (i = 0; command->argv[i]; i++) {
		free(command->argv[i]);
		command->argv[i] = NULL;
	}

	free(command->argv);
	command_init(command);
}

static int parsing_error(const char *message)
{
	fprintf(stderr, "dsh: parsing error: %s\n", message);

	if (!dsh_interactive)
		dsh_operate_state = 0;

	return -1;
}

static char *append_arg(struct command *command, const char *arg)
{
	void *r;

	if (command->state[0] + 2 > command->state[1]) {
		const size_t add = 16;
		size_t size = (command->state[1] + add) * sizeof(char *);
		void *new_argv = malloc(size);

		if (new_argv == NULL)
			return NULL;

		size = command->state[1] * sizeof(char *);
		memcpy(new_argv, command->argv, size);

		command->argv = new_argv;
		command->state[1] += add;
	}

	if ((r = strdup(arg)) == NULL)
		return NULL;

	command->argv[command->state[0] + 0] = r;
	command->argv[command->state[0] + 1] = NULL;
	command->state[0] += 1;

	return r;
}

static int parse_input(struct token *token)
{
	struct command command;
	int i, argc = 0;

	command_init(&command);

	for (;;) {
		if (dsh_token_read(token))
			return command_release(&command), 1;

		if (token->type == token_type_null)
			break;

		if (token->type == token_type_arg) {
			if (!append_arg(&command, token->data))
				return command_release(&command), 1;
			argc += 1;
			continue;
		}

		if (token->type != token_type_op)
			return command_release(&command), 1;

		command_release(&command);

		if (token->data[0] == '<' || token->data[0] == '>') {
			fprintf(stderr, "DEBUG: [%ld]%s\n",
				token->value, token->data);
		}

		return parsing_error(token->data);
	}

	for (i = 0; argc > 0; i++) {
		const char *name = dsh_builtin_array[i].name;
		int (*cmd)(int, char **);

		if (name == NULL) {
			dsh_execute(command.argv);
			break;
		}

		if (!strcmp(name, command.argv[0])) {
			cmd = dsh_builtin_array[i].execute;
			dsh_exit_code = cmd(argc, command.argv);
			break;
		}
	}

	return command_release(&command), 0;
}

void dsh_parse_input(const char *input)
{
	struct token token;

	dsh_token_init(&token, input);
	parse_input(&token);
	dsh_token_reset(&token);
}
