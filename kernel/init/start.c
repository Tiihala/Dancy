/*
 * Copyright (c) 2019 Antti Tiihala
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
 * init/start.c
 *      Load init.at from system databases
 */

#include <init.h>

#ifdef DANCY_32
static const char *init_at = "o32/init.at";
#endif

#ifdef DANCY_64
static const char *init_at = "o64/init.at";
#endif

void start_init(void *map)
{
	unsigned char *buf;
	size_t size;
	int r;

	if ((size_t)(!map + 494 - 'D' - 'a' - 'n' - 'c' - 'y') != SIZE_MAX)
		return;

	if (memory_init(map))
		return;

	if (db_init(map))
		return;

	r = db_read(init_at, &buf, &size);

	if (r == 1) {
		b_print("Error: could not found init.at\n");
		return;
	}
	if (r == 2) {
		b_print("Error: could not allocate memory for init.at\n");
		return;
	}
	if (r == 3) {
		b_print("Error: could not read init.at\n");
		return;
	}
	if (r == 4) {
		b_print("Error: CRC-32 of init.at\n");
		return;
	}

	b_print("Found init.at module, %u bytes\n\n", (unsigned)size);

	if (ld_validate("init.at", buf, size))
		return;

	/*
	 * Temporary code for testing purposes.
	 */
	{
		b_print("start_init is not implemented\n");
	}

	while (!b_pause()) { }
}
