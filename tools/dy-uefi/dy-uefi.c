/*
 * Copyright (c) 2019 Antti Tiihala
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
 * dy-uefi.c
 *      Program for creating UEFI executables
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

#define W_LE32(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

extern const unsigned char x64[8224];

#define PROGRAM_CMDNAME "dy-uefi"

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	const char *arg_t;
	int verbose;
};

#define EF_HEADER_SIZE 0x0040

static size_t uefi_file_size;
static unsigned char *uefi_file;

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	const size_t chunk = 0x100000;
	const size_t size_max = ~((size_t)0);
	unsigned char *ptr;
	FILE *fp;

	fp = (errno = 0, fopen(name, "rb"));
	if (!fp) {
		const char *fmt = "Error: reading file \"%s\" (%s)\n";
		fprintf(stderr, fmt, name, strerror(errno));
		return 1;
	}

	*size = 0;
	ptr = malloc(chunk);
	for (;;) {
		size_t bytes_read;
		int my_errno;
		int stop;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			return (void)fclose(fp), 1;
		}
		*out = ptr;
		ptr += *size;

		bytes_read = (errno = 0, fread(ptr, 1, chunk, fp));
		my_errno = errno;
		stop = feof(fp);

		if (ferror(fp) || (!stop && bytes_read != chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(my_errno));
			return (void)fclose(fp), 1;
		}

		*size += ((bytes_read <= chunk) ? bytes_read : chunk);
		if (stop)
			break;

		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return (void)fclose(fp), 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return (void)fclose(fp), 0;
}

static void free_file(void)
{
	if (uefi_file) {
		free(uefi_file);
		uefi_file_size = 0;
		uefi_file = NULL;
	}
}

static int check_uefi_file(void)
{
	static const char *format_err = "Error: uefi-object-file format\n";
	const size_t min_size = EF_HEADER_SIZE + 212;
	const size_t max_size = 0x00180000;

	if (uefi_file_size < min_size || uefi_file_size > max_size)
		return fputs("Error: uefi-object-file size\n", stderr), 1;
	if (LE16(&uefi_file[EF_HEADER_SIZE + 0]) != 0x8664)
		return fputs(format_err, stderr), 1;
	if (LE16(&uefi_file[EF_HEADER_SIZE + 2]) != 4)
		return fputs(format_err, stderr), 1;
	if (LE32(&uefi_file[EF_HEADER_SIZE + 16]) != 0)
		return fputs(format_err, stderr), 1;

	if (memcmp(&uefi_file[EF_HEADER_SIZE + 20], ".text", 6))
		return fputs(format_err, stderr), 1;
	if (memcmp(&uefi_file[EF_HEADER_SIZE + 60], ".rdata", 7))
		return fputs(format_err, stderr), 1;
	if (memcmp(&uefi_file[EF_HEADER_SIZE + 100], ".data", 6))
		return fputs(format_err, stderr), 1;
	if (memcmp(&uefi_file[EF_HEADER_SIZE + 140], ".bss", 5))
		return fputs(format_err, stderr), 1;
	return 0;
}

static int uefi(struct options *opt)
{
	unsigned char header[180];
	unsigned long patch1 = LE32(&x64[0x2000]) & 0x0FFFul;
	unsigned long patch2 = LE32(&x64[0x2004]) & 0x0FFFul;
	unsigned long patch3 = LE32(&x64[0x2008]) & 0x0FFFul;
	unsigned long patch4 = LE32(&x64[0x200C]) & 0x0FFFul;
	unsigned char *outbuf;
	size_t data_size, size;
	FILE *fp;

	memcpy(&header[0], &uefi_file[EF_HEADER_SIZE], sizeof(header));
	memset(&uefi_file[EF_HEADER_SIZE], 0, sizeof(header));
	memcpy(&uefi_file[0], &header[0], sizeof(header));

	data_size = uefi_file_size + 0x0FFF;
	data_size = (size_t)((unsigned long)data_size & ~(0x0FFFul));
	size = data_size + 0x2000;
	if ((outbuf = malloc(size)) == NULL)
		return fputs("Error: not enough memory\n", stderr), 1;

	memset(&outbuf[0], 0, size);
	memcpy(&outbuf[0], &x64[0], 0x2000);
	memcpy(&outbuf[0x2000], &uefi_file[0], uefi_file_size);

	/*
	 * dd patch1       ; offset of "size of initialized data"
	 * dd patch2       ; offset of "size of image"
	 * dd patch3       ; offset of "virtual size (data)"
	 * dd patch4       ; offset of "size of raw data (data)"
	 */
	W_LE32(&outbuf[patch1], data_size);
	W_LE32(&outbuf[patch2], size);
	W_LE32(&outbuf[patch3], data_size);
	W_LE32(&outbuf[patch4], data_size);

	fp = (errno = 0, fopen(opt->arg_o, "wb"));
	if (!fp) {
		const char *fmt = "Error: opening file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return free(outbuf), 1;
	}
	if ((errno = 0, fwrite(outbuf, 1, size, fp)) != size) {
		const char *fmt = "Error: writing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return free(outbuf), 1;
	}
	if ((errno = 0, fclose(fp))) {
		const char *fmt = "Error: closing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return free(outbuf), 1;
	}
	return free(outbuf), 0;
}

int program(struct options *opt)
{
	if (!*opt->operands || *(opt->operands + 1))
		return opt->error = "missing/unsupported arguments", 1;
	if (!opt->arg_o)
		return opt->error = "missing output file", 1;
	if (opt->arg_t && strcmp(opt->arg_t, "x64"))
		return opt->error = "unsupported object type", 1;
	if ((errno = 0, atexit(free_file)))
		return perror("atexit error"), 1;
	if (read_file(*opt->operands, &uefi_file, &uefi_file_size))
		return 1;
	if (check_uefi_file())
		return 1;
	return uefi(opt);
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [options] uefi-object-file\n"
	"\nOptions:\n"
	"  -o output     output file\n"
	"  -t type       uefi object type (only x64 supported)\n"
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
			case 'o':
				optarg = &opts.arg_o;
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
