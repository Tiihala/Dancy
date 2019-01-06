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
 * lib/bitarray.c
 *      Handle bit arrays
 */

#include <dancy.h>

void bitarray_init(struct bitarray *b, unsigned char *data, size_t size)
{
	b->data = size ? data : NULL;
	b->size = size;
	b->state[0] = 0;
	b->state[1] = 0;
	b->written = 0;
}

long bitarray_aligned_fetch(struct bitarray *b, unsigned bits, void **data)
{
	if (b->state[0]) {
		b->state[0] = 0;
		b->state[1] = 0;
	}
	if (data != NULL)
		*data = b->data;
	return bitarray_fetch(b, bits);
}

long bitarray_fetch(struct bitarray *b, unsigned bits)
{
	unsigned bit = 0;
	unsigned long ret = 0;

	while (bit < bits) {
		if (b->state[0]--) {
			ret |= (unsigned long)(b->state[1] & 1u) << bit++;
			b->state[1] >>= 1;
		} else if (b->size--) {
			b->state[0] = CHAR_BIT;
			b->state[1] = (unsigned)*b->data++;
		} else {
			return bitarray_init(b, NULL, 0), -1;
		}
	}
	return (long)(ret & 0x7FFFFFFFul);
}

int bitarray_shove(struct bitarray *b, unsigned bits, unsigned val)
{
	unsigned bit = 0;

	while (bit++ < bits) {
		b->state[1] |= (val & 1u) << b->state[0], val >>= 1;
		if (++b->state[0] < CHAR_BIT)
			continue;
		if (b->size--) {
			*b->data++ = (unsigned char)b->state[1];
			b->state[0] = 0;
			b->state[1] = 0;
			b->written++;
		} else {
			return bitarray_init(b, NULL, 0), -1;
		}
	}
	return 0;
}

int bitarray_written(struct bitarray *b, size_t *written)
{
	unsigned bits = b->state[0] ? CHAR_BIT - b->state[0] : 0;
	int ret = bitarray_shove(b, bits, 0);

	if (written != NULL)
		*written = b->written;
	return ret;
}
