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
	char op[TOKEN_DATA_SIZE];
	long value;
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

static void parsing_error(const char *message)
{
	fprintf(stderr, "dsh: parsing error: %s\n", message);

	if (!dsh_interactive)
		dsh_operate_state = 0;

	dsh_exit_code = 2;
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

static int parse_pipeline(struct command *commands, int count, int no_wait)
{
	struct dsh_execute_state state;
	int i;

	memset(&state, 0, sizeof(state));
	state.argv = commands->argv;
	state.no_wait = no_wait;

	if (state.no_wait) {
		fputs("dsh: warning: operator '&' is synchronous\n", stderr);
		state.no_wait = 0;
	}

	if (state.argv == NULL || count > 1) {
		parsing_error("redirections and pipes are not supported");
		fputs("dsh: parsing state:", stderr);

		for (i = 0; i < count; i++) {
			struct command *command = &commands[i];
			if (command->argv != NULL)
				fprintf(stderr, "\033[94m arg");
			else
				fprintf(stderr, "\033[95m %s", command->op);

		}
		fputs("\033[0m\n", stderr);
		return -1;
	}

	for (i = 0; dsh_builtin_array[i].name != NULL; i++) {
		int (*builtin_cmd)(int, char **);

		if (!strcmp(dsh_builtin_array[i].name, state.argv[0])) {
			int argc = 0;

			while (state.argv[argc] != NULL)
				argc += 1;

			builtin_cmd = dsh_builtin_array[i].execute;
			dsh_exit_code = builtin_cmd(argc, state.argv);
			return 0;
		}
	}

	posix_spawn_file_actions_init(&state.actions);
	posix_spawn_file_actions_addtcsetpgrp_np(&state.actions, 0);

	posix_spawnattr_init(&state.attr);
	posix_spawnattr_setflags(&state.attr,
		POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setpgroup(&state.attr, 0);
	posix_spawnattr_setsigmask(&state.attr, &dsh_sigmask);

	dsh_execute(&state);

	posix_spawn_file_actions_destroy(&state.actions);
	posix_spawnattr_destroy(&state.attr);

	return 0;
}

static int parse_list(struct command *commands, int count)
{
	const char *prev_op = "";
	struct command *p1, *p2;
	int i;

	p1 = p2 = commands;

	for (i = 0; i < count; i++) {
		const char *op = &p2->op[0];
		int accept_op = 0;
		int no_wait = 0;

		if (dsh_exit_code == 128 + SIGINT)
			break;

		if (!strcmp(op, "\\n"))
			accept_op = 1;
		else if (!strcmp(op, "&"))
			accept_op = 1, no_wait = 1;
		else if (!strcmp(op, "&&"))
			accept_op = 1;
		else if (!strcmp(op, ";"))
			accept_op = 1;
		else if (!strcmp(op, "||"))
			accept_op = 1;

		if (!accept_op) {
			p2 += 1;
			continue;
		}

		if ((int)(p2 - p1) == 0) {
			if (!strcmp(prev_op, "&") || !strcmp(prev_op, ";")) {
				if (!strcmp(op, "\\n"))
					return 0;
			}
			parsing_error(op);
			return -1;
		}

		if (!strcmp(prev_op, "&&") && dsh_exit_code != 0) {
			prev_op = op, p1 = ++p2;
			continue;
		}

		if (!strcmp(prev_op, "||") && dsh_exit_code == 0) {
			prev_op = op, p1 = ++p2;
			continue;
		}

		if (parse_pipeline(p1, (int)(p2 - p1), no_wait))
			return -1;

		if (no_wait)
			dsh_exit_code = 0;

		prev_op = op, p1 = ++p2;
	}

	return 0;
}

static void parse_input(struct token *token)
{
	struct command commands[128];
	int commands_count = 0;
	int commands_limit = (int)(sizeof(commands) / sizeof(commands[0]));
	int state = 0;

	for (;;) {
		struct command *command = &commands[commands_count];

		if (commands_count + 2 >= commands_limit) {
			parsing_error("input overflow"), state = -1;
			break;
		}

		if (dsh_token_read(token)) {
			state = -1;
			break;
		}

		if (token->type == token_type_null) {
			if (state == 1) {
				command = &commands[++commands_count];
				state = 0;
			}
			command_init(command);
			strcpy(&command->op[0], "\\n");
			commands_count += 1;
			break;
		}

		if (token->type == token_type_arg) {
			if (state == 0) {
				command_init(command);
				state = 1;
			}
			if (!append_arg(command, token->data)) {
				parsing_error("out of memory"), state = -1;
				break;
			}
			continue;
		}

		if (token->type == token_type_op) {
			if (state == 1) {
				command = &commands[++commands_count];
				state = 0;
			}
			command_init(command);
			strcpy(&command->op[0], &token->_data[0]);
			command->value = token->value;
			commands_count += 1;
			continue;
		}
	}

	if (!(state < 0) && commands_count > 1) {
		dsh_exit_code = 0;
		parse_list(&commands[0], commands_count);
	}

	while (commands_count > 0)
		command_release(&commands[--commands_count]);
}

void dsh_parse_input(const char *input)
{
	struct token token;

	dsh_token_init(&token, input);
	parse_input(&token);
	dsh_token_reset(&token);
}
