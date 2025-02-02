/*
 * Copyright (c) 2023, 2025 Antti Tiihala
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
 * libc/stdio/rename.c
 *      Rename a file or directory
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static int read_write(int fd[2])
{
	unsigned char buffer[0x2000];
	ssize_t size[2];

	for (;;) {
		size[0] = read(fd[0], &buffer[0], sizeof(buffer));
		size[1] = 0;

		if (size[0] < 0)
			return -1;

		if (size[0] == 0)
			break;

		while (size[0] > size[1]) {
			size_t s = (size_t)(size[0] - size[1]);
			ssize_t w;

			w = (errno = EIO, write(fd[1], &buffer[size[1]], s));

			if (w <= 0)
				return -1;

			size[1] += w;
		}
	}

	return 0;
}

int rename(const char *old_path, const char *new_path)
{
	int r;

	r = (int)__dancy_syscall2(__dancy_syscall_rename, old_path, new_path);

	if (r == -EXDEV) {
		int saved_errno = errno;
		struct stat status;
		int fd[2];

		if (lstat(old_path, &status))
			return -1;

		if (!S_ISREG(status.st_mode))
			return (errno = EXDEV), -1;

		fd[0] = open(old_path, O_RDONLY);

		if (fd[0] < 0)
			return -1;

		fd[1] = open(new_path, O_WRONLY | O_CREAT | O_EXCL, 0777);

		if (fd[1] < 0) {
			if ((r = errno) == EBUSY)
				r = EEXIST;
			close(fd[0]);
			return (errno = r), -1;
		}

		r = read_write(fd);

		close(fd[1]);
		close(fd[0]);

		if (r < 0)
			return -1;

		r = (int)__dancy_syscall1(__dancy_syscall_unlink, old_path);
		errno = saved_errno;
	}

	if (r < 0)
		errno = -r, r = -1;

	return r;
}
