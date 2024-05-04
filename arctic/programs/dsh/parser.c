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
	char **_argv;
	size_t state[2];
	char op[TOKEN_DATA_SIZE];
	long value;
};

static void command_init(struct command *command)
{
	memset(command, 0, sizeof(*command));
	command->argv = NULL;
	command->_argv = NULL;
}

static void command_release(struct command *command)
{
	size_t i;

	if (!command->_argv)
		return;

	for (i = 0; command->_argv[i]; i++) {
		free(command->_argv[i]);
		command->_argv[i] = NULL;
	}

	free(command->_argv);
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

	if (command->argv != command->_argv)
		return NULL;

	if (command->state[0] + 2 > command->state[1]) {
		const size_t add = 16;
		size_t size = (command->state[1] + add) * sizeof(char *);
		void *new_argv = malloc(size);

		if (new_argv == NULL)
			return NULL;

		if (command->argv != NULL) {
			size = command->state[1] * sizeof(char *);
			memcpy(new_argv, command->argv, size);
			free(command->argv);
		}

		command->argv = new_argv;
		command->_argv = new_argv;
		command->state[1] += add;
	}

	if ((r = strdup(arg)) == NULL)
		return NULL;

	command->argv[command->state[0] + 0] = r;
	command->argv[command->state[0] + 1] = NULL;
	command->state[0] += 1;

	return r;
}

static int handle_builtin(char **argv, int fd_out, int fd_err)
{
	int i;

	for (i = 0; dsh_builtin_array[i].name != NULL; i++) {
		int (*builtin_cmd)(int, char **);

		if (!strcmp(dsh_builtin_array[i].name, argv[0])) {
			int current_fd_out = -1, current_fd_err = -1;
			int state = 0;

			if (fd_out >= 0 && state >= 0) {
				fflush(stdout);
				current_fd_out = dup(1);

				if (current_fd_out >= 0) {
					if (fd_out == INT_MAX)
						(void)close(1);
					else
						state = dup2(fd_out, 1);
				} else {
					state = -1;
				}
			}

			if (fd_err >= 0 && state >= 0) {
				fflush(stderr);
				current_fd_err = dup(2);

				if (current_fd_err >= 0) {
					if (fd_err == INT_MAX)
						(void)close(2);
					else
						state = dup2(fd_err, 2);
				} else {
					state = -1;
				}
			}

			if (state >= 0) {
				int argc = 0;

				while (argv[argc] != NULL)
					argc += 1;

				builtin_cmd = dsh_builtin_array[i].execute;
				dsh_exit_code = builtin_cmd(argc, argv);
			}

			if (current_fd_out >= 0) {
				fflush(stdout);
				dup2(current_fd_out, 1);
				close(current_fd_out);
			}

			if (current_fd_err >= 0) {
				fflush(stderr);
				dup2(current_fd_err, 2);
				close(current_fd_err);
			}

			if (state < 0) {
				fputs("dsh: error: "
					"builtin redirections\n", stderr);
				dsh_exit_code = 1;
			}

			return 0;
		}
	}

	return 1;
}

