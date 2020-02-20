/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
 * dy-iso.c
 *      Program for creating bootable dancy.iso image
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

/*
 * file_name: ELTORITO.BIN
 * file_size: 2048
 */
extern const unsigned char eltorito_bin[2048];

#define PROGRAM_CMDNAME "dy-iso"

#define W_LE16(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_LE32(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

#define W_BE16(a,d) ( \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_BE32(a,d) ( \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

static const char readme_txt[] =
	"Dancy Operating System\n"
	"";
static size_t dancy_file_size;
static unsigned char *dancy_file;
static struct tm iso_tm_arg;
static struct tm *iso_tm;

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	const char *arg_t;
	int uefi;
	int verbose;
};

static void write_timestamp(int version, unsigned char *out)
{
	if (version == 1) {
		sprintf((char *)out + 0,  "%04i", iso_tm->tm_year + 1900);
		sprintf((char *)out + 4,  "%02i", iso_tm->tm_mon + 1);
		sprintf((char *)out + 6,  "%02i", iso_tm->tm_mday);
		sprintf((char *)out + 8,  "%02i", iso_tm->tm_hour);
		sprintf((char *)out + 10, "%02i", iso_tm->tm_min);
		sprintf((char *)out + 12, "%02i", iso_tm->tm_sec);
		sprintf((char *)out + 14, "%02i", 0);
		*(out + 16) = 0x00;
		return;
	}
	if (version == 2) {
		if (iso_tm->tm_year <= UCHAR_MAX)
			*(out + 0) = (unsigned char)iso_tm->tm_year;
		else
			*(out + 0) = 0xFF;
		*(out + 1) = (unsigned char)(iso_tm->tm_mon + 1);
		*(out + 2) = (unsigned char)iso_tm->tm_mday;
		*(out + 3) = (unsigned char)iso_tm->tm_hour;
		*(out + 4) = (unsigned char)iso_tm->tm_min;
		*(out + 5) = (unsigned char)iso_tm->tm_sec;
		*(out + 6) = 0x00;
		return;
	}
}

