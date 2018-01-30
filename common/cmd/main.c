/*
 * Copyright (c) 2018 Antti Tiihala
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
 * cmd/main.c
 *      Template program for command-line argument parsing
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [-a] [-b] [-m message]... [-o output] message...\n"
	"\nOptions:\n"
	"  -a            enable option a\n"
	"  -b            enable option b\n"
	"  -m message    add header message\n"
	"  -o output     output file\n"
	"\nGeneral:\n"
	"  --help        help text\n"
	"  --version     version information\n"
	"\n";

static void help(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	if (fmt) {
		fputs("Error: ", stderr);
		vfprintf(stderr, fmt, va);
		fputs("\n\n", stderr);
	}
	va_end(va);
	fputs(help_str, (fmt) ? stderr : stdout);
	exit((fmt) ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void version(void)
{
	fputs(PROGRAM_CMDNAME " " PROGRAM_VERSION "\n", stdout);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	static struct options opts;
	char **argv_i = (argc > 1) ? argv : NULL;

	while (argv_i && *++argv_i) {
		const char *arg = *argv_i;
		if (arg[0] != '-' || arg[1] == '\0')
			continue;
		*argv_i = NULL;
		if (arg[1] == '-') {
			if (arg[2] == '\0') {
				argv_i = &argv[argc];
				break;
			}
			if (!strcmp(arg + 2, "help"))
				help(NULL);
			if (!strcmp(arg + 2, "version"))
				version();
			help("unknown long option \"%s\"", arg);
		}
		do {
			const char **optarg = NULL;
			int (*optfnc)(struct options *, const char *) = NULL;

			switch (*++arg) {
			case '\0':
				arg = NULL;
				break;
			case 'a':
				opts.a = 1;
				break;
			case 'b':
				opts.b = 1;
				break;
			case 'm':
				optfnc = optfnc_m;
				break;
			case 'o':
				optarg = &opts.arg_o;
				break;
			default:
				help("unknown option \"-%c\"", *arg);
				break;
			}
			if (optarg || optfnc) {
				const char *next;
				next = (arg[1]) ? &arg[1] : *++argv_i;
				if (next) {
					if (optarg)
						*optarg = next;
					if (optfnc)
						if ((*optfnc)(&opts, next))
							help(opts.error);
					arg = *argv_i = NULL;
					break;
				}
				help("-%c <option-argument> missing", *arg);
			}
		} while (arg);
	}

	if (argv_i) {
		int i = argc = 1;
		while (argv + i < argv_i)
			if ((argv[argc] = argv[i++]) != NULL)
				argc++;
		argv[argc] = NULL;
	}

	opts.operands = (!argv[0]) ? &argv[0] : &argv[1];
	if (program(&opts)) {
		if (opts.error)
			help(opts.error);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
