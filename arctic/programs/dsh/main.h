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
};

struct dsh_builtin {
	int (*execute)(int argc, char *argv[]);
	const char *name;
};

extern struct dsh_builtin dsh_builtin_array[];

extern int dsh_exit_code;
extern int dsh_operate_state;

int operate(struct options *opt);
void dsh_execute(char **argv);

char **dsh_create_argv(char *buffer);
char *dsh_get_input(const char *prompt, size_t offset);

int dsh_save_termios(void);
void dsh_restore_termios(void);

#else
#error "MAIN_CMDNAME"
#endif
