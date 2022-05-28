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
 * dy-mcopy/program.c
 *      Program for adding files into FAT file system images
 */

#include "program.h"

size_t source_file_size;
unsigned char *source_file;

static FILE *image_file;

static struct tm *fat_tm;
static struct tm fat_tm_buf;

size_t fat_block_size = 512;
size_t fat_block_total = 0;

int fat_get_size(int id, size_t *block_size, size_t *block_total)
{
	(void)id;

	*block_size = fat_block_size;

	if (fat_block_total < 0xFFFFFFFF)
		*block_total = fat_block_total;
	else
		*block_total = 0xFFFFFFFF;

	return 0;
}

int fat_get_time(char iso_8601_format[19])
{
	char buf[128];

	if (!fat_tm) {
		memset(&iso_8601_format[0], 0, 19);
		return 1;
	}

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	sprintf(&buf[0], "%04d-%02d-%02dT%02d:%02d:%02d",
		fat_tm->tm_year + 1900, fat_tm->tm_mon + 1, fat_tm->tm_mday,
		fat_tm->tm_hour, fat_tm->tm_min, fat_tm->tm_sec);

	memcpy(&iso_8601_format[0], &buf[0], 19);
	return 0;
}

int fat_io_read(int id, size_t lba, size_t *size, void *buf)
{
	long offset;

	(void)id;

	if (lba > fat_block_total)
		return *size = 0, 1;

	offset = (long)(lba * fat_block_size);

	if ((errno = 0, fseek(image_file, offset, SEEK_SET)) != 0) {
		perror("Read image file");
		return *size = 0, 1;
	}

	if ((errno = 0, fread(buf, 1, *size, image_file)) != *size) {
		perror("Read image file");
		return *size = 0, 1;
	}

	return 0;
}

int fat_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	long offset;

	(void)id;

	if (lba > fat_block_total)
		return *size = 0, 1;

	offset = (long)(lba * fat_block_size);

	if ((errno = 0, fseek(image_file, offset, SEEK_SET)) != 0) {
		perror("Write image file");
		return *size = 0, 1;
	}

	if ((errno = 0, fwrite(buf, 1, *size, image_file)) != *size) {
		perror("Write image file");
		return *size = 0, 1;
	}

	return 0;
}

void fat_error(int r)
{
	const char *err = "unknown error";

	switch (r) {
	case 0x10:
		err = "block read error";
		break;
	case 0x11:
		err = "block write error";
		break;
	case 0x12:
		err = "directory not empty";
		break;
	case 0x13:
		err = "file already open";
		break;
	case 0x14:
		err = "file not found";
		break;
	case 0x15:
		err = "inconsistent file system";
		break;
	case 0x16:
		err = "invalid file name";
		break;
	case 0x17:
		err = "invalid parameters";
		break;
	case 0x18:
		err = "not enough space";
		break;
	case 0x19:
		err = "file system not ready";
		break;
	case 0x1A:
		err = "read-only file";
		break;
	case 0x1B:
		err = "read-only record";
		break;
	case 0x1C:
		err = "seek error";
		break;
	default:
		break;
	}

	fprintf(stderr, "Error: %s\n", err);
}

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

static void free_files(void)
{
	if (source_file) {
		free(source_file);
		source_file_size = 0;
		source_file = NULL;
	}
}

