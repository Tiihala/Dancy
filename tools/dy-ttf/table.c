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
 * dy-ttf/table.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static unsigned char *ttf_data;
static size_t ttf_size;

static struct {
	unsigned long name;
	unsigned long checksum;
	unsigned char *data;
	size_t size;
} tables[] = {
	{ TTF_TABLE_CMAP, 0, NULL, 4  },
	{ TTF_TABLE_GLYF, 0, NULL, 0  },
	{ TTF_TABLE_HEAD, 0, NULL, 54 },
	{ TTF_TABLE_HHEA, 0, NULL, 36 },
	{ TTF_TABLE_HMTX, 0, NULL, 0  },
	{ TTF_TABLE_LOCA, 0, NULL, 0  },
	{ TTF_TABLE_MAXP, 0, NULL, 6  },
	{ TTF_TABLE_NAME, 0, NULL, 0  },
	{ TTF_TABLE_POST, 0, NULL, 0  }
};

unsigned long table_checksum(const unsigned char *data, size_t size)
{
	unsigned long sum = 0;
	size_t i;

	size = ((size + 3) >> 2);

	for (i = 0; i < size; i++) {
		const unsigned char *p = &data[i * 4];
		sum += BE32(&p[0]);
	}
	return sum & 0xFFFFFFFFul;
}

int table_find(unsigned long name, unsigned char **table, size_t *size)
{
	size_t i, count = sizeof(tables) / sizeof(tables[0]);

	for (i = 0; i < count; i++) {
		if (tables[i].name == name) {
			*table = tables[i].data, *size = tables[i].size;
			return (*table == NULL || *size == 0) ? 1 : 0;
		}
	}
	return *table = NULL, *size = 0, 2;
}

int table_init(unsigned char *data, size_t size)
{
	size_t i, j, count = sizeof(tables) / sizeof(tables[0]);
	unsigned long ntab, val;

	ttf_data = data;
	ttf_size = size;

	if (ttf_size < 12)
		return 1;

	/*
	 * Test the "signature", also known as "scaler type".
	 */
	val = BE32(&data[0]);
	if (val != 0x74727565ul && val != 0x00010000ul)
		return 1;

	/*
	 * Get the number of tables and check other "header" values.
	 */
	ntab = BE16(&data[4]);
	if (ntab == 0)
		return (ttf_size == 12) ? 0 : 1;

	val = BE16(&data[6]);
	if (val != ((unsigned long)table_power_of_two((unsigned)ntab) * 16))
		return 1;

	val = BE16(&data[8]);
	if (val != table_log2(table_power_of_two((unsigned)ntab)))
		return 1;

	val = BE16(&data[10]);
	if (val != (((unsigned long)ntab * 16) - BE16(&data[6])))
		return 1;

	/*
	 * Check the table directory size.
	 */
	if (size < ((ntab * 16) + 12))
		return 1;

	/*
	 * Read the table directory.
	 */
	for (i = 0; i < ntab; i++) {
		const unsigned char *p = &ttf_data[i * 16 + 12];
		unsigned long name = BE32(&p[0]);

		for (j = 0; j < count; j++) {
			if (tables[j].name == name) {
				unsigned long sum = BE32(&p[4]);
				unsigned long off = BE32(&p[8]);
				unsigned long len = BE32(&p[12]);

				if (!(off < ULONG_MAX - len))
					return 2;
				if (off + len > ttf_size)
					return 2;

				tables[j].checksum = sum;
				tables[j].data = ttf_data + off;

				if (tables[j].size > len)
					return 2;

				tables[j].size = len;
				break;
			}
		}
	}

	/*
	 * Verify the checksums.
	 */
	for (i = 0; i < count; i++) {
		unsigned long name = tables[i].name;
		unsigned long sum = tables[i].checksum;
		int err = 0;

		if (name == TTF_TABLE_HEAD) {
			unsigned char *p = tables[i].data;
			unsigned long head_sum, file_sum, test_sum;

			if (tables[i].size < 16)
				return 3;

			head_sum = BE32(&p[8]);
			W_BE32(&p[8], 0);

			file_sum = table_checksum(ttf_data, ttf_size);
			test_sum = (0xB1B0AFBAul - file_sum) & 0xFFFFFFFFul;

			if (head_sum != test_sum)
				err = 4;
			if (table_checksum(&p[0], tables[i].size) != sum)
				err = 5;

			W_BE32(&p[8], head_sum);
		} else {
			unsigned char *p = tables[i].data;

			if (table_checksum(&p[0], tables[i].size) != sum)
				err = 5;
		}

		if (err != 0)
			return err;
	}

	return 0;
}

unsigned table_log2(unsigned num)
{
	unsigned shift = 0;

	while (num > 1) {
		shift += 1;
		num >>= 1;
	}
	return shift;
}

unsigned table_power_of_two(unsigned num)
{
	unsigned power_of_two = 1;
	unsigned next = 2;

	while (next != 0 && next <= num) {
		power_of_two = next;
		next <<= 1;
	}
	return power_of_two;
}
