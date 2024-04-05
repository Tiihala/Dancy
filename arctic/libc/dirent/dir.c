/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/dirent/dir.c
 *      The directory functions
 */

#include <__dancy/syscall.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct internal_data {
	struct dirent entry;
	char *buffer[1024];
};

static DIR *internal_opendir(int fd)
{
	DIR *d;

	if ((d = calloc(1, sizeof(*d))) == NULL)
		return NULL;

	d->_state = 0;
	d->_fd = fd;

	if ((d->_data = calloc(1, sizeof(struct internal_data))) == NULL)
		return free(d), NULL;

	return d;
}

DIR *opendir(const char *name)
{
	int fd = open(name, O_RDONLY | O_DIRECTORY);
	DIR *d;

	if (fd == -1)
		return NULL;

	if ((d = internal_opendir(fd)) == NULL)
		close(fd);

	return d;
}

DIR *fdopendir(int fd)
{
	if (fd < 0 || fd >= __DANCY_OPEN_MAX)
		return (errno = EBADF), NULL;

	return internal_opendir(fd);
}

int closedir(DIR *dirp)
{
	if (dirp->_state < 0)
		return (errno = EBADF), -1;

	close(dirp->_fd);
	dirp->_state = -1;
	dirp->_fd = -1;

	free(dirp->_data);
	dirp->_data = NULL;

	return free(dirp), 0;
}

struct dirent *readdir(DIR *dirp)
{
	struct internal_data *data = dirp->_data;
	int fd = dirp->_fd;
	int r;

	while (fd >= 0) {
		if (dirp->_state > 0) {
			int i = dirp->_state - 1;
			const char *src = data->buffer[i];

			if (src != NULL) {
				strncpy(&data->entry.d_name[0], src, 255);
				dirp->_state += 1;
				return &data->entry;
			}

			dirp->_state = 0;
		}

		r = (int)__dancy_syscall5(__dancy_syscall_getdents,
			fd, data->buffer, sizeof(data->buffer), 256, 0);

		if (r == -EOVERFLOW) {
			r = (int)__dancy_syscall5(__dancy_syscall_getdents,
				fd, data->buffer, sizeof(data->buffer), 4, 0);
		}

		if (r == 0)
			return NULL;

		if (r < 0)
			return (errno = -r), NULL;

		dirp->_state = 1;
	}

	return (errno = EBADF), NULL;
}
