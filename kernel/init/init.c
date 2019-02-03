/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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
 * init/init.c
 *      Initialization of Dancy Operating System
 */

#include <dancy.h>

static void print_memory_map(struct b_mem *mem)
{
	phys_addr_t total = 0;
	size_t i;

	b_print("Memory Map\n");

	for (i = 0; i == 0 || mem[i].base; i++) {
		uint32_t t = mem[i].type;
		phys_addr_t b = mem[i].base;
		phys_addr_t e = mem[i + 1].base - 1;
		const char *desc = "Not Reported";

		if (t == B_MEM_NORMAL)
			desc = "Free", total += mem[i + 1].base - b;
		else if (t == B_MEM_RESERVED)
			desc = "Reserved";
		else if (t == B_MEM_ACPI_RECLAIMABLE)
			desc = "Acpi Reclaimable";
		else if (t == B_MEM_ACPI_NVS)
			desc = "Acpi NVS";
		else if (t == B_MEM_BOOT_LOADER)
			desc = "Loader Runtime (Reclaimable)";
		else if (t == B_MEM_INIT_EXECUTABLE)
			desc = "Executable (Init)";

		if (t >= B_MEM_DATABASE_MIN && t <= B_MEM_DATABASE_MAX) {
			unsigned db = (unsigned)t & 0x0000FFFFu;
			b_print("  %p %p  Database (#%u)\n", b, e, db);
		} else {
			b_print("  %p %p  %s\n", b, e, desc);
		}
	}
	b_print("\n  Total free: %zd KiB\n\n", total / 1024);
}

static void sleep(int seconds)
{
	int i;

	/*
	 * This "sleep" function is not accurate. The pause
	 * function waits approximately 500 milliseconds.
	 */
	for (i = 0; i < (seconds * 2); i++)
		b_pause();
}

void init(void *map)
{
	print_memory_map(map);

	for (;;) {
		const char *cursor = "\r ";
		b_output_string_hl(cursor, 0);
		sleep(1);
		b_output_string(cursor, 0);
		sleep(1);
	}
}
