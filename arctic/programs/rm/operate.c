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
 * rm/operate.c
 *      Remove directory entries
 */

#include "main.h"

static int check_directory(const char *path)
{
	const char *p = path;
	const char *e = strrchr(path, '/');
	int r;

	if (e != NULL)
		p = e;

	if (!strcmp(p, ".") || !strcmp(p, "..")) {
		fprintf(stderr, "rm: refusing to remove \'%s\'\n", p);
		return 1;
	}

	if ((p = realpath(path, NULL)) == NULL)
		return perror("rm"), 1;

	if ((r = !strcmp(p, "/")) != 0)
		fprintf(stderr, "rm: refusing to remove \'%s\'\n", p);

	return free((void *)p), r;
}

static int handle_rm(struct options *opt, const char *path, int recursion)
{
	struct stat status;
	DIR *dir;
	int r = 0;

	if (recursion > 0x1000)
		return 1;

	if (lstat(path, &status)) {
		if (opt->force)
			return 0;
		fprintf(stderr, "rm: \'%s\': %s\n", path, strerror(errno));
		return 1;
	}

	if (!S_ISDIR(status.st_mode)) {
		if (unlink(path) == 0) {
			if (opt->verbose)
				fprintf(stdout, "removed \'%s\'\n", path);
			return 0;
		}
		fprintf(stderr, "rm: \'%s\': %s\n", path, strerror(errno));
		return 1;
	}

	if (!opt->recursive) {
		fprintf(stderr, "rm: \'%s\': Is a directory\n", path);
		return 1;
	}

	if (check_directory(path))
		return opt->force ? 0 : 1;

	dir = opendir(path);

	if (dir == NULL) {
		fprintf(stderr, "rm: \'%s\': %s\n", path, strerror(errno));
		return 1;
	}

	for (;;) {
		struct dirent *d;
		char buffer[0x1000];
		int s;

		errno = 0;
		d = readdir(dir);

		if (d == NULL && errno == 0)
			break;

		if (d == NULL) {
			fprintf(stderr, "rm: \'%s\' (readdir): %s\n",
				path, strerror(errno));
			return closedir(dir), 1;
		}

		if (d->d_name[0] == '\0')
			continue;

		if (!strcmp(&d->d_name[0], "."))
			continue;
		if (!strcmp(&d->d_name[0], ".."))
			continue;

		s = snprintf(&buffer[0], sizeof(buffer),
			"%s/%s", path, d->d_name);

		if (s < 1 || s >= (int)sizeof(buffer) - 1) {
			fprintf(stderr, "rm: \'%s\' (readdir): %s\n",
				path, "path length exceeds limitations");
			return closedir(dir), 1;
		}

		r |= handle_rm(opt, &buffer[0], recursion + 1);
	}

	closedir(dir);

	if (rmdir(path) != 0) {
		fprintf(stderr, "rm: \'%s\': %s\n", path, strerror(errno));
		return 1;
	}

	if (opt->verbose)
		fprintf(stdout, "removed directory \'%s\'\n", path);

	return r;
}

int operate(struct options *opt)
{
	int i, r = 0;

	if (opt->operands[0] == NULL) {
		if (opt->force)
			return 0;
		opt->error = "specify the files";
		return 1;
	}

	for (i = 0; opt->operands[i] != NULL; i++)
		r |= handle_rm(opt, opt->operands[i], 0);

	return r;
}
