/*
 * Copyright (c) 2023, 2024 Antti Tiihala
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
 * base/ret_user.c
 *      Handling user space signals
 */

#include <dancy.h>

void ret_user_handler(struct task *current, void *stack)
{
	uint32_t asm_data3 = (uint32_t)current->asm_data3;
	int sig = 0;
	int i;

	if (asm_data3 != 0) {
		void *address = &current->asm_data3;
		uint32_t mask = current->sig.mask;

		mask &= ~(((uint32_t)1) << (SIGKILL - 1));
		mask &= ~(((uint32_t)1) << (SIGTERM - 1));

		if ((asm_data3 & (~mask)) == 0)
			return;

		for (i = 0; sig == 0 && i < 31; i++) {
			if (cpu_btr32(address, (uint32_t)i))
				sig = i + 1;
		}
	}

	if (sig != 0) {
		cpu_native_t *ip = ((cpu_native_t *)stack) + 0;
		cpu_native_t *sp = ((cpu_native_t *)stack) + 3;

		*ip = 0;
		*sp = 0;

		task_exit(sig);
	}
}
