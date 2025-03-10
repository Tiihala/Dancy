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
 * libc/stdio/perror.c
 *      Write error messages to standard error stream
 */

#include <__dancy/syscall.h>
#include <errno.h>
#include <stdio.h>

void perror(const char *s)
{
	char buffer[64];
	int number = errno;
	char *e = &buffer[0];

	if (__dancy_syscall3(__dancy_syscall_errno, number, e, 1))
		e[0] = '\0';

	if (s != NULL && s[0] != '\0')
		fprintf(stderr, "%s: %s\n", s, e);
	else
		fprintf(stderr, "%s\n", e);
}
