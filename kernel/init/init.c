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

void init(void *map)
{
	const uint32_t req_mem = 0x00100000;
	const uint32_t log_mem = 0x00080000;

	if ((size_t)(!map + 494 - 'D' - 'a' - 'n' - 'c' - 'y') != SIZE_MAX)
		return;

	if (memory_init(map, req_mem) || log_init(log_mem))
		return;

	memory_print_map(log);

	if (cpu_test_features())
		return;

	if (!acpi_get_information()) {
		const char *m = "Info: ACPI is not supported/detected";
		b_print("%s\n", m), log("%s\n", m);
	}

	/*
	 * Temporary code for testing purposes.
	 */
	{
		const char *log_data = log_get_data();
		size_t log_size = log_get_size();

		b_output_string(log_data, (unsigned)log_size);

		for (;;) {
			unsigned long key = b_get_keycode();
			if (key)
				b_print("Key: %08lX\n", key);
		}
	}
}
