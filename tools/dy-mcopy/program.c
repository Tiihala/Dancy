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
	unsigned char *table0;
	unsigned char *table1;
	unsigned char *root;
	unsigned char *data;
	unsigned long data_size;
	unsigned long table_size;
	unsigned long cluster_size;
	unsigned long total_size;
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
	unsigned c_date;
	unsigned c_time;
	unsigned m_date;
	unsigned m_time;
	unsigned read_only;
	unsigned random;
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

	i = (out->tm_year >= 80) ? 0 : 1;
	i |= (out->tm_mon  >= 0 && out->tm_mon  <= 11) ? 0 : 1;
	i |= (out->tm_mday >= 1 && out->tm_mday <= 31) ? 0 : 1;
	i |= (out->tm_hour >= 0 && out->tm_hour <= 23) ? 0 : 1;
	i |= (out->tm_min  >= 0 && out->tm_min  <= 59) ? 0 : 1;
	i |= (out->tm_sec  >= 0 && out->tm_sec  <= 59) ? 0 : 1;

	if (i)
		return fputs(e, stderr), 1;
	return 0;
}

static int get_modification_time(struct options *opt, struct param_block *pb)
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
	if (local->tm_year < 1980 || local->tm_year > 2107)
		return 0;

	u = (unsigned)local->tm_mday & 0x1Fu;
	u |= (((unsigned)local->tm_mon + 1u) & 0x0Fu) << 5;
	u |= (((unsigned)local->tm_year - 80u) & 0x7Fu) << 9;
	pb->m_date = u;

	u = ((unsigned)local->tm_sec >> 1) & 0x1Fu;
	u |= ((unsigned)local->tm_min & 0x1Fu) << 5;
	u |= ((unsigned)local->tm_hour & 0x1Fu) << 11;
	pb->m_time = u;

	if (opt->arg_t) {
		pb->c_date = pb->m_date;
		pb->c_time = pb->m_time;
	}
	return 0;
}

static void set_modification_time(struct param_block *pb, void *entry)
{
	unsigned char *e = entry;

	if (pb->c_date) {
		W_LE16(&e[14], pb->c_time);
		W_LE16(&e[16], pb->c_date);
	} else if (LE16(&e[16])) {
		W_LE16(&e[14], pb->m_time);
		W_LE16(&e[16], pb->m_date);
	}
	W_LE16(&e[18], pb->m_date);
	W_LE16(&e[22], pb->m_time);
	W_LE16(&e[24], pb->m_date);

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
		pb->cluster_size = t5 * (unsigned long)pb->bytes_per_sector;
		pb->total_size = t6 * (unsigned long)pb->bytes_per_sector;
		t7 = (unsigned long)pb->bytes_per_sector;

		pb->table0 = image_file + (t1 * t7);
		pb->table1 = NULL;
		if (pb->tables == 2) {
			size_t num = (size_t)(t2 * t7);
			pb->table1 = image_file + ((t1 + t2) * t7);
			if (memcmp(pb->table0, pb->table1, num)) {
				fputs("Error: table differences\n", stderr);
				return 1;
			}
		}
	}

	pb->read_only = opt->read_only;
	pb->random = opt->random;

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
		printf("Info: table_size        %lu\n",   pb->table_size);
		printf("Info: cluster_size      %lu\n",   pb->cluster_size);
		printf("Info: total_size        %lu\n\n", pb->total_size);
	}
	return get_modification_time(opt, pb);
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

static unsigned char *get_pointer(struct param_block *pb, unsigned cluster)
{
	if (cluster < 2 || cluster - 2 >= pb->clusters)
		return NULL;
	return pb->data + (pb->cluster_size * (unsigned long)(cluster - 2));
}

static unsigned table(struct param_block *pb, unsigned off, unsigned *val)
{
	unsigned long loff = (unsigned long)off;
	unsigned new_val = (val != NULL) ? *val : 0u;
	unsigned old_val;
	unsigned ret_val;

	if (!is_fat16(pb)) {
		loff += (unsigned long)off / 2ul;
		ret_val = old_val = (unsigned)LE16(&pb->table0[loff]);
		if (off & 1u) {
			ret_val = ret_val >> 4;
			new_val = new_val << 4;
			new_val = (new_val & 0xFFF0u) | (old_val & 0x000Fu);
		} else {
			new_val = (new_val & 0x0FFFu) | (old_val & 0xF000u);
		}
		ret_val &= 0x0FFFu;
	} else {
		loff += (unsigned long)off;
		ret_val = (unsigned)LE16(&pb->table0[loff]);
	}

	if (val != NULL) {
		W_LE16(&pb->table0[loff], new_val);
		if (pb->table1)
			W_LE16(&pb->table1[loff], new_val);
	}
	return ret_val;
}

