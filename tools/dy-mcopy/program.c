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
 * dy-mcopy/program.c
 *      Program for adding files into FAT12/FAT16 file system images
 */

#include "program.h"

static size_t image_file_size;
static size_t source_file_size;
static unsigned char *image_file;
static unsigned char *source_file;

struct param_block {
	unsigned char *root;
	unsigned char *data;
	unsigned long data_size;
	unsigned long table_size;
	unsigned bytes_per_sector;
	unsigned cluster_sectors;
	unsigned reserved_sectors;
	unsigned tables;
	unsigned directory_entries;
	unsigned directory_sectors;
	unsigned total_sectors;
	unsigned table_sectors;
	unsigned data_sectors;
	unsigned clusters;
};

static int is_fat16(struct param_block *pb)
{
	return (pb->clusters >= 4085u) ? 1 : 0;
}

static int read_file(const char *name, unsigned char **out, size_t *size)
{
	const size_t chunk = 16384;
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

		if (!(*size < SIZE_MAX - chunk)) {
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
	if (image_file) {
		free(image_file);
		image_file_size = 0;
		image_file = NULL;
	}
	if (source_file) {
		free(source_file);
		source_file_size = 0;
		source_file = NULL;
	}
}

static int get_param_block(struct options *opt, struct param_block *pb)
{
	const size_t size = image_file_size;
	const unsigned char *f = image_file;

	if (size < 163840 || f[0] != 0xEB || f[1] < 0x3C) {
		fputs("Error: image is not typical FAT12/FAT16\n", stderr);
		return 1;
	}
	if (LE16(&f[510]) != 0xAA55ul) {
		fputs("Error: missing boot signature (0xAA55)\n", stderr);
		return 1;
	}

	pb->bytes_per_sector = (unsigned)LE16(&f[11]);
	if (pb->bytes_per_sector == 512 || pb->bytes_per_sector == 1024)
		; /* Accept */
	else if (pb->bytes_per_sector == 2048 || pb->bytes_per_sector == 4096)
		; /* Accept */
	else
		return fputs("Error: bytes per sector\n", stderr), 1;

	pb->cluster_sectors = (unsigned)f[13];
	if (pb->cluster_sectors == 1 || pb->cluster_sectors == 2)
		; /* Accept */
	else if (pb->cluster_sectors == 4 || pb->cluster_sectors == 8)
		; /* Accept */
	else if (pb->cluster_sectors == 16 || pb->cluster_sectors == 32)
		; /* Accept */
	else
		return fputs("Error: sectors per cluster\n", stderr), 1;

	pb->reserved_sectors = (unsigned)LE16(&f[14]);
	if (!pb->reserved_sectors)
		return fputs("Error: reserved sectors\n", stderr), 1;

	pb->tables = (unsigned)f[16];
	if (pb->tables == 1 || pb->tables == 2)
		; /* Accept */
	else
		return fputs("Error: number of tables\n", stderr), 1;

	pb->directory_entries = (unsigned)LE16(&f[17]);
	pb->directory_sectors = 0;
	if (pb->directory_entries) {
		unsigned long t1 = LE16(&f[17]) * 32ul;
		unsigned long t2 = t1 / (unsigned long)pb->bytes_per_sector;
		unsigned long t3 = t1 % (unsigned long)pb->bytes_per_sector;
		if (t2 < 65536ul && t3 == 0ul)
			pb->directory_sectors = (unsigned)t2;
	}
	if (!pb->directory_sectors)
		return fputs("Error: directory entries\n", stderr), 1;

	pb->total_sectors = (unsigned)LE16(&f[19]);
	if (pb->total_sectors && !LE32(&f[32])) {
		unsigned long t1 = (unsigned long)pb->total_sectors;
		unsigned long t2 = t1 * (unsigned long)pb->bytes_per_sector;
		if (t2 != size)
			return fputs("Error: total size\n", stderr), 1;
	} else {
		return fputs("Error: total sectors\n", stderr), 1;
	}

	pb->table_sectors = (unsigned)LE16(&f[22]);
	if (!pb->table_sectors)
		return fputs("Error: sectors per table\n", stderr), 1;

	/*
	 * Get root directory pointer and data area pointer. The
	 * size of data area must be at least one cluster.
	 */
	{
		unsigned long t1 = (unsigned long)pb->reserved_sectors;
		unsigned long t2 = (unsigned long)pb->table_sectors;
		unsigned long t3 = (unsigned long)pb->tables;
		unsigned long t4 = (unsigned long)pb->directory_sectors;
		unsigned long t5 = (unsigned long)pb->cluster_sectors;
		unsigned long t6 = (unsigned long)pb->total_sectors;
		unsigned long t7;

		if ((t1 + (t2 * t3) + t4 + t5) > t6)
			return fputs("Error: data area\n", stderr), 1;

		pb->root = image_file;
		pb->root += (unsigned)(t1 + (t2 * t3)) * pb->bytes_per_sector;
		pb->data = pb->root + (unsigned)t4 * pb->bytes_per_sector;

		pb->data_sectors = (unsigned)(t6 - (t1 + (t2 * t3) + t4));
		pb->clusters = pb->data_sectors / pb->cluster_sectors;

		pb->data_size = (unsigned long)pb->clusters * t5;
		pb->data_size *= (unsigned long)pb->bytes_per_sector;

		if (!is_fat16(pb)) {
			t7 = (unsigned long)pb->clusters * 3ul + 1ul;
			t7 &= ~(1ul);
			t7 /= 2ul;
			t7 += 3ul;
		} else {
			t7 = (unsigned long)pb->clusters * 2ul;
			t7 += 4ul;
		}
		if (t7 > t2 * (unsigned long)pb->bytes_per_sector) {
			const char *e = "Error: table size is %lu bytes\n";
			fprintf(stderr, e, t7);
			return 1;
		}
		pb->table_size = t7;
	}

	if (opt->verbose) {
		unsigned unused = pb->data_sectors % pb->cluster_sectors;
		printf("Info: bytes_per_sector  %u\n", pb->bytes_per_sector);
		printf("Info: cluster_sectors   %u\n", pb->cluster_sectors);
		printf("Info: reserved_sectors  %u\n", pb->reserved_sectors);
		printf("Info: tables            %u\n", pb->tables);
		printf("Info: directory_entries %u\n", pb->directory_entries);
		printf("Info: directory_sectors %u\n", pb->directory_sectors);
		printf("Info: total_sectors     %u\n", pb->total_sectors);
		printf("Info: table_sectors     %u\n", pb->table_sectors);
		printf("Info: data_sectors      %u",   pb->data_sectors);
		if (unused)
			printf(" (%u unused sectors at the end)", unused);
		printf("\n");
		printf("Info: clusters          %u\n\n",  pb->clusters);
		printf("Info: data_size         %lu\n",   pb->data_size);
		printf("Info: table_size        %lu\n\n", pb->table_size);
	}
	return 0;
}

static int is_reserved(const char *name)
{
	const char *prefixes[] = {
		"AUX", "COM", "CON", "LPT", "NUL", "PRN"
	};
	size_t i;

	/*
	 * Do not allow the reserved name even as a prefix. This
	 * is stricter than what is necessary.
	 */
	for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); i++) {
		if (!strncmp(name, prefixes[i], strlen(prefixes[i]))) {
			printf("Error: %s is reserved\n", prefixes[i]);
			return 1;
		}
	}
	return 0;
}

