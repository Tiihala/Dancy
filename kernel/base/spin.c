/*
 * Copyright (c) 2025 Antti Tiihala
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
 * base/spin.c
 *      The spin_lock_yield function
 */

#include <dancy.h>

void spin_lock_yield(int *lock)
{
	unsigned int i = 0;

	if ((cpu_read_flags() & CPU_INTERRUPT_FLAG) == 0)
		kernel->panic("spin_lock_yield: interrupts disabled");

	while (!spin_trylock(lock)) {
		task_yield();

		if (i < 1000) {
			i += 1;
			continue;
		}

		cpu_halt(1);

		if (i < 10000) {
			const char *warning = "Spin Deadlock Warning";

			if ((i += 1) < 10000)
				continue;

			printk("[KERNEL] %s, PID %llu, Address %p\n", warning,
				(unsigned long long)task_current()->id, lock);
		}
	}
}
