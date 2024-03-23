/*
 * Copyright (c) 2024 Antti Tiihala
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
 * ls/operate.c
 *      List directory contents
 */

#include "main.h"

static struct options *ls_opt;
static int ls_exit_failure;
static int ls_print_state;
static int ls_tty_width;

#define LS_PATH_BUFFER 4096

struct ls_record {
	char path[LS_PATH_BUFFER];
	struct stat status;
	int state;
	int unicode;
};

static char *ls_strerror(int errnum)
{
	ls_exit_failure = 1;
	return strerror(errnum);
}

static void read_record(const char *p1, const char *p2, struct ls_record *out)
{
	char *path = &out->path[0];
	size_t i;

	memset(out, 0, sizeof(*out));

	if (p1[0] == '\0' || strlen(p1) + strlen(p2) >= LS_PATH_BUFFER - 4) {
		errno = ENOENT;
		fprintf(stderr, "ls: \'%s\': %s\n", path, ls_strerror(errno));
		return;
	}

	strcpy(path, p1);
	out->state = 1;

	if (p2[0] != '\0') {
		strcat(path, "/");
		strcat(path, p2);
		out->state = 2;

		if (p2[0] == '.') {
			if (p2[1] == '\0' || (p2[1] == '.' && p2[2] == '\0'))
				out->state = 3;
		}
	}

	if ((errno = 0, lstat(path, &out->status)) != 0) {
		fprintf(stderr, "ls: \'%s\': %s\n", path, ls_strerror(errno));
		memset(out, 0, sizeof(*out));
		return;
	}

	for (i = 0; out->unicode == 0 && path[i] != '\0'; i++) {
		int c = (int)((unsigned char)path[i]);

		if (c <= 0x20 || c >= 0x7F)
			out->unicode = 1;
	}
}

static void verify_records(size_t *count, struct ls_record *records)
{
	size_t c = 0;
	size_t i;

	for (i = 0; i < *count; i++) {
		struct ls_record *r = &records[i];

		if (r->state != 0)
			c += 1;
	}

	*count = c;
}

#define LS_WIDTH_COUNT 32

static void build_table_width(size_t count, struct ls_record *records,
	size_t width_count, size_t table[2], size_t width[LS_WIDTH_COUNT])
{
	size_t i, j;

	table[0] = width_count;

	if (table[0] == 0)
		table[0] = 1;

	table[1] = count / table[0];

	if ((count % table[0]) != 0)
		table[1] += 1;

	for (i = 0; i < table[0]; i++) {
		size_t new_width = 0;
		struct ls_record *r;
		const char *name;

		for (j = 0; j < table[1]; j++) {
			size_t records_i = (i * table[1]) + j;
			size_t w;

			if (records_i >= count)
				break;

			r = &records[records_i];
			name = &r->path[0];

			if (r->state > 1)
				name = strrchr(name, '/') + 1;

			w = strlen(name) + 2;

			if (new_width < w)
				new_width = w;
		}

		width[i] = new_width;
	}
}

static void ls_print(size_t count, struct ls_record *records)
{
	int unicode_mode = 0;
	size_t table[2] = { 0 };
	size_t width[LS_WIDTH_COUNT] = { 0 };
	size_t i, j;

	if (count == 0)
		return;

	ls_print_state = 1;

	for (i = 0; i < count; i++) {
		struct ls_record *r = &records[i];

		if (r->unicode) {
			unicode_mode = 1;
			break;
		}
	}

	if (unicode_mode || ls_tty_width < 16) {
		for (i = 0; i < count; i++) {
			struct ls_record *r = &records[i];
			const char *name = &r->path[0];

			if (r->state > 1)
				name = strrchr(name, '/') + 1;

			printf("%s\n", name);
		}
		return;
	}

	for (i = LS_WIDTH_COUNT; i > 0; i--) {
		size_t max_width = 0;
		size_t table2[2] = { 0 };
		size_t width2[LS_WIDTH_COUNT] = { 0 };

		build_table_width(count, records, i, table2, width2);

		for (j = 0; j < table2[0]; j++)
			max_width += width2[j];

		if (max_width + 4 < (size_t)ls_tty_width) {
			memcpy(table, table2, sizeof(table));
			memcpy(width, width2, sizeof(width));
			break;
		}
	}

	if (table[0] == 0)
		build_table_width(count, records, 1, table, width);

	for (i = 0; i < table[1]; i++) {
		struct ls_record *r;
		const char *name;

		for (j = 0; j < table[0]; j++) {
			size_t records_i = i + (j * table[1]);
			int w = (int)width[j];
			const char *c = "\033[0m";

			if (records_i >= count)
				break;

			if (records_i + 1 >= count || j + 1 >= table[0])
				w = 0;

			if (i + ((j + 1) * table[1]) >= count)
				w = 0;

			r = &records[records_i];
			name = &r->path[0];

			if (r->state > 1)
				name = strrchr(name, '/') + 1;

			if (S_ISDIR(r->status.st_mode))
				c = "\033[94m";
			else if (S_ISCHR(r->status.st_mode))
				c = "\033[95m";
			else if (S_ISBLK(r->status.st_mode))
				c = "\033[33m";
			else if (!S_ISREG(r->status.st_mode))
				c = "\033[96m";

			printf("%s%-*s\033[0m", c, w, name);
		}

		printf("\n");
	}
}

