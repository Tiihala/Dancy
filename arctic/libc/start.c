/*
 * Copyright (c) 2022, 2023, 2024 Antti Tiihala
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
 * arctic/libc/start.c
 *      Start function of the C library
 */

#include <__dancy/core.h>
#include <__dancy/syscall.h>
#include <stdio.h>
#include <stdlib.h>

void __dancy_libc_start(int argc, char *argv[]);

void __dancy_libc_start(int argc, char *argv[])
{
	extern int main(int argc, char *argv[]);
	int retval;

	__dancy_free = __dancy_free_default;
	__dancy_stdio_init();
	__dancy_atexit_init();

	retval = main(argc, argv);

	__dancy_atexit_fini();
	__dancy_stdio_fini();

	__dancy_syscall1(__dancy_syscall_exit, retval);
}

void (*__dancy_free)(void *pointer);

void __dancy_free_default(void *pointer)
{
	(void)pointer;
}
