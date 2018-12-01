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
 * dy-vbr.c
 *      Program for installing Dancy volume boot records
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const unsigned char floppy_bin[512];
extern const unsigned char ldr512_bin[512];
extern const unsigned char vbrchs_bin[512];
extern const unsigned char vbrlba_bin[512];

unsigned long crc32c(const void *, size_t);

#define PROGRAM_CMDNAME "dy-vbr"

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	const char *arg_t;
	long blocks;
	int sector_size;
	int verbose;
};

struct image_type {
	int blocks;
	int sectors;
	int sector_size;
	int fat_type;
	unsigned char data[16];
	unsigned char data_fat[4];
};

static struct image_type image_types[] = {
	{ 720, 1440, 512, 12,
		/* data[16] */
		{
			0x00, 0x02, 0x02, 0x01, 0x00, 0x02, 0x70, 0x00,
			0xA0, 0x05, 0xF9, 0x03, 0x00, 0x09, 0x00, 0x02
		},
		/* data_fat[4] */
		{
			0xF9, 0xFF, 0xFF, 0x00
		}
	},
	{ 1440, 2880, 512, 12,
		/* data[16] */
		{
			0x00, 0x02, 0x01, 0x01, 0x00, 0x02, 0xE0, 0x00,
			0x40, 0x0B, 0xF0, 0x09, 0x00, 0x12, 0x00, 0x02
		},
		/* data_fat[4] */
		{
			0xF0, 0xFF, 0xFF, 0x00
		}
	}
};

static int create(struct options *opt)
{
	static unsigned char buf[4096];
	struct image_type *type = NULL;
	int sectors_written = 0;
	FILE *fp;
	size_t size;
	size_t i;

	if (!opt->sector_size)
		opt->sector_size = 512;

	for (i = 0u; i < sizeof(image_types) / sizeof(image_types[0]); i++) {
		if ((long)image_types[i].blocks != opt->blocks)
			continue;
		if (image_types[i].sector_size != opt->sector_size)
			continue;
		type = &image_types[i];
		break;
	}
	if (!type) {
		fputs("Error: unsupported image size\n", stderr);
		return 1;
	}

	fp = (errno = 0, fopen(*opt->operands, "wb"));
	if (!fp)
		return perror("Error"), 1;

	memset(&buf[0], 0, 4096u);
	buf[0] = 0xEB, buf[1] = 0x3C, buf[2] = 0x90;
	memcpy(&buf[3], "dancy.fs", 8u);
	memcpy(&buf[11], &type->data[0], 16u);
	if (opt->arg_t && strcmp(opt->arg_t, "floppy"))
		buf[36] = 0x80;
	buf[38] = 0x29;
	memcpy(&buf[43], "NO NAME    ", 11u);
	if (type->fat_type == 12)
		memcpy(&buf[54], "FAT12   ", 8u);
	else
		memcpy(&buf[54], "FAT16   ", 8u);
	buf[62] = 0xCD, buf[63] = 0x19;
	buf[64] = 0xEB, buf[65] = 0xFE;
	buf[510] = 0x55, buf[511] = 0xAA;

	size = (size_t)opt->sector_size;
	if ((errno = 0, fwrite(&buf[0], 1u, size, fp)) != size)
		return perror("Error"), (void)fclose(fp), 1;
	sectors_written++;

	memset(&buf[0], 0, 4096u);
	memcpy(&buf[0], &type->data_fat[0], 4u);
	if ((errno = 0, fwrite(&buf[0], 1u, size, fp)) != size)
		return perror("Error"), (void)fclose(fp), 1;
	sectors_written++;

	memset(&buf[0], 0, 4u);
	for (i = 0u; i < (size_t)type->data[11] - 1u; i++) {
		if ((errno = 0, fwrite(&buf[0], 1u, size, fp)) != size)
			return perror("Error"), (void)fclose(fp), 1;
		sectors_written++;
	}

	if (type->data[5] == 2u) {
		memcpy(&buf[0], &type->data_fat[0], 4u);
		if ((errno = 0, fwrite(&buf[0], 1u, size, fp)) != size)
			return perror("Error"), (void)fclose(fp), 1;
		sectors_written++;
	}

	memset(&buf[0], 0, 4u);
	while (sectors_written < type->sectors) {
		if ((errno = 0, fwrite(&buf[0], 1u, size, fp)) != size)
			return perror("Error"), (void)fclose(fp), 1;
		sectors_written++;
	}

	if ((errno = 0, fclose(fp)))
		return perror("Error"), 1;
	return 0;
}

