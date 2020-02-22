/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * dy-zip/deflate.c
 *      Program for creating Zip archives
 */

#include "program.h"

struct bit_code {
	unsigned extra;
	unsigned value;
};

static const struct bit_code length_codes[29] = {
	{ 0,   3 }, /* 257 */
	{ 0,   4 }, /* 258 */
	{ 0,   5 }, /* 259 */
	{ 0,   6 }, /* 260 */
	{ 0,   7 }, /* 261 */
	{ 0,   8 }, /* 262 */
	{ 0,   9 }, /* 263 */
	{ 0,  10 }, /* 264 */
	{ 1,  11 }, /* 265 */
	{ 1,  13 }, /* 266 */
	{ 1,  15 }, /* 267 */
	{ 1,  17 }, /* 268 */
	{ 2,  19 }, /* 269 */
	{ 2,  23 }, /* 270 */
	{ 2,  27 }, /* 271 */
	{ 2,  31 }, /* 272 */
	{ 3,  35 }, /* 273 */
	{ 3,  43 }, /* 274 */
	{ 3,  51 }, /* 275 */
	{ 3,  59 }, /* 276 */
	{ 4,  67 }, /* 277 */
	{ 4,  83 }, /* 278 */
	{ 4,  99 }, /* 279 */
	{ 4, 115 }, /* 280 */
	{ 5, 131 }, /* 281 */
	{ 5, 163 }, /* 282 */
	{ 5, 195 }, /* 283 */
	{ 5, 227 }, /* 284 */
	{ 0, 258 }  /* 285 */
};

static const struct bit_code distance_codes[31] = {
	{  0,     1 }, /*  0 */
	{  0,     2 }, /*  1 */
	{  0,     3 }, /*  2 */
	{  0,     4 }, /*  3 */
	{  1,     5 }, /*  4 */
	{  1,     7 }, /*  5 */
	{  2,     9 }, /*  6 */
	{  2,    13 }, /*  7 */
	{  3,    17 }, /*  8 */
	{  3,    25 }, /*  9 */
	{  4,    33 }, /* 10 */
	{  4,    49 }, /* 11 */
	{  5,    65 }, /* 12 */
	{  5,    97 }, /* 13 */
	{  6,   129 }, /* 14 */
	{  6,   193 }, /* 15 */
	{  7,   257 }, /* 16 */
	{  7,   385 }, /* 17 */
	{  8,   513 }, /* 18 */
	{  8,   769 }, /* 19 */
	{  9,  1025 }, /* 20 */
	{  9,  1537 }, /* 21 */
	{ 10,  2049 }, /* 22 */
	{ 10,  3073 }, /* 23 */
	{ 11,  4097 }, /* 24 */
	{ 11,  6145 }, /* 25 */
	{ 12,  8193 }, /* 26 */
	{ 12, 12289 }, /* 27 */
	{ 13, 16385 }, /* 28 */
	{ 13, 24577 }, /* 29 */
	{  0, 65535 }
};

static unsigned code_table[2 * 288];
static unsigned dist_table[2 * 32];

static int put_lendist(struct bitarray *b, unsigned len, unsigned dist)
{
	unsigned i;

	if (len < 3 || len > 258 || dist < 1 || dist > 32768)
		return -1;

	for (i = 0; /* void */; i++) {
		unsigned val = length_codes[i].value;
		if (len == val || len < length_codes[i + 1].value) {
			unsigned t1 = code_table[((257 + i) << 1) + 0];
			unsigned t2 = code_table[((257 + i) << 1) + 1];

			if (bitarray_shove(b, t1, t2))
				return -1;

			t1 = length_codes[i].extra;
			t2 = len - val;

			if (t1 && bitarray_shove(b, t1, t2))
				return -1;
			break;
		}
	}

	for (i = 0; /* void */; i++) {
		unsigned val = distance_codes[i].value;
		if (dist == val || dist < distance_codes[i + 1].value) {
			unsigned t1 = dist_table[(i << 1) + 0];
			unsigned t2 = dist_table[(i << 1) + 1];

			if (bitarray_shove(b, t1, t2))
				return -1;

			t1 = distance_codes[i].extra;
			t2 = dist - val;

			if (t1 && bitarray_shove(b, t1, t2))
				return -1;
			break;
		}
	}
	return 0;
}

static int put_literal(struct bitarray *b, unsigned c)
{
	unsigned t1 = code_table[((c & 0xFFu) << 1) + 0];
	unsigned t2 = code_table[((c & 0xFFu) << 1) + 1];

	return bitarray_shove(b, t1, t2);
}

static int lz77_compress(struct bitarray *b, unsigned char *data, size_t size)
{
	size_t window_size = (size <= 524288) ? 32768 : 1024;
	size_t i, j;

	if (put_literal(b, (unsigned)data[0]))
		return -1;

	for (i = 1; i < size; /* void */) {
		unsigned char c1, c2;
		size_t limit = (size - i < 258) ? size - i : 258;
		unsigned found_len = 2;
		unsigned found_dist = 0;
		size_t dist;

		c1 = data[i];
		c2 = (limit > 2) ? data[i + 1] : 0;

		for (dist = 1; dist <= i && dist <= window_size; dist++) {
			const unsigned char *d = &data[i] - dist;

			if (d[0] == c1 && d[1] == c2) {
				for (j = 2; j < limit; j++) {
					if (d[j] != data[i + j])
						break;
				}
				if (found_len < (unsigned)j) {
					found_len = (unsigned)j;
					found_dist = (unsigned)dist;
					if (found_len >= 64)
						break;
				}
			}
		}

		if (found_dist) {
			if (put_lendist(b, found_len, found_dist))
				return -1;
			i += found_len;
		} else {
			if (put_literal(b, (unsigned)data[i]))
				return -1;
			i += 1;
		}
	}
	return 0;
}

int deflate_compress(unsigned char *data, size_t *size)
{
	struct bitarray b;
	struct huffman h;
	unsigned symbols[288];
	unsigned char *buf;
	size_t written;
	int i;

	if (*size < 4)
		return 1;

	for (i = 0; i <= 143; i++)
		symbols[i] = 8;
	for (i = 144; i <= 255; i++)
		symbols[i] = 9;
	for (i = 256; i <= 279; i++)
		symbols[i] = 7;
	for (i = 280; i <= 285; i++)
		symbols[i] = 8;
	for (i = 286; i <= 287; i++)
		symbols[i] = 8;

	if (huffman_init(&h, symbols, 288))
		return 1;
	if (huffman_table(&h, code_table, 2 * 288))
		return 1;

	for (i = 0; i <= 31; i++)
		symbols[i] = 5;

	if (huffman_init(&h, symbols, 32))
		return 1;
	if (huffman_table(&h, dist_table, 2 * 32))
		return 1;

	if ((buf = malloc(*size)) == NULL) {
		fputs("Warning: not enough memory for compression\n", stderr);
		return 1;
	}

	/*
	 * Deflate stream
	 *
	 * Have only one block and use the static Huffman tree.
	 */
	bitarray_init(&b, buf, *size);
	bitarray_shove(&b, 1, 1);
	bitarray_shove(&b, 2, 1);
	if (lz77_compress(&b, data, *size))
		return free(buf), 1;

	bitarray_shove(&b, code_table[2 * 256 + 0], code_table[2 * 256 + 1]);
	if (bitarray_written(&b, &written) || written > *size - 1)
		return free(buf), 1;

	memcpy(&data[0], &buf[0], written), *size = written;
	return free(buf), 0;
}