static int get_name(struct options *opt, size_t *off, char *out)
{
	const char *dst = opt->operands[1];
	char buf[16];
	int r = -1;
	int i;

	/*
	 * Read destination file path and write directory and file
	 * name components into output buffer. For example:
	 *
	 *      ::my/path/my_file.at
	 *
	 * Call #1: "MY         " (return 1)
	 * Call #2: "PATH       " (return 1)
	 * Call #3: "MY_FILE AT " (return 2)
	 * Call #4: "           " (return 0)
	 *
	 * Return < 0 if there are errors.
	 */
	memset(&out[0], ' ', 11), out[11] = '\0';
	memset(&buf[0], 0, sizeof(buf));

	if (*off == 0) {
		if (strncmp(dst, "::", 2)) {
			fputs("Error: missing \"::\" characters\n", stderr);
			return -1;
		}
		*off += 2;
	}
	if (dst[*off] == '.') {
		fputs("Error: relative path or hidden file\n", stderr);
		return -1;
	}
	if (dst[*off] == '\0') {
		char prev_c = dst[*off - 1];
		if (prev_c == '\\' || prev_c == '/') {
			fputs("Error: missing file name\n", stderr);
			return -1;
		}
		return 0;
	}

	for (i = 0; i < 13; i++) {
		char c = dst[*off];
		if (c == '\0') {
			r = 2;
			break;
		}
		*off += 1;
		c = (char)toupper((int)c);

		if (c == '\\' || c == '/') {
			r = 1;
			break;
		}
		/*
		 * Use only a subset of allowed characters.
		 */
		if (c == '.' || c == '_' || c == '-') {
			; /* Accept */
		} else if (isalnum((int)c)) {
			; /* Accept */
		} else {
			fprintf(stderr, "Error: character \"%c\"\n", c);
			return -1;
		}
		buf[i] = c;
	}

	if (r <= 0 || buf[12] != '\0') {
		buf[12] = '\0';
		fprintf(stderr, "Error: length of \"%s...\"\n", &buf[0]);
		return -1;
	}
	if (is_reserved(&buf[0]))
		return -1;

	for (i = 0; i < 13; i++) {
		char c = buf[i];
		if (c == '\0')
			break;
		if (c == '.') {
			if (r != 2) {
				fputs("Error: \".\" in dir name\n", stderr);
				return -1;
			}
			if (strchr(&buf[i + 1], '.') || buf[i + 4] != '\0') {
				const char *e = "Error: file name \"%s\"\n";
				fprintf(stderr, e, &buf[0]);
				return -1;
			}
			break;
		}
	}

	for (i = 0; i < 9; i++) {
		char c = buf[i];
		if (c == '\0')
			break;
		if (c == '.') {
			out[8]  = (buf[i + 1]) ? buf[i + 1] : ' ';
			out[9]  = (buf[i + 2]) ? buf[i + 2] : ' ';
			out[10] = (buf[i + 3]) ? buf[i + 3] : ' ';
			break;
		}
		if (i == 8) {
			fprintf(stderr, "Error: length of \"%s\"\n", &buf[0]);
			return -1;
		}
		out[i] = c;
	}

	if (out[0] == ' ')
		return fputs("Error: empty path component\n", stderr), -1;
	return r;
}

static int mcopy(struct options *opt, struct param_block *pb)
{
	size_t off = 0;
	char name[12];

	for (;;) {
		int r = get_name(opt, &off, &name[0]);
		if (!r)
			break;
		if (r < 0) {
			fputs("Error: incorrect path/destination\n", stderr);
			return 1;
		}
		if (r == 1)
			printf("D: \"%s\"\n", &name[0]);
		if (r == 2)
			printf("F: \"%s\"\n", &name[0]);
	}
	return 0;
}

int program(struct options *opt)
{
	struct param_block pb;

	if (!opt->arg_i)
		return opt->error = "missing image file", 1;
	if (!opt->operands[0])
		return opt->error = "missing source-file", 1;
	if (!opt->operands[1] || strlen(opt->operands[1]) < 3)
		return opt->error = "missing path/destination-file", 1;

	if ((errno = 0, atexit(free_files)))
		return perror("atexit error"), 1;

	if (read_file(opt->arg_i, &image_file, &image_file_size))
		return 1;
	if (read_file(opt->operands[0], &source_file, &source_file_size))
		return 1;

	if (memset(&pb, 0, sizeof(pb)), get_param_block(opt, &pb))
		return 1;
	return mcopy(opt, &pb);
}
