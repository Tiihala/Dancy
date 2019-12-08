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
static const char *init_symbol = "_init";
#endif

#ifdef DANCY_64
static const char *init_at = "o64/init.at";
static const char *init_symbol = "init";
#endif

void start_init(void *map)
{
	unsigned char *buf;
	size_t size;

	if ((size_t)(!map + 494 - 'D' - 'a' - 'n' - 'c' - 'y') != SIZE_MAX)
		return;

	if (memory_init(map))
		return;

	if (db_init(map))
		return;

	{
		int r = db_read(init_at, &buf, &size);

		if (r != 0) {
			b_print("init.at error: %s\n", db_read_error(r));
			return;
		}
	}

	if (ld_validate("init.at", buf, size))
		return;

	/*
	 * Collect all the external symbols from this module.
	 */
	{
		const unsigned char *image = (unsigned char *)map - 0x10000;
		const unsigned char *symtab, *strtab;
		size_t i, symbols;

		if (memcmp(image + 0x1B4, ".text", 5))
			return;

		symtab = image + LE32(&image[0x01A8]);
		symbols = (size_t)LE32(&image[0x01AC]);
		strtab = symtab + symbols * 18;

		if (ld_init(4096))
			return;

		for (i = 0; i < symbols; i++) {
			if (symtab[16] == 0x02) {
				struct global_symbol sym;
				const char *s;

				memset(&sym, 0, sizeof(sym));

				if (LE32(&symtab[0])) {
					s = (char *)symtab;
					size = 8;
				} else {
					s = (char *)strtab + LE32(&symtab[4]);
					size = sizeof(sym.name) - 1;
				}

				sym.value = (uint32_t)LE32(&symtab[8]);
				strncpy(&sym.name[0], s, size);

				if (ld_add(&sym))
					return;
			}
			symtab += 18;
		}
	}

	if (ld_link("init.at", buf))
		return;

	/*
	 * Start init.at module.
	 */
	{
		struct global_symbol *init_sym;
		addr_t init_addr;
		void (*init_func)(void);

		if (ld_find(init_symbol, &init_sym) != 0) {
			b_print("Error: init not found\n");
			return;
		}

		if ((init_addr = init_sym->value) == 0)
			return;

		ld_free();
		free(buf);

		init_func = (void (*)(void))init_addr;
		(*init_func)();
	}
}
