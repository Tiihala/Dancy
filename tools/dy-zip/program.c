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
 * dy-zip/program.c
 *      Program for creating Zip archives
 */

#include "program.h"

static void put_name(const char *name, char *out)
{
	int i = 0, j = 0;

	if (name[0] == '.' && (name[1] == '\\' || name[1] == '/'))
		i = 2;
	while (name[i] != '\0') {
		char c = name[i];
		out[j] = (c == '\\') ? '/' : c;
		i++, j++;
	}
	out[j] = '\0';
}

static int check_name(const char *name)
{
	char buf[128];
	size_t len;

	if (strlen(name) > 127) {
		fputs("Error: input file name is too long\n", stderr);
		return 1;
	}
	put_name(name, &buf[0]);
	if ((len = strlen(&buf[0])) == 0) {
		fputs("Error: input file name is too short\n", stderr);
		return 1;
	}

	if (!isalnum(buf[0]) || !isalnum(buf[len - 1])) {
		fputs("Error: input file name format\n", stderr);
		return 1;
	}
	if (strstr(&buf[0], "//") || strstr(&buf[0], "/.")) {
		fputs("Error: input file path format\n", stderr);
		return 1;
	}
	return 0;
}

static int create_output(struct options *opt, struct state *zip)
{
	unsigned char *out;
	size_t size = 0;
	size_t off = 0;
	size_t i;

	zip->size = 0x10000;
	zip->output = out = calloc(zip->size, sizeof(unsigned char));
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;

	for (i = 0; i < zip->fnum; i++) {
		unsigned long ul;
		size_t len;

		/*
		 * This limit is important if using database files.
		 */
		if (off > 0xF000) {
			fputs("Error: too many input files\n", stderr);
			return 1;
		}
		put_name(opt->operands[i], (char *)&out[off + 46]);

		/*
		 * File name length.
		 */
		len = strlen((const char *)&out[off + 46]);
		W_LE16(&out[off + 28], len);
		/*
		 * Signature and modification timestamp.
		 */
		W_LE32(&out[off +  0], 0x02014B50);
		W_LE16(&out[off +  4], 0x0014);
		W_LE16(&out[off +  6], 0x0014);
		W_LE16(&out[off + 12], zip->m_time);
		W_LE16(&out[off + 14], zip->m_date);
		/*
		 * CRC-32
		 */
		ul = crc32(zip->fdata[i], zip->fsize[i]);
		W_LE32(&out[off + 16], ul);
		/*
		 * Uncompressed size.
		 */
		ul = zip->fsize[i];
		W_LE32(&out[off + 24], ul);
		/*
		 * Compressed size.
		 */
		if (!deflate_compress(zip->fdata[i], &zip->fsize[i]))
			out[off + 10] = 8;
		ul = zip->fsize[i];
		W_LE32(&out[off + 20], ul);
		/*
		 * Calculate total size (local file header + data).
		 */
		size += 30ul + len + ul;
		if (zip->split) {
			size += 0x0000000Fu;
			size &= 0xFFFFFFF0u;
		}
		off += 46u + len;
	}

	/*
	 * Use special alignment for database files. The output buffer
	 * contains "number_of_dbs * 65504" blocks that are converted
	 * to database files by adding a 32-byte header at the start,
	 * and the size of one database file is 32 + 65504 == 0x10000.
	 * It is guaranteed that the last "block" of the buffer is less
	 * than 65504 bytes and that contains the central directory, so
	 * ignoring the remainder, the "zip->size / 65504" gives a total
	 * number of databases.
	 */
	if (zip->split) {
		size_t remainder = size % 65504u;
		if (remainder)
			size += (65504u - remainder);
	}

	/*
	 * End of central directory record.
	 */
	W_LE32(&out[off +  0], 0x06054B50);
	W_LE16(&out[off +  8], zip->fnum);
	W_LE16(&out[off + 10], zip->fnum);
	W_LE32(&out[off + 12], off);
	W_LE32(&out[off + 16], size);
	off += 22;

	/*
	 * Adjust the buffer size.
	 */
	zip->size = size + off;
	out = realloc(zip->output, size + off);
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;
	zip->output = out;
	zip->size = size + off;

	memmove(&out[size], &out[0], off);
	memset(&out[0], 0, size);
	off = size;
	size = 0;

	/*
	 * Write the local headers and data.
	 */
	for (i = 0; i < zip->fnum; i++) {
		size_t len = LE16(&out[off + 28]);

		W_LE32(&out[size + 0], 0x04034B50);
		memcpy(&out[size + 4], &out[off + 6], 22);
		W_LE16(&out[size + 26], len);
		W_LE16(&out[size + 28], 0);
		memcpy(&out[size + 30], &out[off + 46], len);

		W_LE32(&out[off + 42], size);
		size += len + 30;
		memcpy(&out[size], zip->fdata[i], zip->fsize[i]);
		size += zip->fsize[i];
		if (zip->split) {
			size += 0x0000000Fu;
			size &= 0xFFFFFFF0u;
		}
		off += len + 46;
	}
	return 0;
}

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	size_t chunk = 0x1000;
	unsigned char *ptr;
	size_t size_max;
	FILE *fp;

	size_max = 0, size_max--;

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

		bytes_read = (errno = 0, fread(ptr, 1u, chunk, fp));
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

		if (chunk < 0x00200000)
			chunk <<= 1;
		if (!(*size < size_max - chunk)) {
			const char *fmt = "Error: reading file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, "size_t overflow");
			return (void)fclose(fp), 1;
		}
		ptr = realloc(*out, *size + chunk);
	}
	return (void)fclose(fp), 0;
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

