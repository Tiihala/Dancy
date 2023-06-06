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
	 * Divide-by-Zero Exception
	 */
	if (num == 0)
		task_exit(SIGFPE);

	/*
	 * Debug Exception
	 */
	if (num == 1)
		task_exit(SIGILL);

	/*
	 * Non-Maskable-Interrupt Exception
	 */
	if (num == 2)
		return 1;

	/*
	 * Breakpoint Exception
	 */
	if (num == 3)
		task_exit(SIGILL);

	/*
	 * Overflow Exception
	 */
	if (num == 4)
		task_exit(SIGFPE);

	/*
	 * Bound-Range Exception
	 */
	if (num == 5)
		task_exit(SIGSEGV);

	/*
	 * Invalid-Opcode Exception
	 */
	if (num == 6)
		task_exit(SIGILL);

	/*
	 * Device-Not-Available Exception
	 */
	if (num == 7)
		task_exit(SIGFPE);

	/*
	 * Double-Fault Exception
	 */
	if (num == 8)
		return 1;

	/*
	 * Coprocessor-Segment-Overrun Exception
	 */
	if (num == 9)
		task_exit(SIGFPE);

	/*
	 * Invalid-TSS Exception
	 */
	if (num == 10)
		task_exit(SIGILL);

	/*
	 * Segment-Not-Present Exception
	 */
	if (num == 11)
		task_exit(SIGILL);

	/*
	 * Stack Exception
	 */
	if (num == 12)
		task_exit(SIGSEGV);

	/*
	 * General-Protection Exception
	 */
	if (num == 13)
		task_exit(SIGILL);

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