static int create_iso(int uefi)
{
	size_t size = dancy_file_size;
	unsigned char *ptr;

	if (size < 0x0800) {
		fputs("Error: too small dancy-image-file\n", stderr);
		return 1;
	}
	if (size > (0xFFFF * 0x0800)) {
		fputs("Error: too big dancy-image-file\n", stderr);
		return 1;
	}
	if ((size & 0x07FF) != 0) {
		fputs("Error: alignment of dancy-image-file\n", stderr);
		return 1;
	}
	if (dancy_file[0] != 0xEB || dancy_file[12] != 0x08) {
		fputs("Error: format of dancy-image-file\n", stderr);
		return 1;
	}

	size += 0x00280000;
	size &= 0xFFF00000;
	ptr = realloc(dancy_file, size);
	if (!ptr) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}
	dancy_file = ptr;

	memset(&dancy_file[dancy_file_size], 0, (size - dancy_file_size));
	memmove(&dancy_file[0x80000], &dancy_file[0], dancy_file_size);
	memset(&dancy_file[0], 0, 0x80000);

	/*
	 * ECMA-119 Second Edition
	 * Primary Volume Descriptor
	 *
	 * Volume Descriptor Type
	 */
	ptr[0x8000] = 0x01;
	/*
	 * Standard Identifier
	 */
	memcpy(&ptr[0x8001], "CD001", 5);
	/*
	 * Volume Descriptor Version
	 */
	ptr[0x8006] = 0x01;
	/*
	 * System Identifier and Volume Identifier
	 */
	memcpy(&ptr[0x8008], "DANCY   ", 8);
	memset(&ptr[0x8010], ' ', 24);
	memcpy(&ptr[0x8028], "CDROM   ", 8);
	memset(&ptr[0x8030], ' ', 24);
	/*
	 * Volume Space Size (LSB and MSB)
	 */
	W_LE32(&ptr[0x8050], size);
	W_BE32(&ptr[0x8054], size);
	/*
	 * Volume Set Size (LSB and MSB)
	 */
	W_LE16(&ptr[0x8078], 0x0001);
	W_BE16(&ptr[0x807A], 0x0001);
	/*
	 * Volume Sequence Number (LSB and MSB)
	 */
	W_LE16(&ptr[0x807C], 0x0001);
	W_BE16(&ptr[0x807E], 0x0001);
	/*
	 * Logical Block Size (LSB and MSB)
	 */
	W_LE16(&ptr[0x8080], 0x0800);
	W_BE16(&ptr[0x8082], 0x0800);
	/*
	 * Path Table Size (LSB and MSB)
	 */
	W_LE32(&ptr[0x8084], 0x0000000A);
	W_BE32(&ptr[0x8088], 0x0000000A);
	/*
	 * Location of Occurrence of Type L Path Table
	 */
	W_LE32(&ptr[0x808C], 0x00000014);
	/*
	 * Location of Occurrence of Type M Path Table
	 */
	W_BE32(&ptr[0x8094], 0x00000018);

	/*
	 * Directory Record for Root Directory
	 *
	 * Length of Directory Record
	 */
	ptr[0x809C] = 0x22;
	/*
	 * Location of Extent (LSB and MSB)
	 */
	W_LE32(&ptr[0x809E], 0x00000020);
	W_BE32(&ptr[0x80A2], 0x00000020);
	/*
	 * Data Length (LSB and MSB)
	 */
	W_LE32(&ptr[0x80A6], 0x00000800);
	W_BE32(&ptr[0x80AA], 0x00000800);
	/*
	 * Recording Date and Time
	 */
	write_timestamp(2, &ptr[0x80AE]);
	/*
	 * Recording Date and Time
	 */
	ptr[0x80B5] = 0x02;
	/*
	 * Volume Sequence Number (LSB and MSB)
	 */
	W_LE16(&ptr[0x80B8], 0x0001);
	W_BE16(&ptr[0x80BA], 0x0001);
	/*
	 * Length of File Identifier
	 */
	ptr[0x80BC] = 0x01;
	/*
	 * Volume Set Identifier, etc.
	 */
	memset(&ptr[0x80BE], ' ', 623);
	/*
	 * Volume Creation Date and Time
	 */
	write_timestamp(1, &ptr[0x832D]);
	/*
	 * Volume Modification Date and Time
	 */
	write_timestamp(1, &ptr[0x833E]);
	/*
	 * Volume Expiration Date and Time
	 */
	memset(&ptr[0x834F], '0', 16);
	/*
	 * Volume Effective Date and Time
	 */
	write_timestamp(1, &ptr[0x8360]);
	/*
	 * File Structure Version
	 */
	ptr[0x8371] = 0x01;

	/*
	 * "El Torito" Bootable CD-ROM Specification Version 1.0
	 */
	ptr[0x8800] = 0x00;
	memcpy(&ptr[0x8801], "CD001", 5);
	ptr[0x8806] = 0x01;
	memcpy(&ptr[0x8807], "EL TORITO SPECIFICATION", 23);
	/*
	 * Pointer to first sector of Boot Catalog
	 */
	W_LE32(&ptr[0x8847], 0x00000080);

	/*
	 * Volume Descriptor Set Terminator
	 */
	ptr[0x9000] = 0xFF;
	memcpy(&ptr[0x9001], "CD001", 5);
	ptr[0x9006] = 0x01;

	/*
	 * Type L Path Table
	 */
	ptr[0xA000] = 0x01;
	W_LE32(&ptr[0xA002], 0x00000020);
	W_LE16(&ptr[0xA006], 0x0001);
	/*
	 * Type M Path Table
	 */
	ptr[0xC000] = 0x01;
	W_BE32(&ptr[0xC002], 0x00000020);
	W_BE16(&ptr[0xC006], 0x0001);

	/*
	 * Root directory
	 *
	 * "00" Directory
	 */
	ptr = dancy_file + 0x10000;
	ptr[0x00] = 0x22;
	W_LE32(&ptr[0x02], 0x00000020);
	W_BE32(&ptr[0x06], 0x00000020);
	W_LE32(&ptr[0x0A], 0x00000800);
	W_BE32(&ptr[0x0E], 0x00000800);
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x02;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x01;
	ptr[0x21] = 0x00;
	ptr += ptr[0x00];
	/*
	 * "01" Directory
	 */
	ptr[0x00] = 0x22;
	W_LE32(&ptr[0x02], 0x00000020);
	W_BE32(&ptr[0x06], 0x00000020);
	W_LE32(&ptr[0x0A], 0x00000800);
	W_BE32(&ptr[0x0E], 0x00000800);
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x02;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x01;
	ptr[0x21] = 0x01;
	ptr += ptr[0x00];
	/*
	 * "BOOT.CAT" File
	 */
	ptr[0x00] = 0x2C;
	W_LE32(&ptr[0x02], 0x00000080);
	W_BE32(&ptr[0x06], 0x00000080);
	W_LE32(&ptr[0x0A], 0x00000800);
	W_BE32(&ptr[0x0E], 0x00000800);
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x01;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x0A;
	memcpy(&ptr[0x21], "BOOT.CAT;1", 0x0A);
	ptr += ptr[0x00];
	/*
	 * "DANCY.IMG" File
	 */
	ptr[0x00] = 0x2C;
	W_LE32(&ptr[0x02], 0x00000100);
	W_BE32(&ptr[0x06], 0x00000100);
	W_LE32(&ptr[0x0A], dancy_file_size);
	W_BE32(&ptr[0x0E], dancy_file_size);
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x00;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x0B;
	memcpy(&ptr[0x21], "DANCY.IMG;1", 0x0B);
	ptr += ptr[0x00];
	/*
	 * "ELTORITO.BIN" File
	 */
	ptr[0x00] = 0x30;
	W_LE32(&ptr[0x02], 0x000000A0);
	W_BE32(&ptr[0x06], 0x000000A0);
	W_LE32(&ptr[0x0A], 0x00000800);
	W_BE32(&ptr[0x0E], 0x00000800);
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x01;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x0E;
	memcpy(&ptr[0x21], "ELTORITO.BIN;1", 0x0E);
	ptr += ptr[0x00];
	/*
	 * "README.TXT" File
	 */
	ptr[0x00] = 0x2E;
	W_LE32(&ptr[0x02], 0x000000C0);
	W_BE32(&ptr[0x06], 0x000000C0);
	W_LE32(&ptr[0x0A], (sizeof(readme_txt) - 1));
	W_BE32(&ptr[0x0E], (sizeof(readme_txt) - 1));
	write_timestamp(2, &ptr[0x12]);
	ptr[0x19] = 0x00;
	W_LE16(&ptr[0x1C], 0x0001);
	W_BE16(&ptr[0x1E], 0x0001);
	ptr[0x20] = 0x0C;
	memcpy(&ptr[0x21], "README.TXT;1", 0x0C);
	ptr += ptr[0x00];

	/*
	 * El Torito Bootable CD-ROM Specification Version 1.0
	 * Validation Entry and Default Entry
	 */
	ptr = dancy_file + 0x40000;
	ptr[0x00] = 0x01;
	ptr[0x1C] = 0xAA, ptr[0x1D] = 0x55;
	ptr[0x1E] = 0x55, ptr[0x1F] = 0xAA;
	ptr[0x20] = 0x88;
	ptr[0x26] = 0x04;
	W_LE32(&ptr[0x28], 0x000000A0);

	if (uefi) {
		size_t efi_size = dancy_file_size / 512;
		ptr[0x40] = 0x91, ptr[0x41] = 0xEF, ptr[0x42] = 0x01;
		ptr[0x60] = 0x88;
		W_LE16(&ptr[0x66], efi_size);
		W_LE32(&ptr[0x68], 0x00000100);
	}

	/*
	 * "ELTORITO.BIN" Data
	 */
	ptr = dancy_file + 0x50000;
	memcpy(&ptr[0], &eltorito_bin[0], 2048);
	/*
	 * "README.TXT" Data
	 */
	ptr = dancy_file + 0x60000;
	memcpy(&ptr[0], &readme_txt[0], (sizeof(readme_txt) - 1));

	dancy_file_size = size;
	return 0;
}

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	const size_t chunk = 0x100000;
	FILE *fp = stdin;
	int is_stdin = 1;
	unsigned char *ptr;
	size_t size_max;

	size_max = 0, size_max--;

	if (strcmp(name, "-")) {
		fp = (errno = 0, fopen(name, "rb"));
		if (!fp) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return 1;
		}
		is_stdin = 0;
	}

	*size = 0;
	ptr = malloc(chunk);
	for (;;) {
		size_t bytes_read;
		int my_errno;
		int stop;

		if (!ptr) {
			fputs("Error: not enough memory\n", stderr);
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}
		*out = ptr;
		ptr += *size;

		bytes_read = (errno = 0, fread(ptr, 1, chunk, fp));
		my_errno = errno;
		stop = feof(fp);

		if (ferror(fp) || (!stop && bytes_read != chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(my_errno));
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}

		*size += ((bytes_read <= chunk) ? bytes_read : chunk);
		if (stop)
			break;

		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return !is_stdin ? ((void)fclose(fp), 1) : 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return !is_stdin ? ((void)fclose(fp), 0) : 0;
}

