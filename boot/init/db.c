/*
 * Copyright (c) 2019, 2022 Antti Tiihala
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
 * init/db.c
 *      Handle database files
 */

#include <boot/init.h>

static int db_count;
static void **db_array;
static int db_callback_state;

static struct b_mem *db_map;
static size_t db_offset;

int db_init(struct b_mem *map)
{
	const char *err = "Error: boot loader gave a corrupted memory map";
	int i, c;

	db_count = 0;
	db_array = calloc(1000, sizeof(void *));
	if (!db_array)
		return 1;

	for (i = 0; (i == 0 || map[i].base); i++) {
		uint32_t t = map[i].type;
		unsigned char *m;

		if (t < B_MEM_DATABASE_MIN || t > (B_MEM_DATABASE_MIN + 999))
			continue;

		t -= B_MEM_DATABASE_MIN;

		if (db_count < (int)(t + 1))
			db_count = (int)(t + 1);

		db_array[t] = m = (unsigned char *)map[i].base;

		if (LE32(&m[0]) != 0x0D54418D || LE32(&m[28]) != t) {
			b_print("%s\n", err);
			return free(db_array), 1;
		}
	}

	for (i = 0, c = 0; i < db_count; i++) {
		if (db_array[i]) {
			db_array[c] = db_array[i];
			if ((c++) != i)
				db_array[i] = NULL;
		}
	}

	db_count = c;
	db_map = map;

	return 0;
}

void db_free(void)
{
	free(db_array);

	db_count = 0;
	db_array = NULL;
	db_map = NULL;
}

static int db_callback(struct bitarray *b)
{
	int q = (int)(db_offset / 65504);
	int r = (int)(db_offset % 65504);

	if (q >= db_count)
		return -1;

	if (db_callback_state == 1 && q > 0) {
		unsigned char *m1 = db_array[q - 1];
		unsigned char *m2 = db_array[q - 0];

		if ((LE32(&m1[28]) + 1) != LE32(&m2[28]))
			db_callback_state = -1;
	}

	if (db_callback_state < 0)
		return -1;

	b->data = (unsigned char *)db_array[q] + 32 + r;
	b->size = (size_t)(65504 - r);

	db_offset += b->size;
	return 0;
}

static void db_fast_forward(struct bitarray *b, size_t bytes)
{
	while (bytes) {
		if (b->size > bytes) {
			b->data += bytes;
			b->size -= bytes;
			bytes = 0;
		} else if (b->size != 0) {
			bytes -= b->size;
			b->data = NULL;
			b->size = 0;
		} else {
			if (bitarray_fetch(b, 8) < 0)
				return;
			bytes -= 1;
		}
	}
}

int db_read(const char *name, unsigned char **buf, size_t *size)
{
	struct bitarray b;
	int padding_bytes = 0;
	int i;

	*buf = NULL;
	*size = 0;

	if (!db_count)
		return 1;

	db_offset = 0;
	bitarray_init(&b, NULL, 0);
	bitarray_callback(&b, db_callback);

	for (;;) {
		const long pk_max = 268435456;
		unsigned long pk_crc32 = 0;
		int err = 0;
		long pk_header;
		long pk_method;
		long pk_size_compressed;
		long pk_size_uncompressed;
		long pk_name_length;
		long pk_extra_length;

		db_callback_state = 0;

		if ((pk_header = bitarray_fetch(&b, 8)) < 0)
			break;

		if (pk_header == 0x00) {
			if ((padding_bytes++) > 64) {
				bitarray_clear(&b);
				padding_bytes = 0;
			}
			continue;
		}

		padding_bytes = 0;

		if (pk_header != 0x50)
			continue;

		db_callback_state = 1;

		if ((pk_header = bitarray_fetch(&b, 24)) != 0x0004034B)
			continue;

		db_fast_forward(&b, 4);
		pk_method = bitarray_fetch(&b, 16);
		db_fast_forward(&b, 4);

		{
			long t1 = bitarray_fetch(&b, 16);
			long t2 = bitarray_fetch(&b, 16);
			if (t1 >= 0 && t2 >= 0) {
				pk_crc32 |= (unsigned long)t1;
				pk_crc32 |= (unsigned long)t2 << 16;
			}
		}

		pk_size_compressed = bitarray_fetch(&b, 32);
		pk_size_uncompressed = bitarray_fetch(&b, 32);
		pk_name_length = bitarray_fetch(&b, 16);
		pk_extra_length = bitarray_fetch(&b, 16);

		if (pk_method != 0 && pk_method != 8)
			continue;
		if (pk_size_compressed < 0 || pk_size_compressed > pk_max)
			continue;
		if (pk_size_uncompressed < 0 || pk_size_uncompressed > pk_max)
			continue;
		if (pk_name_length <= 0)
			continue;
		if (pk_extra_length < 0)
			continue;
		if (!pk_method && pk_size_compressed != pk_size_uncompressed)
			continue;

		for (i = 0; i < pk_name_length; i++) {
			long c1 = bitarray_fetch(&b, 8);
			long c2 = (long)(unsigned char)name[i];
			if (c1 != c2)
				err = 1;
		}

		db_fast_forward(&b, (size_t)pk_extra_length);

		if (!err) {
			size_t out_size = (size_t)pk_size_uncompressed;
			void *out;

			if (!out_size)
				return 0;
			if ((out = malloc(out_size)) == NULL)
				return 2;

			if (!pk_method) {
				unsigned char *p = out;
				while (out_size--) {
					long c = bitarray_fetch(&b, 8);
					if (c < 0)
						return free(out), 3;
					*p++ = (unsigned char)c;
				}
				out_size = (size_t)pk_size_uncompressed;
			} else {
				if (inflate(&b, out, &out_size))
					return free(out), 3;
			}

			if (db_callback_state < 0)
				return free(out), 3;

			if (pk_crc32 != crc32(out, out_size))
				return free(out), 4;

			*buf = out;
			*size = out_size;
			return 0;
		}
		db_fast_forward(&b, (size_t)pk_size_compressed);
	}
	return 1;
}

const char *db_read_error(int retval)
{
	static const char *e[] = {
		"success",                    /* 0 */
		"could not found",            /* 1 */
		"could not allocate memory",  /* 2 */
		"could not read",             /* 3 */
		"CRC-32 does not match"       /* 4 */
	};

	if (retval < 0 || retval >= (int)(sizeof(e) / sizeof(e[0])))
		return e[0];
	return e[retval];
}
