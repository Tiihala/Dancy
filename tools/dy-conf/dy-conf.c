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
 * dy-conf.c
 *      Program for writing Dancy config files
 */

#include <ctype.h>
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

unsigned long crc32c(const void *obj, size_t len);

#define PROGRAM_CMDNAME "dy-conf"

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	int no_vbe_mode;
	int no_vbe_menu;
	int no_hlt;
	unsigned video_width;
	unsigned video_height;
	unsigned video_bits;
	unsigned long mem_base_low;
	unsigned long mem_base_high;
	unsigned long mem_size_low;
	unsigned long mem_size_high;
	int mem_set;
	int verbose;
};

static int write_file(struct options *opt, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!opt->arg_o)
		return 0;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return 1;
		}
		is_stdout = 0;
	}
	if ((errno = 0, fwrite(out, 1, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return 1;
	}

	if (is_stdout)
		return (errno = 0, fflush(fp)) ? perror("Error"), 1 : 0;
	return fclose(fp) ? perror("Error"), 1 : 0;
}

#define W_LE16(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_LE32(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

int create_config_file(struct options *opt)
{
	static const unsigned char file_header[16] = {
		0x8D, 0x41, 0x54, 0x0D, 0x0A, 0x73, 0x74, 0x64,
		0x0C, 0x43, 0x0C, 0x30, 0x0C, 0x0A, 0x71, 0xF8
	};
	unsigned flags = 0;
	unsigned long crc;
	unsigned char out[128];

	memcpy(&out[0], &file_header[0], 16);
	memset(&out[16], 0, 128 - 16);
	out[16] = 128;

	/*
	 * Flags at offset 0x20
	 *
	 * Bit 0 = no VBE modes
	 *     1 = no VBE menu
	 *     2 = no HLT instruction
	 */
	flags |= (opt->no_vbe_mode ? 1 : 0);
	flags |= (opt->no_vbe_menu ? 2 : 0);
	flags |= (opt->no_hlt ? 4 : 0);
	out[0x20] = (unsigned char)flags;

	/*
	 * Preferred video mode
	 *
	 * Off 0x24 = width
	 *     0x26 = (height << 4) | (bits_per_pixel >> 2)
	 */
	W_LE16(&out[0x24], opt->video_width);
	W_LE16(&out[0x26], (opt->video_height << 4) | (opt->video_bits >> 2));

	/*
	 * Reserved memory area
	 *
	 * Off 0x28 = base
	 *     0x30 = size
	 */
	W_LE32(&out[0x28], opt->mem_base_low);
	W_LE32(&out[0x2C], opt->mem_base_high);
	W_LE32(&out[0x30], opt->mem_size_low);
	W_LE32(&out[0x34], opt->mem_size_high);

	crc = crc32c(&out[0], 128), W_LE32(&out[0x18], crc);
	return write_file(opt, &out[0], 128);
}

int program(struct options *opt)
{
	int i, j;

	for (i = 0; opt->operands[i] != NULL; i++) {
		const char *op = opt->operands[i];

		/*
		 * Fixed format: 0x0000000000000000,0x0000000000000000
		 */
		if (strchr(op, ',')) {
			const char *e = "Error: wrong base,size format\n";
			char buf1[17], buf2[17];

			if (strlen(op) != 37)
				return fputs(e, stderr), 1;
			if (strncmp(op, "0x", 2))
				return fputs(e, stderr), 1;
			if (strncmp(op + 18, ",0x", 3))
				return fputs(e, stderr), 1;

			memcpy(&buf1[0], &op[2],  16), buf1[16] = '\0';
			memcpy(&buf2[0], &op[21], 16), buf2[16] = '\0';

			for (j = 0; j < 16; j++) {
				int c1 = (int)buf1[j];
				int c2 = (int)buf2[j];
				if (!isxdigit(c1) || !isxdigit(c2))
					return fputs(e, stderr), 1;
			}

			opt->mem_base_low = strtoul(&buf1[8], NULL, 16);
			opt->mem_size_low = strtoul(&buf2[8], NULL, 16);
			buf1[8] = '\0', buf2[8] = '\0';
			opt->mem_base_high = strtoul(&buf1[0], NULL, 16);
			opt->mem_size_high = strtoul(&buf2[0], NULL, 16);
			continue;
		}

		/*
		 * Format: 800x600x16
		 */
		if (isdigit((int)op[0])) {
			const char *e = "Error: wrong video mode format\n";
			const char *ptr = &op[0];

			opt->video_width = 0;
			while (*ptr != '\0' && *ptr != 'x') {
				if (!(*ptr >= '0' && *ptr <= '9'))
					return fputs(e, stderr), 1;
				opt->video_width *= 10;
				opt->video_width += (unsigned)(*ptr - '0');
				if (opt->video_width > 4096)
					return fputs(e, stderr), 1;
				ptr++;
			}
			if (*ptr != 'x')
				return fputs(e, stderr), 1;
			ptr++;

			opt->video_height = 0;
			while (*ptr != '\0' && *ptr != 'x') {
				if (!(*ptr >= '0' && *ptr <= '9'))
					return fputs(e, stderr), 1;
				opt->video_height *= 10;
				opt->video_height += (unsigned)(*ptr - '0');
				if (opt->video_height > 3072)
					return fputs(e, stderr), 1;
				ptr++;
			}
			if (*ptr != 'x')
				return fputs(e, stderr), 1;
			ptr++;

			opt->video_bits = 0;
			while (*ptr != '\0' && *ptr != 'x') {
				if (!(*ptr >= '0' && *ptr <= '9'))
					return fputs(e, stderr), 1;
				opt->video_bits *= 10;
				opt->video_bits += (unsigned)(*ptr - '0');
				if (opt->video_bits > 32)
					return fputs(e, stderr), 1;
				ptr++;
			}
			if (*ptr != '\0')
				return fputs(e, stderr), 1;

			if (!opt->video_width)
				return fputs(e, stderr), 1;
			if (!opt->video_height)
				return fputs(e, stderr), 1;
			if (!opt->video_bits)
				return fputs(e, stderr), 1;
			continue;
		}
		fprintf(stderr, "Error: unknown operand \"%s\"\n", op);
		return 1;
	}
	return create_config_file(opt);
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [-o output] [options] [<width>x<height>x<bits>] [<base>,<size>]\n"
	"\nOptions:\n"
	"  -o output     output file\n"
	"  --no-vbe-mode do not use VBE video modes\n"
	"  --no-vbe-menu do not display user interface\n"
	"  --no-hlt      do not use hlt instruction\n"
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
			if (!strcmp(arg + 2, "no-vbe-mode")) {
				opts.no_vbe_mode = 1;
				continue;
			}
			if (!strcmp(arg + 2, "no-vbe-menu")) {
				opts.no_vbe_menu = 1;
				continue;
			}
			if (!strcmp(arg + 2, "no-hlt")) {
				opts.no_hlt = 1;
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
