/*
 * Copyright (c) 2021 Antti Tiihala
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
 * base/start.c
 *      Kernel start functions
 */

#include <dancy.h>

#ifdef DANCY_32
struct kernel_table _dancy_kernel_table;
struct kernel_table *kernel = &_dancy_kernel_table;
#endif

#ifdef DANCY_64
struct kernel_table __dancy_kernel_table;
struct kernel_table *kernel = &__dancy_kernel_table;
#endif

void kernel_start(void)
{
	cpu_halt(0);
}

void kernel_start_ap(void)
{
	cpu_halt(0);
}
