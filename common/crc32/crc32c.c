/*
 * Copyright (c) 2017 Antti Tiihala
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
 * crc32c.c
 *      Cyclic Redundancy Check - "CRC-32C" for SSE4.2, ext4, etc.
 */

#include <limits.h>
#include <stddef.h>

unsigned long crc32c(const void *obj, size_t len)
{
	const unsigned char *ptr = (const unsigned char *)obj;
	unsigned long crc = 0xFFFFFFFFul;
	int i;

	while (len--) {
		crc ^= (unsigned long)*ptr++;
		for (i = 0; i < CHAR_BIT; i++) {
			if (crc & 1ul)
				crc >>= 1, crc ^= 0x82F63B78ul;
			else
				crc >>= 1;
		}
	}

	return ~crc & 0xFFFFFFFFul;
}
