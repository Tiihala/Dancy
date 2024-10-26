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
 * cp/operate.c
 *      Copy files and directories
 */

#include "main.h"

static int read_write(int fd[2], const char *source, const char *destination)
{
	static unsigned char buffer[0x20000];
	ssize_t size[2];

	for (;;) {
		size[0] = read(fd[0], &buffer[0], sizeof(buffer));
		size[1] = 0;

		if (size[0] < 0) {
			fprintf(stderr,
				"cp: cannot read \'%s\': %s\n",
				source, strerror(errno));
			return 1;
		}

		if (size[0] == 0)
			break;

		while (size[0] > size[1]) {
			size_t s = (size_t)(size[0] - size[1]);
			ssize_t w;

			w = (errno = EIO, write(fd[1], &buffer[size[1]], s));

			if (w <= 0) {
				fprintf(stderr,
					"cp: cannot write \'%s\': %s\n",
					destination, strerror(errno));
				return 1;
			}

			size[1] += w;
		}
	}

	return 0;
}

static int copy(struct options *opt, struct stat *destination_status,
	const char *source, const char *destination, int recursion)
{
	struct stat status;
	DIR *dir;
	int r = 0;

	if (recursion > 128) {
		fputs("cp: deep recursion error\n", stderr);
		return 1;
	}

	if (lstat(source, &status)) {
		fprintf(stderr,
			"cp: cannot stat \'%s\': %s\n",
			source, strerror(errno));
		return 1;
	}

	if (S_ISLNK(status.st_mode)) {
		fprintf(stderr,
			"cp: source \'%s\' is a symbolic link\n",
			source);
		return 1;
	}

	if (!S_ISDIR(status.st_mode)) {
		mode_t mode = (status.st_mode & 0777);
		int fd[2];

		if (!S_ISREG(status.st_mode)) {
			fprintf(stderr,
				"cp: source \'%s\' is not a regular file\n",
				source);
			return 1;
		}

		fd[0] = open(source, O_RDONLY);

		if (fd[0] < 0) {
			fprintf(stderr,
				"cp: cannot open source \'%s\': %s\n",
				source, strerror(errno));
			return 1;
		}

		fd[1] = open(destination, O_WRONLY | O_CREAT, mode);

		if (fd[1] < 0) {
			fprintf(stderr,
				"cp: cannot open destination \'%s\': %s\n",
				destination, strerror(errno));
			return close(fd[0]), 1;
		}

		r = read_write(fd, source, destination);

		close(fd[1]);
		close(fd[0]);

		if (r != 0)
			unlink(destination);
		else if (opt->verbose)
			printf("\'%s\' -> \'%s\'\n", source, destination);

		return r;
	}

	if (!opt->recursive) {
		fprintf(stderr,
			"cp: omitting directory \'%s\'\n",
			source);
		return 1;
	}

	if (destination_status != NULL) {
		if (!S_ISDIR(destination_status->st_mode)) {
			fprintf(stderr,
				"cp: cannot overwrite \'%s\'\n",
				destination);
			return 1;
		}
	}

	if ((dir = opendir(source)) == NULL) {
		fprintf(stderr,
			"cp: cannot open \'%s\': %s\n",
			source, strerror(errno));
		return 1;
	}

	if (mkdir(destination, 0777) != 0) {
		if (errno != EEXIST) {
			fprintf(stderr,
				"cp: cannot create directory \'%s\': %s\n",
				destination, strerror(errno));
			return closedir(dir), 1;
		}

	} else if (opt->verbose) {
		printf("\'%s\' -> \'%s\'\n", source, destination);
	}

	for (;;) {
		struct dirent *d;
		char buffer1[0x1000];
		char buffer2[0x1000];
		int s;

		errno = 0;
		d = readdir(dir);

		if (d == NULL && errno == 0)
			break;

		if (d == NULL) {
			fprintf(stderr, "cp: \'%s\' (readdir): %s\n",
				source, strerror(errno));
			return closedir(dir), 1;
		}

		if (d->d_name[0] == '\0')
			continue;

		if (!strcmp(&d->d_name[0], "."))
			continue;
		if (!strcmp(&d->d_name[0], ".."))
			continue;

		s = snprintf(&buffer1[0], sizeof(buffer1),
			"%s/%s", source, d->d_name);

		if (s < 1 || s >= (int)sizeof(buffer1) - 1) {
			fputs("cp: path length too long\n", stderr);
			return closedir(dir), 1;
		}

		s = snprintf(&buffer2[0], sizeof(buffer2),
			"%s/%s", destination, d->d_name);

		if (s < 1 || s >= (int)sizeof(buffer2) - 1) {
			fputs("cp: path length too long\n", stderr);
			return closedir(dir), 1;
		}

		r |= copy(opt, NULL, &buffer1[0], &buffer2[0], recursion + 1);
	}

	return closedir(dir), r;
}

int operate(struct options *opt)
{
	const char *destination;
	struct stat status;
	int destination_exists = 1;
	int i, r = 0, count = 0;

	for (i = 0; opt->operands[i] != NULL; i++)
		count += 1;

	if (count == 0) {
		opt->error = "specify the source";
		return 1;
	}

	if (count == 1) {
		opt->error = "specify the destination";
		return 1;
	}

	if (*(destination = opt->operands[count - 1]) == '\0') {
		opt->error = "specify the destination";
		return 1;
	}

	if (lstat(destination, &status)) {
		const char *e;

		if (errno != ENOENT) {
			fprintf(stderr,
				"cp: cannot stat \'%s\': %s\n",
				destination, strerror(errno));
			return 1;
		}

		if ((e = strrchr(destination, '/')) == NULL)
			e = destination;

		memset(&status, 0, sizeof(status));

		status.st_mode = (mode_t)S_IFREG;

		if (!strcmp(e, "/") || !strcmp(e, "/.") || !strcmp(e, "/.."))
			status.st_mode = (mode_t)S_IFDIR;

		destination_exists = 0;
	}

	if (S_ISLNK(status.st_mode)) {
		fprintf(stderr,
			"cp: destination \'%s\' is a symbolic link\n",
			destination);
		return 1;
	}

	if (destination_exists && S_ISDIR(status.st_mode)) {
		char tmp[0x1000];
		for (i = 0; i < count - 1; i++) {
			const char *source = strrchr(opt->operands[i], '/');
			size_t s1, s2;

			if (source == NULL)
				source = opt->operands[i];

			s1 = strlen(source);
			s2 = strlen(destination);

			if (s1 == 0 || s2 == 0)
				continue;

			if (s1 + s2 >= sizeof(tmp) - 4) {
				fputs("cp: path length too long\n", stderr);
				r = 1;
				continue;
			}

			strcpy(&tmp[0], destination);

			if (source[0] != '/' && destination[s2 - 1] != '/')
				strcat(&tmp[0], "/");

			strcat(&tmp[0], source);

			r |= copy(opt, &status, opt->operands[i], &tmp[0], 0);
		}
		return r;
	}

	if (count != 2) {
		fprintf(stderr,
			"cp: destination \'%s\' is not a directory\n",
			destination);
		return 1;
	}

	if (!S_ISREG(status.st_mode)) {
		fprintf(stderr,
			"cp: destination \'%s\' is not a regular file\n",
			destination);
		return 1;
	}

	if (destination_exists)
		return copy(opt, &status, opt->operands[0], destination, 0);

	return copy(opt, NULL, opt->operands[0], destination, 0);
}
