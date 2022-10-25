/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * dy-link/main.c
 *      Linker for Dancy operating system
 */

#include "program.h"

#define OFILES_CHUNK 16384

static int nr_ofiles;
static struct file ofiles[OFILES_LIMIT];

static int read_ofile(const char *name, int i)
{
	unsigned char *ptr;
	FILE *fp = stdin;
	int is_stdin = 1;

	if (ofiles[i].data || i >= OFILES_LIMIT)
		return fputs("Error: too many input files\n", stderr), 1;
	if (strcmp(name, "-")) {
		fp = (errno = 0, fopen(name, "rb"));
		if (!fp) {
			const char *fmt = "Error: input \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return 1;
		}
		is_stdin = 0;
	}
	ptr = (unsigned char *)malloc((size_t)(OFILES_CHUNK));

	for (;;) {
		int size;
		int my_errno;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			ofiles[i].size = -1;
			break;
		}
		ofiles[i].data = ptr;
		ptr = ofiles[i].data + ofiles[i].size;

		errno = 0;
		size = (int)fread(ptr, 1, (size_t)OFILES_CHUNK, fp);
		my_errno = errno;
		if (ferror(fp)) {
			fprintf(stderr, "Error: %s\n", strerror(my_errno));
			ofiles[i].size = -1;
			break;
		}

		my_errno = 2;
		if (ofiles[i].size < (INT_MAX - size)) {
			ofiles[i].size += size;
			if (feof(fp))
				break;
			my_errno--;
		}
		if (ofiles[i].size < (INT_MAX - OFILES_CHUNK)) {
			size = ofiles[i].size + OFILES_CHUNK;
			my_errno--;
		}
		if (my_errno) {
			fprintf(stderr, "Error: %s is too big\n", name);
			ofiles[i].size = -1;
			break;
		}
		ptr = (unsigned char *)realloc(ofiles[i].data, (size_t)size);
	}

	if (!is_stdin && (errno = 0, fclose(fp)))
		return perror("Error"), 1;
	return (ofiles[i].size < 0);
}

static void free_ofiles(void)
{
	int i;
	for (i = 0; i < OFILES_LIMIT && ofiles[i].data != NULL; i++)
		free(ofiles[i].data);
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [-f format] [-o output] object-files...\n"
	"\nOptions:\n"
	"  -f format     output format\n"
	"  -o output     output file\n"
	"  --dump        object file information\n"
	"  --dump-ext    unresolved symbols\n"
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

	{
		size_t null_test = sizeof(&argv[argc]);
		while (null_test) {
			if (*((unsigned char *)&argv[argc] + (--null_test)))
				return EXIT_FAILURE;
		}
	}

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
			if (!strcmp(arg + 2, "dump")) {
				opts.dump = 1;
				continue;
			}
			if (!strcmp(arg + 2, "dump-ext")) {
				opts.dump_ext = 1;
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
			case 'f':
				optarg = &opts.arg_f;
				break;
			case 'h':
				help(NULL);
				break;
			case 'o':
				optarg = &opts.arg_o;
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
	if (*opts.operands) {
		int i = 0;
		int total_size = 0;
		argv_i = opts.operands;
		if ((errno = 0, atexit(free_ofiles)))
			return perror("atexit error"), EXIT_FAILURE;
		while (*argv_i) {
			if (read_ofile(*argv_i, i))
				return EXIT_FAILURE;
			if (total_size < (INT_MAX - ofiles[i].size)) {
				total_size += ofiles[i].size;
			} else {
				fputs("Error: total size\n", stderr);
				return EXIT_FAILURE;
			}
			if (opts.verbose) {
				const char *fmt = "Read file %s, %i bytes\n";
				printf(fmt, *argv_i, ofiles[i].size);
			}
			argv_i++, i++, nr_ofiles++;
		}
	}
	opts.nr_ofiles = nr_ofiles;
	opts.ofiles = &ofiles[0];

	if (program(&opts)) {
		if (opts.error)
			help(opts.error);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