static int get_modification_time(struct options *opt, struct state *zip)
{
	struct tm arg_tm;
	struct tm *local;
	unsigned u;

	if (opt->arg_t) {
		if (parse_time(opt->arg_t, &arg_tm))
			return 1;
		local = &arg_tm;
	} else {
		time_t t = time(NULL);
		local= localtime(&t);
	}
	if (local->tm_year < 80 || local->tm_year > 207) {
		fputs("Warning: timestamp ignored\n", stderr);
		return 0;
	}

	u = (unsigned)local->tm_mday & 0x1Fu;
	u |= (((unsigned)local->tm_mon + 1u) & 0x0Fu) << 5;
	u |= (((unsigned)local->tm_year - 80u) & 0x7Fu) << 9;
	zip->m_date = u;

	u = ((unsigned)local->tm_sec >> 1) & 0x1Fu;
	u |= ((unsigned)local->tm_min & 0x3Fu) << 5;
	u |= ((unsigned)local->tm_hour & 0x1Fu) << 11;
	zip->m_time = u;

	return 0;
}

static void free_zip(struct state *zip)
{
	size_t i;

	if (zip->output)
		free(zip->output);
	if (zip->fdata) {
		for (i = 0; i < zip->fnum; i++)
			free(zip->fdata[i]);
		free(zip->fdata);
	}
	if (zip->fsize)
		free(zip->fsize);
	memset(zip, 0, sizeof(*zip));
}

static int write_output_db(struct options *opt, struct state *zip)
{
	static const unsigned char file_header[16] = {
		0x8Du, 0x41u, 0x54u, 0x0Du, 0x0Au, 0x73u, 0x74u, 0x64u,
		0x0Cu, 0x44u, 0x0Cu, 0x42u, 0x0Cu, 0x0Au, 0x71u, 0xF8u
	};
	unsigned char *db = malloc(0x10000);
	size_t len = strlen(opt->arg_o);
	char name[256];
	FILE *fp;
	int i;

	if (!db)
		return fputs("Error: not enough memory\n", stderr), 1;
	if (len < 9 || len >= sizeof(name)) {
		fputs("Error: output file path format\n", stderr);
		return free(db), 1;
	}
	strcpy(&name[0], &opt->arg_o[0]);

	for (i = 0; i < 1000; i++) {
		if ((unsigned long)i >= (zip->size / 65504u))
			break;
		/*
		 * "db_000.at", change the number only.
		 */
		sprintf(&name[len - 6], "%03i%s", i, &opt->arg_o[len - 3]);
		if (opt->verbose)
			printf("Writing %s\n", name);

		memcpy(&db[0], &file_header[0], 16);
		W_LE32(&db[16], 0x10000);
		W_LE32(&db[20], 0);
		W_LE32(&db[24], 0);
		W_LE32(&db[28], i);
		memcpy(&db[32], &zip->output[i * 65504], 65504);

		fp = (errno = 0, fopen(name, "wb"));
		if (!fp) {
			const char *fmt = "Error: opening file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return free(db), 1;
		}
		if ((errno = 0, fwrite(db, 1, 0x10000, fp)) != 0x10000) {
			const char *fmt = "Error: writing file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return free(db), 1;
		}
		if ((errno = 0, fclose(fp))) {
			const char *fmt = "Error: closing file \"%s\" (%s)\n";
			fprintf(stderr, fmt, name, strerror(errno));
			return free(db), 1;
		}
	}
	/*
	 * Try to remove other database files. This procedure is not meant
	 * to be very robust but is useful when testing the program. The
	 * other strategy should be used for deleting any existing database
	 * files before this program is used.
	 */
	for (/* void */; i < 1000; i++) {
		sprintf(&name[len - 6], "%03i%s", i, &opt->arg_o[len - 3]);
		fp = fopen(name, "rb");
		if (!fp)
			break;
		(void)fclose(fp);
		if ((errno = 0, remove(name))) {
			perror("Error");
			return free(db), 1;
		}
	}
	return free(db), 0;
}