static int parse_time(const char *time_str, struct tm *out)
{
	static const char *e = "Error: wrong timestamp format\n";
	char buf[32];
	int i;

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	memset(out, 0, sizeof(struct tm));
	if (strlen(time_str) != 19)
		return fputs(e, stderr), 1;
	memcpy(&buf[0], &time_str[0], 20);

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

static int db_files(struct options *opt)
{
	static char src[32];
	static char dst[32];
	size_t len = strlen(opt->operands[0]);
	unsigned i = 0;
	unsigned long limit = 1000;
	FILE *fp;

	if (opt->operands[1]) {
		limit = strtoul(opt->operands[1], NULL, 0);
		if (limit < 1 || limit > 1000 || opt->operands[2])
			return opt->error = "db mode operands", 1;
	}

	/*
	 * Handle "db_???.at" (or e.g. "system/db_???.at") as a special case.
	 */
	if (len < 9 || len > 31)
		return opt->error = "db mode operands", 1;
	strcpy(&src[0], opt->operands[0]);
	src[len - 9] = (char)tolower((int)src[len - 9]);
	src[len - 8] = (char)tolower((int)src[len - 8]);
	src[len - 6] = isdigit((int)src[len - 6]) ? '0' : '-';
	src[len - 5] = isdigit((int)src[len - 5]) ? '0' : '-';
	src[len - 4] = isdigit((int)src[len - 4]) ? '0' : '-';
	src[len - 2] = (char)tolower((int)src[len - 2]);
	src[len - 1] = (char)tolower((int)src[len - 1]);
	if (strcmp(&src[len - 9], "db_000.at"))
		return opt->error = "db mode operands", 1;
	strcpy(&src[0], opt->operands[0]);

	i += (unsigned)(((src[len - 6]) - '0') * 100);
	i += (unsigned)(((src[len - 5]) - '0') * 10);
	i += (unsigned)(((src[len - 4]) - '0') * 1);

	for (/* void */; i < 1000; i++) {
		sprintf(&src[len - 6], "%03u", i), src[len - 3] = '.';
		sprintf(&dst[0], "::SYSTEM/DB_%03u.AT", i);

		if ((fp = fopen(&src[0], "rb")) == NULL)
			break;
		(void)fclose(fp);

		if (source_file) {
			free(source_file);
			source_file_size = 0;
			source_file = NULL;
		}
		if (read_file(&src[0], &source_file, &source_file_size))
			return 1;
		/*
		 * opt->operands[1] can always be accessed safely, and the
		 * array would still be null-terminated, because at least
		 * the long option --db has been available in main's argv.
		 */
		opt->operands[1] = &dst[0];
		if (mcopy(opt))
			return 1;
		if (--limit == 0)
			break;
	}
	return 0;
}

int program(struct options *opt)
{
	int r;

	fat_block_total = (~((size_t)0)) / fat_block_size;
	if (fat_block_total > (unsigned long)(LONG_MAX / fat_block_size))
		fat_block_total = (size_t)(LONG_MAX / fat_block_size);

	if ((errno = 0, atexit(free_files)))
		return perror("atexit error"), 1;

	if (!opt->arg_i)
		return opt->error = "missing image file", 1;
	if (!opt->operands[0])
		return opt->error = "missing source-file", 1;
	if (!opt->operands[1] && !opt->db_mode)
		return opt->error = "missing path/destination-file", 1;

	if (!opt->arg_t) {
		time_t t = time(NULL);
		fat_tm = localtime(&t);
	} else {
		if (parse_time(opt->arg_t, &fat_tm_buf))
			return 1;
		fat_tm = &fat_tm_buf;
	}

	image_file = (errno = 0, fopen(opt->arg_i, "rb+"));
	if (!image_file) {
		const char *fmt = "Error: opening file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_i, strerror(errno));
		return 1;
	}

	if (fat_create(&opt->fat, 0)) {
		fputs("Error: file system is not supported\n", stderr);
		(void)fclose(image_file);
		return 1;
	}

	if (opt->db_mode) {
		r = db_files(opt);

		(void)fat_delete(opt->fat);

		if ((errno = 0, fclose(image_file)))
			return perror("Error"), 1;

		return r;
	}

	if (opt->operands[2]) {
		opt->error = "too many operands";

		(void)fat_delete(opt->fat);
		(void)fclose(image_file);

		return 1;
	}

	if (read_file(opt->operands[0], &source_file, &source_file_size)) {
		(void)fat_delete(opt->fat);
		(void)fclose(image_file);

		return 1;
	}

	r = mcopy(opt);

	(void)fat_delete(opt->fat);

	if ((errno = 0, fclose(image_file)))
		return perror("Error"), 1;

	return r;
}