static unsigned allocate_cluster(struct param_block *pb)
{
	static int random = 0;
	unsigned val = 0xFFFFu;
	unsigned i;

	if (pb->random) {
		unsigned retry = 32;
		random = (!random) ? (srand((unsigned)time(NULL)), 1) : 1;
		while (retry--) {
			i = ((unsigned)rand % pb->clusters) + 2u;
			if (!table(pb, i, NULL))
				return (void)table(pb, i, &val), i;
		}
	}
	for (i = 2; i < pb->clusters + 2u; i++) {
		if (!table(pb, i, NULL))
			return (void)table(pb, i, &val), i;
	}
	return 0;
}

static int directory(struct param_block *pb, const char *name, void **current)
{
	static const char *e1 = "Error: no free space\n";
	static const char *e2 = "Error: inconsistent file system\n";
	static const char *e3 = "Error: directory name already used\n";
	static const char *e4 = "Error: no free root directory entries\n";
	unsigned char *entry;
	unsigned cluster;
	unsigned i;
	void *ptr;

	/*
	 * Root directory.
	 */
	if (*current == NULL) {
		unsigned owner = 0;
		for (i = 0; i < pb->directory_entries; i++) {
			entry = pb->root + (i * 32u);
			if (!entry[0]) {
				cluster = allocate_cluster(pb);
				ptr = get_pointer(pb, cluster);

				if (ptr == NULL)
					return fputs(e1, stderr), 1;

				memset(&entry[0], 0, 32);
				memcpy(&entry[0], &name[0], 11);
				entry[11] = 16;
				W_LE16(&entry[26], cluster);
				set_modification_time(pb, &entry[0]);

				entry = ptr;
				memset(&entry[0], 0, pb->cluster_size);
				memcpy(&entry[0], ".          ", 11);
				entry[11] = 16;
				W_LE16(&entry[26], cluster);

				entry += 32;
				memset(&entry[0], 0, pb->cluster_size);
				memcpy(&entry[0], "..         ", 11);
				entry[11] = 16;
				W_LE16(&entry[26], owner);

				*current = ptr;
				return 0;
			}
			if (!memcmp(&entry[0], &name[0], 11)) {
				cluster = (unsigned)LE16(&entry[26]);
				ptr = get_pointer(pb, cluster);

				if (ptr == NULL)
					return fputs(e2, stderr), 1;
				if (!((unsigned)entry[11] & 16u))
					return fputs(e3, stderr), 1;
				set_modification_time(pb, &entry[0]);

				*current = ptr;
				return 0;
			}
		}
		return fputs(e4, stderr), 1;
	}

	/*
	 * Subdirectory.
	 */
	if ((entry = *current) != NULL) {
		unsigned owner = (unsigned)LE16(&entry[26]);

		cluster = owner;
		ptr = get_pointer(pb, cluster);
		if (memcmp(&entry[0], ".          ", 11))
			return fputs(e2, stderr), 1;
		if (memcmp(&entry[32], "..         ", 11))
			return fputs(e2, stderr), 1;
		if (entry != (unsigned char *)ptr)
			return fputs(e2, stderr), 1;

		set_modification_time(pb, &entry[0]);
		set_modification_time(pb, &entry[32]);

		for (i = 64; /* void */; i += 32) {
			if (i >= pb->cluster_size) {
				unsigned c = table(pb, cluster, NULL);
				ptr = get_pointer(pb, c);
				if (ptr == NULL) {
					c = allocate_cluster(pb);
					ptr = get_pointer(pb, c);
					if (ptr == NULL)
						return fputs(e1, stderr), 1;
					(void)table(pb, cluster, &c);
				}
				*current = ptr;
				i = 0;
			}

			entry = (unsigned char *)*current + i;
			if (!entry[0]) {
				cluster = allocate_cluster(pb);
				ptr = get_pointer(pb, cluster);

				if (ptr == NULL)
					return fputs(e1, stderr), 1;

				memset(&entry[0], 0, 32);
				memcpy(&entry[0], &name[0], 11);
				entry[11] = 16;
				W_LE16(&entry[26], cluster);
				set_modification_time(pb, &entry[0]);

				entry = ptr;
				memset(&entry[0], 0, pb->cluster_size);
				memcpy(&entry[0], ".          ", 11);
				entry[11] = 16;
				W_LE16(&entry[26], cluster);

				entry += 32;
				memset(&entry[0], 0, pb->cluster_size);
				memcpy(&entry[0], "..         ", 11);
				entry[11] = 16;
				W_LE16(&entry[26], owner);

				*current = ptr;
				return 0;
			}
			if (!memcmp(&entry[0], &name[0], 11)) {
				cluster = (unsigned)LE16(&entry[26]);
				ptr = get_pointer(pb, cluster);

				if (ptr == NULL)
					return fputs(e2, stderr), 1;
				if (!((unsigned)entry[11] & 16u))
					return fputs(e3, stderr), 1;
				set_modification_time(pb, &entry[0]);

				*current = ptr;
				return 0;
			}
		}
	}
	return fputs(e2, stderr), 1;
}

