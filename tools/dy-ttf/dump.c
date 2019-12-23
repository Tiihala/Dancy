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
 * dy-ttf/dump.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

int dump(unsigned char *data, size_t size)
{
	unsigned char *table;
	unsigned long val;
	int i;

	printf("file_size:      %u\n", (unsigned)size);
	printf("tables:         %lu\n\n", BE16(&data[4]));

	/*
	 * Table 'head'
	 */
	if (!table_find(TTF_TABLE_HEAD, &table, &size)) {
		printf("head_size:      %u\n", (unsigned)size);
		printf("head_version:   %08lX\n", BE32(&table[0]));
		printf("revision:       %08lX\n", BE32(&table[4]));
		printf("checksum:       %08lX\n", BE32(&table[8]));
		printf("magic:          %08lX\n", BE32(&table[12]));

		printf("\n%-16s", "flags:");
		val = BE16(&table[16]);

		for (i = 15; i >= 0; i--) {
			if (((val >> i) & 1) == 1)
				printf("1");
			else
				printf("0");
			if ((i % 4) == 0)
				printf("%s", (i == 0) ? "\n" : "_");
		}

		printf("units_per_em:   %lu\n\n", BE16(&table[18]));

		val = BE16(&table[36]);
		printf("x_min:          %ld\n", BE16_TO_LONG(val));

		val = BE16(&table[38]);
		printf("y_min:          %ld\n", BE16_TO_LONG(val));

		val = BE16(&table[40]);
		printf("x_max:          %ld\n", BE16_TO_LONG(val));

		val = BE16(&table[42]);
		printf("y_max:          %ld\n", BE16_TO_LONG(val));

		printf("\n%-16s", "style:");
		val = BE16(&table[44]);

		for (i = 15; i >= 0; i--) {
			if (((val >> i) & 1) == 1)
				printf("1");
			else
				printf("0");
			if ((i % 4) == 0)
				printf("%s", (i == 0) ? "\n" : "_");
		}

		printf("lowest:         %lu\n", BE16(&table[46]));

		val = BE16(&table[48]);
		printf("direction:      %ld\n", BE16_TO_LONG(val));

		val = BE16(&table[50]);
		printf("index_format:   %ld\n", BE16_TO_LONG(val));

		val = BE16(&table[52]);
		printf("data_format:    %ld\n", BE16_TO_LONG(val));
	}

	return fputs("\n", stdout), 0;
}
