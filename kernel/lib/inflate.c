/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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
 * lib/inflate.c
 *      Uncompress Deflate stream
 */

#include <misc/lib.h>

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

static int inflate_init(struct huffman **h_code, struct huffman **h_dist)
{
	static struct huffman h1;
	static struct huffman h2;
	static unsigned code_symbols[288];
	static unsigned dist_symbols[32];
	int i;

	for (i = 0; i <= 143; i++)
		code_symbols[i] = 8;
	for (i = 144; i <= 255; i++)
		code_symbols[i] = 9;
	for (i = 256; i <= 279; i++)
		code_symbols[i] = 7;
	for (i = 280; i <= 285; i++)
		code_symbols[i] = 8;
	for (i = 286; i <= 287; i++)
		code_symbols[i] = 8;

	for (i = 0; i <= 31; i++)
		dist_symbols[i] = 5;

	if (huffman_init(&h1, &code_symbols[0], 288))
		return 1;
	if (huffman_init(&h2, &dist_symbols[0], 32))
		return 1;

	*h_code = &h1;
	*h_dist = &h2;
	return 0;
}

int inflate(struct bitarray *b, unsigned char *out, size_t *size)
{
	static int init = 0;
	static struct huffman *h_code;
	static struct huffman *h_dist;
	size_t off = 0;
	int sym;

	if (!init && inflate_init(&h_code, &h_dist))
		return 2;
	else
		init = 1;

	/*
	 * Deflate stream must have only one block and "static Huffman tree".
	 */
	if (bitarray_fetch(b, 1) != 1 || bitarray_fetch(b, 2) != 1)
		return 2;

	while ((sym = huffman_fetch(h_code, b)) != 256) {
		unsigned bits, len, dist;
		long val;

		if (sym < 0)
			return 3;
		if (sym < 256) {
			if (off >= *size)
				return 4;
			out[off++] = (unsigned char)sym;
			continue;
		}
		if (sym >= 286)
			return 5;

		sym -= 257;
		bits = length_codes[sym].extra;
		if ((val = bitarray_fetch(b, bits)) < 0)
			return 5;

		len = length_codes[sym].value + (unsigned)val;
		if (off + (size_t)len > *size)
			return 5;

		if ((sym = huffman_fetch(h_dist, b)) < 0 || sym >= 30)
			return 5;

		bits = distance_codes[sym].extra;
		if ((val = bitarray_fetch(b, bits)) < 0)
			return 5;

		dist = distance_codes[sym].value + (unsigned)val;
		if (dist > off)
			return 4;

		while (len--) {
			out[off] = out[off - (size_t)dist];
			off += 1;
		}
	}

	if (*size != off) {
		*size = off;
		return 1;
	}
	return 0;
}
