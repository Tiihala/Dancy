/*
 * Copyright (c) 2022 Antti Tiihala
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
 * syscall/syscall.c
 *      System calls
 */

#include <dancy.h>
#include <errno.h>

static long long dancy_syscall_exit(va_list va)
{
	int retval = va_arg(va, int);

	task_exit((int)((unsigned int)retval & 0xFF));

	return kernel->panic("__dancy_syscall_exit: unexpected behavior"), 0;
}

static long long dancy_syscall_reserved(va_list va)
{
	return (void)va, -EINVAL;
}

static struct { long long (*handler)(va_list va); } handler_array[] = {
	{ dancy_syscall_exit },
	{ dancy_syscall_reserved }
};

int syscall_init(void)
{
	static int run_once;
	extern void syscall_init_asm(void);
	uint32_t t0, t1, t2, t3;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	syscall_init_asm();

	t0 = (uint32_t)(sizeof( handler_array));
	t1 = (uint32_t)(sizeof(*handler_array));
	t2 = (uint32_t)(__dancy_syscall_arg0__);
	t3 = (uint32_t)(__dancy_syscall_argn__);

	if (cpu_read32(&t0) / t1 != t3 - t2)
		return DE_UNEXPECTED;

	return 0;
}

long long syscall_handler(int arg0, ...)
{
	long long r = -EINVAL;

	va_list va;
	va_start(va, arg0);

	if (arg0 > __dancy_syscall_arg0__ && arg0 < __dancy_syscall_argn__) {
		int i = arg0 - __dancy_syscall_arg0__ - 1;
		r = handler_array[i].handler(va);
	}

	va_end(va);

	return r;
}
