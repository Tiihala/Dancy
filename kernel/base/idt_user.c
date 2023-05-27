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
 * base/idt_user.c
 *      Handling user space exceptions
 */

#include <dancy.h>

int idt_user_exception(int num, void *stack)
{
	const cpu_native_t *p = stack;

	/*
	 * Page-Fault Exception
	 */
	if (num == 14) {
		const cpu_native_t stack_min = 0x7FC00000;
		const cpu_native_t stack_max = 0x7FFFFFFF;

		cpu_native_t code = p[-1];
		cpu_native_t cr2 = cpu_read_cr2();

		if ((code & 1) == 0 && cr2 >= stack_min && cr2 <= stack_max) {
			if (pg_map_user((addr_t)cr2, 1))
				return 0;
		}

		task_exit(SIGSEGV);
	}

	return 1;
}
