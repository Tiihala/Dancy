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
 * dsh/main.h
 *      The Dancy Shell
 */

#ifndef MAIN_CMDNAME
#define MAIN_CMDNAME "dsh"

#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

struct options {
	char **operands;
	const char *error;

	const char *command_string;
	size_t command_string_i;

	FILE *input_stream;
	int input_stream_eof;
};

struct dsh_builtin {
	int (*execute)(int argc, char *argv[]);
	const char *name;
};

extern struct dsh_builtin dsh_builtin_array[];

extern int dsh_exit_code;
extern int dsh_operate_state;
extern int dsh_interactive;

extern pid_t dsh_tcpgrp;
extern sigset_t dsh_sigmask;

int operate(struct options *opt);

struct dsh_execute_state {
	pid_t pid;
	pid_t pgroup;
	char **argv;
	posix_spawn_file_actions_t actions;
	posix_spawnattr_t attr;
	int no_wait;
	int *pipe_fd;
};

void dsh_execute(struct dsh_execute_state *state);

char *dsh_get_input(struct options *opt, const char *prompt, size_t offset);
void dsh_parse_input(const char *input);

enum token_type {
	token_type_null,
	token_type_arg,
	token_type_glob,
	token_type_op,
	token_max_type
};

#define TOKEN_DATA_SIZE 8

struct token {
	char *data;
	char _data[TOKEN_DATA_SIZE];
	int type;
	long value;

	const char *_input;
	size_t _i;
};

void dsh_token_init(struct token *token, const char *input);
void dsh_token_reset(struct token *token);
int dsh_token_read(struct token *token);

#else
#error "MAIN_CMDNAME"
#endif
