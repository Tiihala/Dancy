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
 * dy-uefi/x64.c
 *      PE32+ UEFI Image
 */

#include <dancy.h>

const unsigned char x64[8224] = {
	DANCY_BLOB(0x4D5A,0x0000,0x0400,0x0000,0x2000,0x0000,0xFFFF,0x0000),
	DANCY_BLOB(0x0004,0x0000,0x4000,0x0000,0x4000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0008,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x4461,0x6E63,0x7920,0x4F70,0x6572,0x6174,0x696E,0x6720),
	DANCY_BLOB(0x5379,0x7374,0x656D,0x0D0A,0x2400,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x8CC8,0x8ED8,0xB800,0x0931,0xD2CD,0x21B8,0x004C,0xCD21),
	DANCY_BLOB(0xEBFE,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x5045,0x0000,0x6486,0x0200,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0xF000,0x2200,0x0B02,0x0000,0x0010,0x0000),
	DANCY_BLOB(0x0010,0x0000,0x0000,0x0000,0x0010,0x0000,0x0010,0x0000),
	DANCY_BLOB(0x0000,0x0080,0x0100,0x0000,0x0010,0x0000,0x0010,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0030,0x0000,0x0010,0x0000,0x0000,0x0000,0x0A00,0x0000),
	DANCY_BLOB(0x0000,0x0100,0x0000,0x0000,0x0010,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0100,0x0000,0x0000,0x0010,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x1000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x2E74,0x6578,0x7400,0x0000),
	DANCY_BLOB(0x0010,0x0000,0x0010,0x0000,0x0010,0x0000,0x0010,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x2000,0x0060),
	DANCY_BLOB(0x2E64,0x6174,0x6100,0x0000,0x0010,0x0000,0x0020,0x0000),
	DANCY_BLOB(0x0010,0x0000,0x0020,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x4000,0x00C0,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x31C0,0x4989,0xE450,0x4883,0xE4F0,0x5152,0x4154,0x6A00),
	DANCY_BLOB(0xB901,0x0000,0x00BA,0x0100,0x0000,0x41B8,0x0002,0x0000),
	DANCY_BLOB(0x4C8D,0x0C24,0x41C7,0x0100,0x0000,0xF048,0x8B44,0x2410),
	DANCY_BLOB(0x488B,0x4060,0x4883,0xEC20,0xFF50,0x2848,0x85C0,0x7520),
	DANCY_BLOB(0x8B7C,0x2420,0x837C,0x2424,0x0075,0x1583,0xFF00,0x7410),
	DANCY_BLOB(0x81FF,0x0000,0x00F0,0x7308,0xF7C7,0xFF0F,0x0000,0x7430),
	DANCY_BLOB(0x488B,0x4C24,0x3048,0x8B49,0x4048,0x8D15,0x1002,0x0000),
	DANCY_BLOB(0xFF51,0x084C,0x89E4,0x4881,0x3C24,0x0000,0x0000,0x7501),
	DANCY_BLOB(0xF4B8,0x0100,0x0000,0xBA00,0x0000,0x008D,0x0A74,0xF1C3),
	DANCY_BLOB(0x8B0D,0xAAF8,0xFFFF,0x488D,0x3563,0x0F00,0x00FC,0xF3A4),
	DANCY_BLOB(0xB900,0x0020,0x002B,0x0D95,0xF8FF,0xFF72,0xB3FC,0xF3AA),
	DANCY_BLOB(0x4883,0xC420,0x5B4C,0x3964,0x2400,0x7511,0x4883,0xC408),
	DANCY_BLOB(0x4159,0x4158,0x4883,0xEC10,0x488D,0x33EB,0x03F4,0xEBFD),
	DANCY_BLOB(0x8B4E,0x008B,0x6E10,0x81E9,0x6486,0x0400,0x09E9,0x753D),
	DANCY_BLOB(0x8B05,0x5AF8,0xFFFF,0x8986,0xA000,0x0000,0x488D,0x3C24),
	DANCY_BLOB(0xB904,0x0000,0x008D,0x5628,0x8B02,0x01D8,0x721F,0x8907),
	DANCY_BLOB(0x488D,0x5228,0x488D,0x7F04,0x83E9,0x0175,0xEB8B,0x4E0C),
	DANCY_BLOB(0xBAFF,0xFF00,0x008B,0x7E08,0x8D3C,0x3BEB,0x03F4,0xEBFD),
	DANCY_BLOB(0x8B47,0x0C21,0xD074,0x3683,0xF804,0x7721,0x8B44,0x84FC),
	DANCY_BLOB(0x817F,0x0073,0x7461,0x7275,0x0F81,0x7F04,0x7400,0x0000),
	DANCY_BLOB(0x7506,0x8B6F,0x088D,0x2C28,0x0147,0x0872,0x108D,0x7F12),
	DANCY_BLOB(0x4883,0xE901,0x75CA,0xBE14,0x0000,0x00EB,0x03F4,0xEBFD),
	DANCY_BLOB(0x8B4C,0x3320,0x81E1,0xFFFF,0x0000,0x7434,0x8B44,0x3314),
	DANCY_BLOB(0x01D8,0x8B54,0x3318,0x01DA,0x894C,0x2400,0x8944,0x2404),
	DANCY_BLOB(0x8954,0x2408,0x8B83,0x0800,0x0000,0x01D8,0x8944,0x240C),
	DANCY_BLOB(0xE85B,0x0000,0x0083,0x4424,0x080A,0x832C,0x2401,0x75F0),
	DANCY_BLOB(0x8D76,0x2881,0xFE8C,0x0000,0x0072,0xB574,0x03F4,0xEBFD),
	DANCY_BLOB(0x4C89,0xE485,0xED74,0x0655,0xE803,0x0000,0x00F4,0xEBFD),
	DANCY_BLOB(0x488D,0x6424,0x084C,0x89C1,0x4C89,0xCA49,0x89D8,0x31C0),
	DANCY_BLOB(0x31DB,0x31ED,0x4831,0xF648,0x31FF,0x4D31,0xC94D,0x31D2),
	DANCY_BLOB(0x4D31,0xDB4D,0x31E4,0x4D31,0xED4D,0x31F6,0x4D31,0xFFC3),
	DANCY_BLOB(0x5356,0x31C0,0x31C9,0x488D,0x5C24,0x18B0,0x128B,0x7308),
	DANCY_BLOB(0x660B,0x4E08,0x7447,0x8B7E,0x04F7,0xE78B,0x3E03,0x7B04),
	DANCY_BLOB(0x8B73,0x0C8D,0x3406,0x8B46,0x0883,0xF902,0x7403,0x7706),
	DANCY_BLOB(0x4801,0x4700,0xEB27,0x83F9,0x0375,0x072B,0x4304,0x0107),
	DANCY_BLOB(0xEB1B,0x83F9,0x0977,0x098D,0x140F,0x29D0,0x0107,0xEB0D),
	DANCY_BLOB(0x8B5B,0x088B,0x038B,0x5304,0x31DB,0xF4EB,0xFD5E,0x5BC3),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x4100,0x6C00,0x6C00,0x6F00,0x6300,0x6100,0x7400,0x6500),
	DANCY_BLOB(0x5000,0x6100,0x6700,0x6500,0x7300,0x3A00,0x2000,0x6F00),
	DANCY_BLOB(0x7500,0x7400,0x2000,0x6F00,0x6600,0x2000,0x7200,0x6500),
	DANCY_BLOB(0x7300,0x6F00,0x7500,0x7200,0x6300,0x6500,0x7300,0x0D00),
	DANCY_BLOB(0x0A00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x2008,0x0000,0x5008,0x0000,0x3809,0x0000,0x4009,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000)
};