static int ls_qsort(const void *p1, const void *p2)
{
	const struct ls_record *r1 = p1;
	const struct ls_record *r2 = p2;
	int r;

	if (r1->state == 0)
		return (r2->state == 0) ? 0 : 1;

	if (r2->state == 0)
		return -1;

	r = strcasecmp(&r1->path[0], &r2->path[0]);

	if (ls_opt->reverse_order && r != 0)
		return (r > 0) ? -1 : 1;

	return r;
}

static void ls(size_t count, struct ls_record *records, int recursion)
{
	size_t print_count = count;
	size_t i;

	if (ls_exit_failure > 1)
		return;

	if (recursion > 128) {
		fputs("ls: deep recursion error\n", stderr);
		ls_exit_failure = 2;
		return;
	}

	if (recursion == 0) {
		for (print_count = 0, i = 0; i < count; i++) {
			struct ls_record *r = &records[i];

			if (S_ISDIR(r->status.st_mode))
				break;
			print_count += 1;
		}
	}

	ls_print(print_count, records);

	if (!ls_opt->recursive && recursion > 0)
		return;

	for (i = 0; i < count; i++) {
		struct ls_record *r = &records[i];
		const char *path = &r->path[0];
		DIR *dir;

		size_t new_count = 0;
		size_t new_count_buffer = 0;
		struct ls_record *new_records = NULL;

		if (ls_exit_failure > 1)
			break;

		if (!S_ISDIR(r->status.st_mode) || S_ISLNK(r->status.st_mode))
			continue;

		if (r->state == 3)
			continue;

		if (ls_opt->recursive || (recursion == 0 && count != 1)) {
			if (ls_print_state != 0)
				fputs("\n", stdout);
			printf("%s:\n", path);
			ls_print_state = 1;
		}

		errno = 0;
		dir = opendir(path);

		if (dir == NULL) {
			char *e = ls_strerror(errno);
			fprintf(stderr, "ls: \'%s\': %s\n", path, e);
			continue;
		}

		for (;;) {
			struct ls_record *nr = new_records;
			const char *p1, *p2;
			struct dirent *d;

			if (new_count_buffer > 0x10000) {
				const char *e = "too many directory entries";
				fprintf(stderr, "ls: \'%s\': %s\n", path, e);
				ls_exit_failure = 1;
				break;
			}

			if (nr == NULL || new_count >= new_count_buffer) {
				size_t old_size, new_size;

				old_size = new_count_buffer * sizeof(*nr);
				new_count_buffer += 128;
				new_size = new_count_buffer * sizeof(*nr);

				new_records = malloc(new_size);

				if (new_records == NULL) {
					fputs("ls: out of memory\n", stderr);
					ls_exit_failure = 2;
					free(nr), nr = NULL;
					break;
				}

				if (nr != NULL) {
					memcpy(new_records, nr, old_size);
					free(nr), nr = NULL;
				}
			}

			errno = 0;
			d = readdir(dir);

			if (d == NULL && errno == 0)
				break;

			if (d == NULL) {
				char *e = ls_strerror(errno);
				fprintf(stderr, "ls: \'%s\': %s\n", path, e);
				ls_exit_failure = 1;
				break;
			}

			p1 = path;
			p2 = &d->d_name[0];

			if (p2[0] == '\0')
				continue;

			if (p2[0] == '.' && !ls_opt->list_all)
				continue;

			read_record(p1, p2, &new_records[new_count++]);
		}

		closedir(dir);

		if (new_count == 0 || new_records == NULL) {
			free(new_records), new_records = NULL;
			continue;
		}

		qsort(new_records, new_count, sizeof(*new_records), ls_qsort);
		verify_records(&new_count, new_records);

		ls(new_count, new_records, recursion + 1);
		free(new_records), new_records = NULL;
	}
}

static int ls_0_qsort(const void *p1, const void *p2)
{
	const struct ls_record *r1 = p1;
	const struct ls_record *r2 = p2;

	int r1_dir = S_ISDIR(r1->status.st_mode);
	int r2_dir = S_ISDIR(r2->status.st_mode);

	if (r1->state == 0)
		return (r2->state == 0) ? 0 : 1;

	if (r2->state == 0)
		return -1;

	if (r1_dir)
		return r2_dir ? ls_qsort(r1, r2) : 1;

	if (r2_dir)
		return -1;

	return ls_qsort(r1, r2);
}

static int ls_0(size_t count, char **operands)
{
	size_t size = count * sizeof(struct ls_record);
	struct ls_record *records = malloc(size);
	size_t i;

	if (records == NULL)
		return fputs("ls: out of memory\n", stderr), EXIT_FAILURE;

	for (i = 0; i < count; i++)
		read_record(operands[i], "", &records[i]);

	qsort(records, count, sizeof(*records), ls_0_qsort);
	verify_records(&count, records);

	ls(count, records, 0);
	free(records);

	return ls_exit_failure;
}

int operate(struct options *opt)
{
	size_t count = 0;
	int i;

	ls_opt = opt;

	if (isatty(STDOUT_FILENO)) {
		struct winsize ws = { 0, 0, 0, 0 };

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

		if (ws.ws_col >= 16 && ws.ws_col <= 4096)
			ls_tty_width = (int)ws.ws_col;
	}

	for (i = 0; opt->operands[i] != NULL; i++)
		count += 1;

	if (count == 0) {
		char dot[2] = { '.', '\0' };
		char *default_operands[2] = { dot, NULL };

		return ls_0(1, &default_operands[0]);
	}

	return ls_0(count, opt->operands);
}
