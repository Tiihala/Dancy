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
 * dy-png/huffman.c
 *      Huffman coding scheme
 */

#include "program.h"

int huffman_init(struct huffman *h, unsigned *symbols, int n)
{
	int i;

	for (i = 0; i < 16; i++)
		h->lengths[i] = 0;
	h->symbols = symbols;
	h->completed = 0;

	/*
	 * At this point, "symbols" is an array of bit-lengths
	 * but the same array is used for actual symbols later.
	 */
	for (i = 0; i < n; i++) {
		unsigned len = symbols[i];
		if (len > 15)
			return -1;
		h->lengths[len] += 1;
	}

	/*
	 * All following algorithms are not invented here. The
	 * implementation is based on ideas/algorithms used in
	 * inftrees.c by Mark Adler (https://zlib.net). Thanks!
	 *
	 * The idea of using bits 0-3 for length and then using
	 * other bits for saving the symbol value is invented
	 * here but might have been implemented by someone else
	 * before this. The nice feature here is that the final
	 * shift right (4) clears the unused symbol entries (if
	 * there are any) so bit-lengths do not "leak" back to
	 * the caller. For example:
	 *
	 * Input:  symbols[] == { 2, 1, 0, 3, 3 }  bit-lengths
	 * Output: symbols[] == { 1, 0, 3, 4, 0 }  symbols
	 *                                    ^
	 *                                    |___ set to zero
	 */
	if ((int)h->lengths[0] != n) {
		unsigned buf[16];
		long t = 1L;

		/*
		 * Check that given bit-lengths are reasonable.
		 */
		for (i = 1; i < 16; i++) {
			long len = (long)h->lengths[i];
			if ((t *= 2, t -= len) < 0)
				return -2;
		}
		h->completed = (t == 0L);

		/*
		 * Build the array of symbols. Each entry is an
		 * integer value that represents a symbol.
		 */
		for (buf[1] = 0, i = 1; i < 15; i++) {
			unsigned len = h->lengths[i];
			unsigned off = buf[i];
			buf[i + 1] = len + off;
		}
		for (i = 0; i < n; i++) {
			unsigned len = h->symbols[i] & 0x0Fu;
			unsigned sym = (unsigned)i << 4;
			if (!len)
				continue;
			h->symbols[buf[len]] |= sym;
			buf[len] += 1;
		}
		for (i = 0; i < n; i++)
			h->symbols[i] >>= 4;
	}
	return 0;
}

int huffman_fetch(struct huffman *h, struct bitarray *b)
{
	unsigned base = 0;
	unsigned off = 0;
	unsigned val = 0;
	int i = 1;

	while (i < 16) {
		if (b->state[0]--) {
			unsigned len = h->lengths[i++];
			val = (val << 1) | (b->state[1] & 1u);
			b->state[1] >>= 1;
			if (val < base + len) {
				unsigned n = val - base;
				return (int)h->symbols[off + n];
			}
			off += len;
			base = (base + len) << 1;
		} else if (b->size--) {
			b->state[0] = CHAR_BIT;
			b->state[1] = (unsigned)*b->data++;
		} else {
			bitarray_clear(b);
			if (b->callback(b) || !b->size)
				return -1;
		}
	}
	return -2;
}

int huffman_table(struct huffman *h, unsigned *table, int n)
{
	unsigned sum = 0;
	unsigned off = 0;
	unsigned val = 0;
	unsigned i;

	if (table == h->symbols || n < 0)
		return -1;
	for (i = 0; i < (unsigned)n; i++)
		table[i] = 0;

	for (i = 1; i < 16; i++)
		sum += h->lengths[i];
	if (sum * 2u != (unsigned)n)
		return -1;

	for (i = 1; i < 16; i++) {
		unsigned len = h->lengths[i];
		unsigned j;

		for (j = 0; j < len; j++) {
			unsigned sym = h->symbols[off + j] * 2u;
			unsigned rev = 0;
			unsigned k;

			if (sym >= (unsigned)n)
				return -2;
			for (k = 0; k < i; k++) {
				rev <<= 1;
				rev |= (val & (1u << k)) ? 1 : 0;
			}
			table[sym + 0] = i;
			table[sym + 1] = rev;
			val += 1;
		}
		val <<= 1;
		off += len;
	}
	return 0;
}
