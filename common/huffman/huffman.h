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
 * huffman.h
 *      Huffman coding scheme
 */

#ifndef COMMON_HUFFMAN_H
#define COMMON_HUFFMAN_H

#include <limits.h>
#include <stddef.h>
#include <bitarray/bitarray.h>

struct huffman {
	unsigned lengths[16];
	unsigned *symbols;
	unsigned completed;
};

int huffman_init(struct huffman *h, unsigned *symbols, int n);
int huffman_fetch(struct huffman *h, struct bitarray *b);
int huffman_table(struct huffman *h, unsigned *table, int n);

#endif