static int parse_pipeline_part(struct command *commands, int count,
	int pipe_fd_in, int pipe_fd_out, struct dsh_execute_state *state)
{
	int fd_out = pipe_fd_out, fd_err = -1;
	int e = 0;

	struct {
		int fd[2];
		int close_fd;
	} fd_array[16];

	size_t i, fd_array_i = 0;

	for (i = 0; i < sizeof(fd_array) / sizeof(fd_array[0]); i++) {
		fd_array[i].fd[0] = -1;
		fd_array[i].fd[1] = -1;
		fd_array[i].close_fd = -1;
	}

	for (i = 0; i < (size_t)count; i++) {
		struct command *command = &commands[i];
		char *op = &command->op[0];

		if (command->argv != NULL)
			continue;

		if (fd_array_i >= sizeof(fd_array) / sizeof(fd_array[0])) {
			fputs("dsh: too many operators\n", stderr);
			e = -1;
			break;
		}

		if (!strcmp(op, "<") || !strcmp(op, "<>")) {
			int flags, fd0, fd1;

			if (command->value >= 0)
				fd0 = (int)command->value;
			else
				fd0 = 0;

			command = &commands[++i];

			if (command->argv == NULL) {
				fprintf(stderr,
					"dsh: word missing after %s\n", op);
				e = -1;
				break;
			}

			if (!strcmp(op, "<"))
				flags = O_RDONLY;
			else
				flags = O_RDWR | O_CREAT;

			flags |= O_CLOEXEC;

			if ((fd1 = open(command->argv[0], flags)) < 0) {
				fprintf(stderr, "dsh: %s: %s\n",
					command->argv[0], strerror(errno));
				e = -1;
				break;
			}

			fd_array[fd_array_i].fd[0] = fd0;
			fd_array[fd_array_i].fd[1] = fd1;
			fd_array[fd_array_i].close_fd = fd1;

			switch (fd0) {
				case 1: fd_out = fd1; break;
				case 2: fd_err = fd1; break;
			}

			command->argv += 1;
			fd_array_i += 1;
			continue;
		}

		if (!strcmp(op, ">") || !strcmp(op, ">>")) {
			int flags, fd0, fd1;

			if (command->value >= 0)
				fd0 = (int)command->value;
			else
				fd0 = 1;

			command = &commands[++i];

			if (command->argv == NULL) {
				fprintf(stderr,
					"dsh: word missing after %s\n", op);
				e = -1;
				break;
			}

			if (!strcmp(op, ">"))
				flags = O_WRONLY | O_CREAT | O_TRUNC;
			else
				flags = O_WRONLY | O_CREAT | O_APPEND;

			flags |= O_CLOEXEC;

			if ((fd1 = open(command->argv[0], flags, 0666)) < 0) {
				fprintf(stderr, "dsh: %s: %s\n",
					command->argv[0], strerror(errno));
				e = -1;
				break;
			}

			fd_array[fd_array_i].fd[0] = fd0;
			fd_array[fd_array_i].fd[1] = fd1;
			fd_array[fd_array_i].close_fd = fd1;

			switch (fd0) {
				case 1: fd_out = fd1; break;
				case 2: fd_err = fd1; break;
			}

			command->argv += 1;
			fd_array_i += 1;
			continue;
		}

		if (!strcmp(op, "<&") || !strcmp(op, ">&")) {
			int fd0, fd1 = -1;
			int mode = O_RDWR;

			if (command->value >= 0)
				fd0 = (int)command->value;
			else
				fd0 = !strcmp(op, "<&") ? 0 : 1;

			command = &commands[++i];

			if (command->argv == NULL) {
				fprintf(stderr,
					"dsh: word missing after %s\n", op);
				e = -1;
				break;
			}

			if (!strcmp(command->argv[0], "-")) {
				fd1 = INT_MAX;
			} else {
				const char *p = command->argv[0];
				int value = -1;

				while (*p != '\0') {
					if (*p < '0' || *p > '9') {
						value = -1;
						break;
					}

					if (value < 0)
						value = 0;

					value *= 10;
					value += ((int)*p - '0');

					if (value > 0xFFFF)
						value = 0xFFFF;

					p += 1;
				}

				if (value >= 0)
					fd1 = value;
			}

			if (fd1 < 0) {
				fprintf(stderr,
					"dsh: number missing after %s\n", op);
				e = -1;
				break;
			}

			if (fd1 != INT_MAX) {
				int fl = fcntl(fd1, F_GETFL);

				if (fl < 0) {
					fprintf(stderr,
						"dsh: %d: %s\n",
						fd1, strerror(errno));
					e = -1;
					break;
				}

				mode = (fl & O_ACCMODE);
			}


			if (!strcmp(op, "<&")) {
				if (mode != O_RDWR && mode != O_RDONLY) {
					fprintf(stderr,
						"dsh: %d: %s\n",
						fd1, "not open for input");
					e = -1;
					break;
				}

			} else {
				if (mode != O_RDWR && mode != O_WRONLY) {
					fprintf(stderr,
						"dsh: %d: %s\n",
						fd1, "not open for output");
					e = -1;
					break;
				}
			}

			fd_array[fd_array_i].fd[0] = fd0;
			fd_array[fd_array_i].fd[1] = fd1;

			switch (fd0) {
				case 1: fd_out = fd1; break;
				case 2: fd_err = fd1; break;
			}

			command->argv += 1;
			fd_array_i += 1;
			continue;
		}

		fprintf(stderr, "dsh: operator \033[95m%s"
			"\033[0m is not supported\n", op);
		e = -1;
		break;
	}

	for (i = 0; i < (size_t)count && state->argv == NULL; i++) {
		struct command *command = &commands[i];
		char *op = &command->op[0];

		if (!strcmp(op, "\\n"))
			break;

		if (command->argv != NULL) {
			char **argv = command->argv;
			if (argv[0] != NULL)
				state->argv = argv;
		}
	}

	if (e != 0 || state->argv == NULL) {
		for (i = 0; i < fd_array_i; i++) {
			if (fd_array[i].close_fd >= 0)
				close(fd_array[i].close_fd);
		}
		if (e != 0)
			dsh_exit_code = 1;
		return e;
	}

	if (handle_builtin(state->argv, fd_out, fd_err) == 0) {
		for (i = 0; i < fd_array_i; i++) {
			if (fd_array[i].close_fd >= 0)
				close(fd_array[i].close_fd);
		}
		return 0;
	}

	posix_spawn_file_actions_init(&state->actions);

	if (dsh_tcpgrp && !state->no_wait)
		posix_spawn_file_actions_addtcsetpgrp_np(&state->actions, 0);

	if (pipe_fd_in >= 0) {
		int fd0 = 0;
		int fd1 = pipe_fd_in;

		posix_spawn_file_actions_adddup2(&state->actions, fd1, fd0);
	}

	if (pipe_fd_out >= 0) {
		int fd0 = 1;
		int fd1 = pipe_fd_out;

		posix_spawn_file_actions_adddup2(&state->actions, fd1, fd0);
		state->no_wait = 1;
	}

	for (i = 0; i < fd_array_i; i++) {
		int fd0 = fd_array[i].fd[0];
		int fd1 = fd_array[i].fd[1];

		if (fd0 == fd1)
			continue;

		if (fd1 == INT_MAX) {
			posix_spawn_file_actions_addclose(
				&state->actions, fd0);
			continue;
		}

		posix_spawn_file_actions_adddup2(&state->actions, fd1, fd0);
	}

	posix_spawnattr_init(&state->attr);
	posix_spawnattr_setflags(&state->attr,
		POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setpgroup(&state->attr, state->pgroup);
	posix_spawnattr_setsigmask(&state->attr, &dsh_sigmask);

	{
		int current_fd_err = -1;

		if (fd_err >= 0) {
			fflush(stderr);
			current_fd_err = fcntl(2, F_DUPFD_CLOEXEC, 0);

			if (current_fd_err >= 0) {
				if (fd_err == INT_MAX)
					(void)close(2);
				else
					e = dup2(fd_err, 2);
			} else {
				e = -1;
			}
		}

		if (e >= 0)
			dsh_execute(state);

		if (current_fd_err >= 0) {
			fflush(stderr);
			dup2(current_fd_err, 2);
			close(current_fd_err);
		}

		if (e < 0) {
			fputs("dsh: error: "
				"redirections\n", stderr);
			dsh_exit_code = 1;
		}
	}

	posix_spawn_file_actions_destroy(&state->actions);
	posix_spawnattr_destroy(&state->attr);

	for (i = 0; i < fd_array_i; i++) {
		if (fd_array[i].close_fd >= 0)
			close(fd_array[i].close_fd);
	}

	return 0;
}

static int parse_pipeline(struct command *commands, int count, int no_wait)
{
	pid_t pgroup = 0;

	struct {
		int fd[2];
		struct command *commands;
		int count;
	} fd_array[16];

	size_t i, fd_array_i = 0;
	int c, e = 0;

	fd_array[fd_array_i].fd[0] = -1;
	fd_array[fd_array_i].fd[1] = -1;
	fd_array[fd_array_i].commands = commands;
	fd_array[fd_array_i].count = count;

	fd_array_i += 1;

	for (i = 0; i < (size_t)count; i++) {
		struct command *command = &commands[i];
		char *op = &command->op[0];
		int flags = FD_CLOEXEC;

		if (command->argv != NULL || strcmp(op, "|"))
			continue;

		if (fd_array_i >= sizeof(fd_array) / sizeof(fd_array[0])) {
			fputs("dsh: too many pipe operators\n", stderr);
			e = -1;
			break;
		}

		if (pipe(fd_array[fd_array_i].fd) != 0) {
			perror("dsh: pipe");
			e = -1;
			break;
		}

		if (fcntl(fd_array[fd_array_i].fd[0], F_SETFD, flags) == -1)
			perror("dsh: fcntl"), e = -1;

		if (fcntl(fd_array[fd_array_i].fd[1], F_SETFD, flags) == -1)
			perror("dsh: fcntl"), e = -1;

		c = (int)(&commands[i] - fd_array[fd_array_i - 1].commands);
		fd_array[fd_array_i - 1].count = c;

		fd_array[fd_array_i].commands = &commands[i + 1];
		fd_array[fd_array_i].count = count - 1 - (int)i;

		fd_array_i += 1;
	}

	if (e != 0) {
		for (i = 1; i < fd_array_i; i++) {
			close(fd_array[i].fd[0]);
			close(fd_array[i].fd[1]);
		}
		dsh_exit_code = 1;
		return -1;
	}

	for (i = 0; i < fd_array_i; i++) {
		int pipe_fd_in = fd_array[i].fd[0];
		int pipe_fd_out = -1;

		struct dsh_execute_state state;

		memset(&state, 0x00, sizeof(state));

		state.pgroup = pgroup;
		state.argv = NULL;
		state.no_wait = no_wait;
		state.pipe_fd = &fd_array[i].fd[0];

		if (i + 1 < fd_array_i)
			pipe_fd_out = fd_array[i + 1].fd[1];

		e = parse_pipeline_part(
			fd_array[i].commands, fd_array[i].count,
			pipe_fd_in, pipe_fd_out, &state);

		if (e != 0 || dsh_operate_state == 0)
			break;

		if (pgroup == 0)
			pgroup = state.pid;
	}

	for (i = 1; i < fd_array_i; i++) {
		if (fd_array[i].fd[0] >= 0)
			close(fd_array[i].fd[0]);
		if (fd_array[i].fd[1] >= 0)
			close(fd_array[i].fd[1]);
	}

	return e;
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

		if (dsh_operate_state == 0)
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

		if (dsh_exit_code == 128 + SIGINT)
			break;

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

	if (!(state < 0) && commands_count > 1)
		parse_list(&commands[0], commands_count);

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
