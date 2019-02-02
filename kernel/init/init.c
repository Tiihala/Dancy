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
	char buf[128];
	size_t i;

	b_output_string("Memory Map\r\n", 0);

	for (i = 0; i == 0 || mem[i].base; i++) {
		uint32_t t = mem[i].type;
		phys_addr_t b = mem[i].base;
		phys_addr_t s = mem[i + 1].base - b;

		snprintf(buf, 128, "  %p %p  ", b, mem[i + 1].base - 1);
		b_output_string(buf, 0);

		if (t == B_MEM_NORMAL)
			b_output_string("Free", 0), total += s;
		else if (t== B_MEM_RESERVED)
			b_output_string("Reserved", 0);
		else if (t == B_MEM_ACPI_RECLAIMABLE)
			b_output_string("Acpi Reclaimable", 0);
		else if (t == B_MEM_ACPI_NVS)
			b_output_string("Acpi NVS", 0);
		else if (t == B_MEM_BOOT_LOADER)
			b_output_string("Loader Runtime (Reclaimable)", 0);
		else if (t == B_MEM_INIT_EXECUTABLE)
			b_output_string("Executable (Init)", 0);
		else if (t >= B_MEM_DATABASE_MIN && t <= B_MEM_DATABASE_MAX)
			b_output_string("Database", 0);
		else
			b_output_string("Not Reported", 0);
		b_output_string("\r\n", 2);
	}

	snprintf(buf, 128, "\r\n  Total free: %zd KiB\r\n\n", total / 1024);
	b_output_string(buf, 0);
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