static int file(struct param_block *pb, const char *name, void **current)
{
	static const char *e1 = "Error: no free space\n";
	static const char *e2 = "Error: inconsistent file system\n";
	static const char *e3 = "Error: file name already used\n";
	static const char *e4 = "Error: no free root directory entries\n";
	unsigned char *entry = NULL;
	unsigned cluster;
	unsigned i;
	void *ptr;

	/*
	 * Root directory.
	 */
	if (*current == NULL) {
		for (i = 0; i < pb->directory_entries; i++) {
			entry = pb->root + (i * 32u);
			if (!entry[0])
				break;
			if (!memcmp(&entry[0], &name[0], 11))
				return fputs(e3, stderr), 1;
			entry = NULL;
		}
		if (entry == NULL)
			return fputs(e4, stderr), 1;
	}

	/*
	 * Subdirectory.
	 */
	if (entry == NULL && (entry = *current) != NULL) {
		cluster = (unsigned)LE16(&entry[26]);
		ptr = get_pointer(pb, cluster);

		if (memcmp(&entry[0], ".          ", 11))
			return fputs(e2, stderr), 1;
		if (memcmp(&entry[32], "..         ", 11))
			return fputs(e2, stderr), 1;
		if (entry != (unsigned char *)ptr)
			return fputs(e2, stderr), 1;

		set_modification_time(pb, &entry[0]);
		set_modification_time(pb, &entry[32]);

		for (i = 64; /* void */; i += 32) {
			if (i >= pb->cluster_size) {
				unsigned c = table(pb, cluster, NULL);
				ptr = get_pointer(pb, c);
				if (ptr == NULL) {
					c = allocate_cluster(pb);
					ptr = get_pointer(pb, c);
					if (ptr == NULL)
						return fputs(e1, stderr), 1;
					(void)table(pb, cluster, &c);
				}
				*current = ptr;
				i = 0;
			}

			entry = (unsigned char *)*current + i;
			if (!entry[0])
				break;
			if (!memcmp(&entry[0], &name[0], 11))
				return fputs(e3, stderr), 1;
		}
	}

	if (entry != NULL) {
		size_t written = 0;
		size_t size;

		memset(&entry[0], 0, 32);
		memcpy(&entry[0], &name[0], 11);
		if (pb->read_only)
			entry[11] = 0x01;
		set_modification_time(pb, &entry[0]);
		if (!source_file_size)
			return 0;

		cluster = allocate_cluster(pb);
		W_LE16(&entry[26], cluster);
		W_LE32(&entry[28], source_file_size);

		for (;;) {
			ptr = get_pointer(pb, cluster);
			if (ptr == NULL)
				return fputs(e1, stderr), 1;

			size = source_file_size - written;
			if (size <= (size_t)pb->cluster_size) {
				memset(ptr, 0, pb->cluster_size);
				memcpy(ptr, &source_file[written], size);
				return 0;
			} else {
				unsigned new_c = allocate_cluster(pb);
				(void)table(pb, cluster, &new_c);
				cluster = new_c;
			}
			size = pb->cluster_size;
			memcpy(ptr, &source_file[written], size);
			written += size;
		}
	}
	return 1;
}

static int mcopy(struct options *opt, struct param_block *pb)
{
	void *current = NULL;
	size_t size = image_file_size;
	size_t name_offset = 0;
	char name[12];
	FILE *fp;

	for (;;) {
		int r = get_name(opt, &name_offset, &name[0]);

		if (r == 1) {
			if (directory(pb, &name[0], &current))
				return 1;
			continue;
		}
		if (r == 2) {
			if (file(pb, &name[0], &current))
				return 1;
			break;
		}
		fputs("Error: incorrect path/destination\n", stderr);
		return 1;
	}

	fp = (errno = 0, fopen(opt->arg_i, "wb"));
	if (!fp) {
		const char *fmt = "Error: opening file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_i, strerror(errno));
		return 1;
	}
	if ((errno = 0, fwrite(image_file, 1u, size, fp)) != size) {
		const char *fmt = "Error: writing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_i, strerror(errno));
		return 1;
	}
	if ((errno = 0, fclose(fp))) {
		const char *fmt = "Error: closing file \"%s\" (%s)\n";
		fprintf(stderr, fmt, opt->arg_i, strerror(errno));
		return 1;
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
	if (opt->operands[2])
		return opt->error = "too many operands", 1;

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
