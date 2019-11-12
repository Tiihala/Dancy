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
 * init/db.c
 *      Handle database files
 */

#include <init.h>

static int db_count;
static void **db_array;
static struct b_mem *db_map;
static size_t db_offset;

static void *get_db(const struct b_mem *map, int *num, int *off)
{
	int i;

	for (i = *off; (i == 0 || map[i].base); i++) {
		uint32_t t = map[i].type - (uint32_t)*num;
		if (t == B_MEM_DATABASE_MIN) {
			*num += 1;
			*off = i + 1;
			return (void *)map[i].base;
		}
	}
	for (i = 1; i < *off; i++) {
		uint32_t t = map[i].type - (uint32_t)*num;
		if (t == B_MEM_DATABASE_MIN) {
			*num += 1;
			*off = i + 1;
			return (void *)map[i].base;
		}
	}
	return NULL;
}

int db_init(struct b_mem *map)
{
	const char *err = "Error: boot loader gave a corrupted memory map";
	int off;

	for (db_count = 0, off = 0; /* void */; /* void */)
		if (get_db(map, &db_count, &off) == NULL)
			break;
	if (!db_count)
		return 0;

	db_array = calloc((size_t)db_count, sizeof(void *));
	if (!db_array)
		return 1;

	for (db_count = 0, off = 0; /* void */; /* void */) {
		unsigned db = (unsigned)db_count;
		unsigned char *m = get_db(map, &db_count, &off);

		if (!m)
			break;
		if (LE32(&m[0]) != 0x0D54418D || LE32(&m[28]) != db) {
			b_print("%s\n", err);
			return free(db_array), 1;
		}
		db_array[db] = m;
	}

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
			(void)bitarray_fetch(b, 8);
			bytes -= 1;
		}
	}
}

int db_read(const char *name, unsigned char **buf, size_t *size)
{
	struct bitarray b;
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

		if ((pk_header = bitarray_fetch(&b, 8)) < 0)
			break;
		if (pk_header != 0x50)
			continue;
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

		err = (pk_name_length == (long)strlen(name)) ? 0 : 1;

		for (i = 0; !err && i < pk_name_length; i++) {
			long c1 = bitarray_fetch(&b, 8);
			long c2 = (long)(unsigned char)name[i];
			if (c1 != c2)
				err = 1;
		}

		db_fast_forward(&b, (size_t)pk_extra_length);

		if (!err) {
			size_t out_size = (size_t)pk_size_uncompressed;
			void *out = malloc(out_size);

			if (!out_size)
				return 0;
			if (!out)
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
				if (inflate_uncompress(&b, out, &out_size))
					return free(out), 3;
			}

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