int program(struct options *opt)
{
	const unsigned char *vbr = &vbrchs_bin[0];
	unsigned char buf[512];

	if (!opt->arg_o && !*opt->operands)
		return opt->error = "missing/unsupported arguments", 1;

	if (*(opt->operands + 1)) {
		opt->blocks = strtol(*(opt->operands + 1), NULL, 0);
		if (opt->blocks <= 0L || *(opt->operands + 2))
			return opt->error = "unsupported arguments", 1;
		if (create(opt))
			return 1;
	} else if (opt->sector_size) {
		fputs("Warning: sector size argument ignored\n", stderr);
	}

	if (opt->arg_t) {
		if (!strcmp(opt->arg_t, "floppy"))
			vbr = &floppy_bin[0];
		else if (!strcmp(opt->arg_t, "chs"))
			; /* Default */
		else if (!strcmp(opt->arg_t, "lba"))
			vbr = &vbrlba_bin[0];
		else
			return opt->error = "unknown record type argument", 1;
	}

	if (opt->arg_o) {
		FILE *fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp)
			return perror("Error"), 1;
		if ((errno = 0, fwrite(&ldr512_bin[0], 1u, 512u, fp)) != 512u)
			return perror("Error"), (void)fclose(fp), 1;
		if ((errno = 0, fclose(fp)))
			return perror("Error"), 1;
		if (opt->verbose) {
			unsigned long crc = crc32c(&ldr512_bin[0], 512u);
			printf("ldr512_bin (crc32c): 0x%08lX\n", crc);
		}
	}

	if (*opt->operands) {
		FILE *fp = (errno = 0, fopen(*opt->operands, "r+b"));
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
		 * Simple detection for "standard" FAT12 / FAT16 file systems.
		 */
		if (buf[0x0000] != 0xEBu || buf[0x0001] != 0x3Cu) {
			fputs("Error: unknown file system\n", stderr);
			return (void)fclose(fp), 1;
		}
		if (buf[0x0022] != 0x00u || buf[0x0023] != 0x00u) {
			fputs("Error: unsupported partition size\n", stderr);
			return (void)fclose(fp), 1;
		}
		if (buf[0x01FE] != 0x55u || buf[0x01FF] != 0xAAu) {
			fputs("Error: missing boot signature\n", stderr);
			return (void)fclose(fp), 1;
		}

		memcpy(&buf[0x0000], &vbr[0x0000], 3u);
		memcpy(&buf[0x003E], &vbr[0x003E], 450u);

		if ((errno = 0, fwrite(&buf[0], 1u, 512u, fp)) != 512u)
			return perror("Error"), (void)fclose(fp), 1;
		if ((errno = 0, fclose(fp)))
			return perror("Error"), 1;

		if (opt->verbose) {
			unsigned long crc = crc32c(&vbr[0], 512u);
			printf("vbr (crc32c): 0x%08lX\n", crc);
		}
	}
	return 0;
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" [-o file] [-t type] file-system-image [1024-byte-blocks]\n"
	"\nOptions:\n"
	"  -o file       write loader.512 file\n"
	"  -t type       volume boot record type\n"
	"                floppy, chs (default), or lba\n"
	"  --512         512-byte sectors (default)\n"
	"  --1024        1024-byte sectors\n"
	"  --2048        2048-byte sectors\n"
	"  --4096        4096-byte sectors\n"
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
			if (!strcmp(arg + 2, "512")) {
				opts.sector_size = 512;
				continue;
			}
			if (!strcmp(arg + 2, "1024")) {
				opts.sector_size = 1024;
				continue;
			}
			if (!strcmp(arg + 2, "2048")) {
				opts.sector_size = 2048;
				continue;
			}
			if (!strcmp(arg + 2, "4096")) {
				opts.sector_size = 4096;
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
