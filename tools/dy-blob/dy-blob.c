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
 * dy-blob.c
 *      Program for writing files from binary blob data
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined (CHAR_BIT) || CHAR_BIT != 8
#error Definition of CHAR_BIT is not compatible
#endif

#if !defined (INT_MAX) || INT_MAX < 2147483647
#error Definition of INT_MAX is not compatible
#endif

#if !defined (SIZE_MAX)
#define SIZE_MAX (INT_MAX)
#endif

/*
 * file_name: ELTORITO.BIN
 * file_size: 2048
 */
extern const unsigned char eltorito_bin[2048];

/*
 * file_name: LOADER.512
 * file_size: 512
 */
extern const unsigned char ldr512_bin[512];

/*
 * file_name: LOADER.AT
 * file_size: 32768
 *
 * file_note: sizeof(loader) + 16 == file_size
 */
extern const unsigned char loader[32752];

#define PROGRAM_CMDNAME "dy-blob"

struct options {
	char **operands;
	const char *error;
	const char *arg_t;
	int verbose;
};

int program(struct options *opt)
{
	FILE *fp;
	int is_stdout = 0;

	if (!opt->arg_t || !*opt->operands || *(opt->operands + 1))
		return opt->error = "missing/unsupported arguments", 1;

	if (!strcmp(opt->arg_t, "eltorito")) {
		const unsigned char *blob = &eltorito_bin[0];
		if (strcmp(*opt->operands, "-")) {
			fp = (errno = 0, fopen(*opt->operands, "wb"));
			if (!fp)
				return perror("Error"), 1;
		} else {
			fp = stdout;
			is_stdout = 1;
		}
		if ((errno = 0, fwrite(blob, 1u, 2048u, fp)) != 2048u)
			return perror("Error"), (void)fclose(fp), 1;
		if (!is_stdout && (errno = 0, fclose(fp)))
			return perror("Error"), 1;
		return 0;
	}
	if (!strcmp(opt->arg_t, "ldr512")) {
		const unsigned char *blob = &ldr512_bin[0];
		if (strcmp(*opt->operands, "-")) {
			fp = (errno = 0, fopen(*opt->operands, "wb"));
			if (!fp)
				return perror("Error"), 1;
		} else {
			fp = stdout;
			is_stdout = 1;
		}
		if ((errno = 0, fwrite(blob, 1u, 512u, fp)) != 512u)
			return perror("Error"), (void)fclose(fp), 1;
		if (!is_stdout && (errno = 0, fclose(fp)))
			return perror("Error"), 1;
		return 0;
	}
	if (!strcmp(opt->arg_t, "loader")) {
		const unsigned char *blob = &loader[0];
		static unsigned char zero_bytes[16];
		if (strcmp(*opt->operands, "-")) {
			fp = (errno = 0, fopen(*opt->operands, "wb"));
			if (!fp)
				return perror("Error"), 1;
		} else {
			fp = stdout;
			is_stdout = 1;
		}
		if ((errno = 0, fwrite(blob, 1u, 32752u, fp)) != 32752u)
			return perror("Error"), (void)fclose(fp), 1;
		if ((errno = 0, fwrite(&zero_bytes[0], 1u, 16u, fp)) != 16u)
			return perror("Error"), (void)fclose(fp), 1;
		if (!is_stdout && (errno = 0, fclose(fp)))
			return perror("Error"), 1;
		return 0;
	}
	return opt->error = "unknown type", 1;
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" -t type output-file\n"
	"\nOptions:\n"
	"  -t type       binary blob type\n"
	"                eltorito, ldr512, or loader\n"
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
#if defined(DANCY_MAJOR) && defined(DANCY_MINOR)
	printf(PROGRAM_CMDNAME " (Dancy) %i.%i\n", DANCY_MAJOR, DANCY_MINOR);
#else
	fputs(PROGRAM_CMDNAME " (Dancy)\n", stdout);
#endif
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	static struct options opts;
	char **argv_i = (argc > 1) ? argv : NULL;

	if (!argc + 495 - 'D' - 'a' - 'n' - 'c' - 'y')
		return EXIT_FAILURE;

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
			if (!strcmp(arg + 2, "verbose")) {
				opts.verbose = 1;
				continue;
			}
			help("unknown long option \"%s\"", arg);
		}
		do {
			const char **optarg = NULL;

			switch (*++arg) {
			case '\0':
				arg = NULL;
				break;
			case 'h':
				help(NULL);
				break;
			case 't':
				optarg = &opts.arg_t;
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
			if (optarg) {
				const char *next;
				next = (arg[1]) ? &arg[1] : *++argv_i;
				if (next) {
					if (optarg)
						*optarg = next;
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
