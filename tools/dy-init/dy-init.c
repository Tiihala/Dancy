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
 * dy-init.c
 *      Program for processing Dancy init executables
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined (CHAR_BIT) || CHAR_BIT != 8
#error Definition of CHAR_BIT is not compatible
#endif

#if !defined (INT_MAX) || INT_MAX < 2147483647
#error Definition of INT_MAX is not compatible
#endif

extern const unsigned char ia32[416];
extern const unsigned char x64[416];

static unsigned long crc32c(const void *obj, size_t len)
{
	const unsigned char *ptr = (const unsigned char *)obj;
	unsigned long crc = 0xFFFFFFFFul;
	int i;

	while (len--) {
		crc ^= (unsigned long)*ptr++;
		for (i = 0; i < CHAR_BIT; i++) {
			if (crc & 1ul)
				crc >>= 1, crc ^= 0x82F63B78ul;
			else
				crc >>= 1;
		}
	}
	return ~crc & 0xFFFFFFFFul;
}

#define PROGRAM_CMDNAME "dy-init"

struct options {
	char **operands;
	const char *error;
	const char *arg_t;
	int legacy;
	int set_header;
	int verbose;
};

static int set_header(unsigned char type, unsigned char *buf, size_t size)
{
	static const unsigned char file_header[16] = {
		0x8Du, 0x41u, 0x54u, 0x0Du, 0x0Au, 0x73u, 0x74u, 0x64u,
		0x0Cu, 0x49u, 0x0Cu, 0x4Eu, 0x0Cu, 0x0Au, 0x71u, 0xF8u
	};
	static unsigned char zero_bytes[16];
	unsigned long crc;

	if (!memcmp(buf, file_header, sizeof(file_header)))
		; /* Accept */
	else if (!memcmp(buf, zero_bytes, sizeof(zero_bytes)))
		; /* Accept */
	else
		return 1;

	if (type == 0x32u) {
		size_t code_size = sizeof(ia32);
		size_t i;

		for (i = 0u; i < code_size; i++) {
			if ((unsigned)buf[i])
				break;
		}
		if (i == code_size)
			memcpy(&buf[0], &ia32[0], code_size);
	}
	if (type == 0x64u) {
		size_t code_size = sizeof(x64);
		size_t i;

		for (i = 0u; i < code_size; i++) {
			if ((unsigned)buf[i])
				break;
		}
		if (i == code_size)
			memcpy(&buf[0], &x64[0], code_size);
	}

	memcpy(&buf[0], &file_header[0], sizeof(file_header));
	memcpy(&buf[16], &zero_bytes[0], sizeof(zero_bytes));
	buf[16] = (unsigned char)((size) & 0xFFu);
	buf[17] = (unsigned char)((size >> 8) & 0xFFu);
	buf[28] = (unsigned char)(type & 0x7Fu);
	buf[30] = (unsigned char)((type & 0x80u) ? 0x01u : 0x00u);

	crc = crc32c(buf, size);
	buf[24] = (unsigned char)((crc) & 0xFFu);
	buf[25] = (unsigned char)((crc >> 8) & 0xFFu);
	buf[26] = (unsigned char)((crc >> 16) & 0xFFu);
	buf[27] = (unsigned char)((crc >> 24) & 0xFFu);
	return 0;
}

int program(struct options *opt)
{
	unsigned char type = 0x00u;

	if (!*opt->operands || *(opt->operands + 1))
		return opt->error = "missing/unsupported arguments", 1;

	if (opt->arg_t) {
		if (!strcmp(opt->arg_t, "ia16"))
			type = 0x16u;
		else if (!strcmp(opt->arg_t, "ia32"))
			type = 0x32u;
		else if (!strcmp(opt->arg_t, "x64"))
			type = 0x64u;
		else
			return fputs("Error: unknown type", stderr), 1;
		if (opt->legacy)
			type |= 0x80u;
	}

	if (opt->set_header) {
		static unsigned char buf[65536];

		size_t nbytes;
		FILE *fp;
		if (!type)
			return fputs("Error: type is not defined", stderr), 1;
		fp = (errno = 0, fopen(*opt->operands, "rb"));
		if (!fp)
			return perror("Error"), 1;
		nbytes = fread(&buf[0], 1u, sizeof(buf), fp);
		if (nbytes < 32u || !feof(fp) || ferror(fp)) {
			fputs("Error: can not read input\n", stderr);
			return (void)fclose(fp), 1;
		}
		if ((errno = 0, fclose(fp)))
			return perror("Error"), 1;

		/*
		 * Set the header and write the modified buffer into the file.
		 */
		if (set_header(type, &buf[0], nbytes)) {
			fputs("Error: unsupported file format\n", stderr);
			return 1;
		}
		fp = (errno = 0, fopen(*opt->operands, "wb"));
		if (!fp)
			return perror("Error"), 1;
		if (fwrite(&buf[0], 1u, nbytes, fp) != nbytes) {
			fputs("Error: can not write output\n", stderr);
			return (void)fclose(fp), 1;
		}
		if ((errno = 0, fclose(fp)))
			return perror("Error"), 1;
	} else {
		fputs("Warning: nothing is done\n", stderr);
		return 1;
	}
	return 0;
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [options] init-executable\n"
	"\nOptions:\n"
	"  -t type       init executable type\n"
	"                ia16, ia32, or x64\n"
	"  --legacy      set legacy flag\n"
	"  --set-header  write header and crc32c checksum\n"
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
			if (!strcmp(arg + 2, "legacy")) {
				opts.legacy = 1;
				continue;
			}
			if (!strcmp(arg + 2, "set-header")) {
				opts.set_header = 1;
				continue;
			}
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
