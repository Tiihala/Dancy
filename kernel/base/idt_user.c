/*
 * Copyright (c) 2023, 2026 Antti Tiihala
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

static int handle_get_tls(cpu_native_t *ip, cpu_native_t *ax)
{
	void *e = task_current()->tls._e;

#if DANCY_32
	unsigned char gs_zero_to_eax[6] = {
		0x65, 0xA1, 0x00, 0x00, 0x00, 0x00
	};
	const void *mov = &gs_zero_to_eax[0];
	size_t size = sizeof(gs_zero_to_eax);
#endif

#if DANCY_64
	unsigned char fs_zero_to_rax[9] = {
		0x64, 0x48, 0x8B, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00
	};
	const void *mov = &fs_zero_to_rax[0];
	size_t size = sizeof(fs_zero_to_rax);
#endif

	if (pg_check_user_read((const void *)(*ip), size))
		return -1;

	if (memcmp((const void *)(*ip), mov, size))
		return -1;

	if (e == NULL || pg_check_user_read(e, sizeof(cpu_native_t)))
		return -1;

	*ip += ((cpu_native_t)size);
	memcpy(ax, e, sizeof(cpu_native_t));

	return 0;
}

int idt_user_exception(int num, void *stack)
{
	cpu_native_t *p = stack;

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
	if (num == 13) {
		if (!handle_get_tls(&p[0], &p[-2]))
			return 0;

		task_exit(SIGILL);
	}

	/*
	 * Page-Fault Exception
	 */
	if (num == 14) {
		const cpu_native_t stack_min = 0x78000000;
		const cpu_native_t stack_max = 0x7FFFFFFF;

		cpu_native_t code = p[-1];
		cpu_native_t cr2 = cpu_read_cr2();

		if ((code & 1) == 0 && cr2 >= stack_min && cr2 <= stack_max) {
			if (pg_map_user((addr_t)cr2, 1, pg_noexec))
				return 0;
		}

		if (!handle_get_tls(&p[0], &p[-2]))
			return 0;

		printk("[PROCESS] ID %llu, IP %08llX, "
			"CR2 %08llX, Page-Fault Exception\n",
			(unsigned long long)task_current()->id,
			(unsigned long long)p[0], (unsigned long long)cr2);

		task_exit(SIGSEGV);
	}

	return 1;
}
