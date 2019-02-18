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
 * dy-gpt.c
 *      Program for installing Dancy master boot record (GPT)
 */

#include <ctype.h>
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

extern const unsigned char gpt_bin[512];

unsigned long crc32c(const void *, size_t);

#define PROGRAM_CMDNAME "dy-gpt"

struct options {
	char **operands;
	const char *error;
	const char *guid;
	int verbose;
};

static int hex_string_to_byte(const char *in, unsigned char *out)
{
	unsigned tmp = 0u;
	if (isxdigit((int)in[0]) && isxdigit((int)in[1])) {
		if (sscanf(&in[0], "%2x", &tmp) == 1)
			return (out[0] = (unsigned char)tmp), 0;
	}
	return (out[0] = 0x00u), 1;
}

int program(struct options *opt)
{
	unsigned char buf[512];
	unsigned char guid[16];
	FILE *fp;

	if (!*opt->operands || *(opt->operands + 1))
		return opt->error = "missing/unsupported arguments", 1;

	/*
	 * Check globally unique identifier. The format is exactly 36 chars,
	 * "aabbccdd-eeff-gghh-iijj-kkllmmnnoopp", and the pairs aa-pp are
	 * two characters in the range 0-9, a-f, or A-F.
	 */
	if (opt->guid) {
		int err = 0;
		if (strlen(opt->guid) != 36u)
			return fputs("Error: guid length\n", stderr), 1;

		err |= hex_string_to_byte(&opt->guid[0], &guid[3]);
		err |= hex_string_to_byte(&opt->guid[2], &guid[2]);
		err |= hex_string_to_byte(&opt->guid[4], &guid[1]);
		err |= hex_string_to_byte(&opt->guid[6], &guid[0]);

		err |= (opt->guid[8] == '-') ? 0 : 1;
		err |= hex_string_to_byte(&opt->guid[9], &guid[5]);
		err |= hex_string_to_byte(&opt->guid[11], &guid[4]);

		err |= (opt->guid[13] == '-') ? 0 : 1;
		err |= hex_string_to_byte(&opt->guid[14], &guid[7]);
		err |= hex_string_to_byte(&opt->guid[16], &guid[6]);

		err |= (opt->guid[18] == '-') ? 0 : 1;
		err |= hex_string_to_byte(&opt->guid[19], &guid[8]);
		err |= hex_string_to_byte(&opt->guid[21], &guid[9]);

		err |= (opt->guid[23] == '-') ? 0 : 1;
		err |= hex_string_to_byte(&opt->guid[24], &guid[10]);
		err |= hex_string_to_byte(&opt->guid[26], &guid[11]);
		err |= hex_string_to_byte(&opt->guid[28], &guid[12]);
		err |= hex_string_to_byte(&opt->guid[30], &guid[13]);
		err |= hex_string_to_byte(&opt->guid[32], &guid[14]);
		err |= hex_string_to_byte(&opt->guid[34], &guid[15]);

		if (err)
			return fputs("Error: guid format\n", stderr), 1;
	}

	fp = (errno = 0, fopen(*opt->operands, "r+b"));
	if (!fp)
		return perror("Error"), 1;
	if (fread(&buf[0], 1u, 512u, fp) != 512u) {
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
	if (buf[0x0000] == 0xEBu && buf[0x0001] == 0x3Cu) {
		fputs("Error: file system detected\n", stderr);
		return (void)fclose(fp), 1;
	}
	if (buf[0x01FE] != 0x55u || buf[0x01FF] != 0xAAu) {
		fputs("Error: missing boot signature\n", stderr);
		return (void)fclose(fp), 1;
	}

	memcpy(&buf[0], &gpt_bin[0], 440u);
	if (opt->guid)
		memcpy(&buf[16], &guid[0], 16u);

	if ((errno = 0, fwrite(&buf[0], 1u, 512u, fp)) != 512u)
		return perror("Error"), (void)fclose(fp), 1;
	if ((errno = 0, fclose(fp)))
		return perror("Error"), 1;

	if (opt->verbose) {
		unsigned long crc = crc32c(&gpt_bin[0], 512u);
		printf("gpt_bin (crc32c): 0x%08lX\n", crc);
	}
	return 0;
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [-g guid] hdd-image\n"
	"\nOptions:\n"
	"  -g guid       bootable UniquePartitionGUID\n"
	"                e.g. \"aabbccdd-eeff-gghh-iijj-kkllmmnnoopp\"\n"
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
			case 'g':
				optarg = &opts.guid;
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
