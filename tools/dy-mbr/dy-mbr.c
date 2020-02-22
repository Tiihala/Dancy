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
 * dy-mbr.c
 *      Program for installing Dancy master boot record
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

extern const unsigned char mbr_bin[512];

unsigned long crc32c(const void *, size_t);

#define PROGRAM_CMDNAME "dy-mbr"

struct options {
	char **operands;
	const char *error;
	int verbose;
};

int program(struct options *opt)
{
	unsigned char buf[512];
	FILE *fp;

	if (!*opt->operands || *(opt->operands + 1))
		return opt->error = "missing/unsupported arguments", 1;

	fp = (errno = 0, fopen(*opt->operands, "r+b"));
	if (!fp)
		return perror("Error"), 1;
	if (fread(&buf[0], 1, 512, fp) != 512) {
		fputs("Error: can not read 512 bytes\n", stderr);
		return (void)fclose(fp), 1;
	}
	if (fseek(fp, 0L, SEEK_SET)) {
		fputs("Error: fseek position\n", stderr);
		return (void)fclose(fp), 1;
	}

	/*
	 * Simple detection for master boot records.
	 */
	if (buf[0x0000] == 0xEB && buf[0x0001] == 0x3C) {
		fputs("Error: file system detected\n", stderr);
		return (void)fclose(fp), 1;
	}
	if (buf[0x01FE] != 0x55 || buf[0x01FF] != 0xAA) {
		fputs("Error: missing boot signature\n", stderr);
		return (void)fclose(fp), 1;
	}

	memcpy(&buf[0], &mbr_bin[0], 440);

	if ((errno = 0, fwrite(&buf[0], 1, 512, fp)) != 512)
		return perror("Error"), (void)fclose(fp), 1;
	if ((errno = 0, fclose(fp)))
		return perror("Error"), 1;

	if (opt->verbose) {
		unsigned long crc = crc32c(&mbr_bin[0], 512);
		printf("mbr_bin (crc32c): 0x%08lX\n", crc);
	}
	return 0;
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME " hdd-image\n"
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
