/*
 * Copyright (c) 2023 Antti Tiihala
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
 * terminal/main.c
 *      The terminal program
 */

#include "main.h"

static const char *help_str =
	"Usage: " MAIN_CMDNAME
	" [options] executable [args...]\n"
	"\nOptions:\n"
	"  --1           console 1 (default)\n"
	"  --2           console 2\n"
	"  --3           console 3\n"
	"  --4           console 4\n"
	"  --5           console 5\n"
	"  --6           console 6\n"
	"\nGeneral:\n"
	"  --help, -h    help text\n"
	"  --verbose, -v additional information\n"
	"  --version, -V version information\n"
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
#ifdef MAIN_VERSION
	fputs(MAIN_VERSION "\n", stdout);
#else
	fputs(MAIN_CMDNAME "\n", stdout);
#endif
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	static struct options opts;
	char **argv_i = (argc > 1) ? argv : NULL;

	while (argv_i && *++argv_i) {
		const char *arg = *argv_i;

		/*
		 * In this program, the first operand
		 * prevents interpreting other options.
		 */
		if (arg[0] != '-') {
			argv_i = &argv[argc];
			break;
		}

		*argv_i = NULL;

		if (arg[1] == '-') {
			if (arg[2] == '\0') {
				argv_i = &argv[argc];
				break;
			}
			if (arg[2] == '1' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-1";
				continue;
			}
			if (arg[2] == '2' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-2";
				continue;
			}
			if (arg[2] == '3' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-3";
				continue;
			}
			if (arg[2] == '4' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-4";
				continue;
			}
			if (arg[2] == '5' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-5";
				continue;
			}
			if (arg[2] == '6' && arg[3] == '\0') {
				opts.keyboard = "/dev/dancy-keyboard-6";
				continue;
			}
			if (!strcmp(arg + 2, "help"))
				help(NULL);
			if (!strcmp(arg + 2, "version"))
				version();
			if (!strcmp(arg + 2, "verbose")) {
				opts.verbose = 1;
				continue;
			}
			help("unknown long option \"%s\"", arg);
		}

		do {
			const char **optional_arg = NULL;

			switch (*++arg) {
			case '\0':
				arg = NULL;
				break;
			case 'h':
				help(NULL);
				break;
			case 'v':
				opts.verbose = 1;
				break;
			case 'V':
				version();
				break;
			default:
				help("unknown option \"-%c\"", *arg);
				break;
			}
			if (optional_arg) {
				const char *next;
				next = (arg[1]) ? &arg[1] : *++argv_i;
				if (next) {
					if (optional_arg)
						*optional_arg = next;
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

	if (operate(&opts)) {
		if (opts.error)
			help(opts.error);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