static int write_output(struct options *opt, struct state *zip)
{
	FILE *fp;

	if (!zip->size)
		return 0;

	fp = (errno = 0, fopen(opt->arg_o, "wb"));
	if (!fp) {
		const char *fmt = "Error: opening file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return 1;
	}
	if ((errno = 0, fwrite(zip->output, 1, zip->size, fp)) != zip->size) {
		const char *fmt = "Error: writing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return 1;
	}
	if ((errno = 0, fclose(fp))) {
		const char *fmt = "Error: closing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_o, strerror(errno));
		return 1;
	}
	return 0;
}

int program(struct options *opt)
{
	struct state zip;
	size_t total_size = 0;
	size_t size_max;
	int i;

	size_max = 0, size_max--;

	memset(&zip, 0, sizeof(zip));
	if (opt->operands[0] == NULL)
		return opt->error = "missing source-file", 1;
	for (i = 0; opt->operands[i] != NULL; i++) {
		if (check_name(opt->operands[i]))
			return 1;
		zip.fnum++;
	}
	if (get_modification_time(opt, &zip))
		return 1;
	if (opt->arg_o) {
		const char *o = opt->arg_o;
		size_t len = strlen(o);
		char buf[16];
		/*
		 * Handle "db_000.at" as a special case.
		 */
		if (len >= 9) {
			o = &o[len - 9];
			memcpy(&buf[0], &o[0], 10);
			buf[0] = (char)tolower((int)o[0]);
			buf[1] = (char)tolower((int)o[1]);
			buf[7] = (char)tolower((int)o[7]);
			buf[8] = (char)tolower((int)o[8]);
			if (!strcmp(&buf[0], "db_000.at"))
				zip.split = 1;
		}
		if (opt->verbose && zip.split)
			fputs("Using split mode\n", stdout);
	} else {
		return opt->error = "missing output", 1;
	}

	zip.fdata = calloc(zip.fnum, sizeof(void *));
	zip.fsize = calloc(zip.fnum, sizeof(size_t));
	if (!zip.fdata || !zip.fsize) {
		fputs("Error: not enough memory\n", stderr);
		return free_zip(&zip), 1;
	}

	for (i = 0; opt->operands[i] != NULL; i++) {
		if (read_file(opt->operands[i], &zip.fdata[i], &zip.fsize[i]))
			return free_zip(&zip), 1;
		if (total_size < size_max - zip.fsize[i])
			total_size += zip.fsize[i];
		else
			total_size = size_max;

		if (total_size > 0x7F000000ul || total_size == size_max) {
			fputs("Error: total size of input files\n", stderr);
			return free_zip(&zip), 1;
		}
		if (opt->verbose) {
			const char *fmt = "Read file \"%s\", %u bytes\n";
			printf(fmt, opt->operands[i], (unsigned)zip.fsize[i]);
		}
	}

	if (create_output(opt, &zip))
		return free_zip(&zip), 1;
	if (zip.split ? write_output_db(opt, &zip) : write_output(opt, &zip))
		return free_zip(&zip), 1;
	return free_zip(&zip), 0;
}