static int end(const char *name, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!out || !size)
		return 1;
	if (strcmp(name, "-")) {
		fp = (errno = 0, fopen(name, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return free(out), 1;
		}
		is_stdout = 0;
	}
	if ((errno = 0, fwrite(out, 1, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return free(out), 1;
	}
	free(out);
	errno = 0;

	if (is_stdout)
		return fflush(fp) ? perror("Error"), 1 : 0;
	return fclose(fp) ? perror("Error"), 1 : 0;
}

static int parse_time(const char *time, struct tm *out)
{
	static const char *e = "Error: wrong timestamp format\n";
	char buf[32];
	int i;

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	memset(out, 0, sizeof(struct tm));
	if (strlen(time) != 19)
		return fputs(e, stderr), 1;
	memcpy(&buf[0], &time[0], 20);

	if (buf[4] != '-' || buf[7] != '-' || buf[10] != 'T')
		return fputs(e, stderr), 1;
	if (buf[13] != ':' || buf[16] != ':')
		return fputs(e, stderr), 1;
	buf[4] = buf[7] = buf[10] = buf[13] = buf[16] = '\0';

	for (i = 0; i < 19; i++) {
		if (buf[i] != '\0' && !isdigit(buf[i]))
			return fputs(e, stderr), 1;
	}

	out->tm_year = atoi(&buf[0]) - 1900;
	out->tm_mon =  atoi(&buf[5]) - 1;
	out->tm_mday = atoi(&buf[8]);
	out->tm_hour = atoi(&buf[11]);
	out->tm_min  = atoi(&buf[14]);
	out->tm_sec  = atoi(&buf[17]);

	i =  (out->tm_year >= 0) ? 0 : 1;
	i |= (out->tm_mon  >= 0 && out->tm_mon  <= 11) ? 0 : 1;
	i |= (out->tm_mday >= 1 && out->tm_mday <= 31) ? 0 : 1;
	i |= (out->tm_hour >= 0 && out->tm_hour <= 23) ? 0 : 1;
	i |= (out->tm_min  >= 0 && out->tm_min  <= 59) ? 0 : 1;
	i |= (out->tm_sec  >= 0 && out->tm_sec  <= 59) ? 0 : 1;

	if (i)
		return fputs(e, stderr), 1;
	return 0;
}

int program(struct options *opt)
{
	if (opt->arg_t) {
		if (parse_time(opt->arg_t, &iso_tm_arg))
			return 1;
		iso_tm = &iso_tm_arg;
	} else {
		time_t t = time(NULL);
		iso_tm = gmtime(&t);
	}

	if (!opt->arg_o)
		return opt->error = "missing output", 1;
	if (!opt->operands[0])
		return opt->error = "missing dancy-image-file", 1;
	if (opt->operands[1])
		return opt->error = "too many operands", 1;

	if (read_file(opt->operands[0], &dancy_file, &dancy_file_size))
		return 1;
	if (create_iso(opt->uefi))
		return free(dancy_file), 1;
	return end(opt->arg_o, dancy_file, dancy_file_size);
}

static const char *help_str =
	"Usage: " PROGRAM_CMDNAME
	" -o output [-t timestamp] dancy-image-file\n"
	"\nOptions:\n"
	"  -o output     output file\n"
	"  -t timestamp  YYYY-MM-DDThh:mm:ss\n"
	"  --uefi        EFI System Partition\n"
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
			if (!strcmp(arg + 2, "uefi")) {
				opts.uefi = 1;
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
