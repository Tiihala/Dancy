/*
 * Copyright (c) 2017, 2018 Antti Tiihala
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
 * gpt.c
 *      General purpose implementation of Master Boot Record (GPT)
 */

#include <dancy.h>

const unsigned char gpt_bin[512] = {
	DANCY_BLOB(0xB841,0x54EB,0x1BEB,0xF900,0x0100,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x4755,0x4944,0x5F42,0x4F4F,0x544D,0x414E,0x4147,0x4552),
	DANCY_BLOB(0x31C0,0x8EC0,0x8ED8,0xB900,0x01BE,0x007C,0xBF00,0x06FA),
	DANCY_BLOB(0x8ED0,0xBC00,0x7CFB,0xFCF3,0xA5B6,0x03EA,0x4006,0x0000),
	DANCY_BLOB(0x5050,0x52F9,0xBBAA,0x55B4,0x41CD,0x13FB,0xFC5A,0x1F07),
	DANCY_BLOB(0x7208,0xD1E9,0x81D3,0xAA55,0x7406,0xBE93,0x07E9,0x2001),
	DANCY_BLOB(0xBE08,0x06E8,0xCA00,0x72F8,0xB808,0x0089,0xC1BE,0x9C07),
	DANCY_BLOB(0xBF00,0x80F3,0xA6BE,0x9A07,0x7412,0x3A26,0x0706,0x7563),
	DANCY_BLOB(0xD026,0x4B07,0x3A06,0x4B07,0x73D6,0xEB57,0x668B,0x4518),
	DANCY_BLOB(0x66A3,0x0806,0x668B,0x451C,0x66A3,0x0C06,0x66FF,0x7544),
	DANCY_BLOB(0x66FF,0x7540,0x8B45,0x202D,0x0300,0x8B5D,0x4C4B,0x89E6),
	DANCY_BLOB(0xBF10,0x8060,0xE879,0x0061,0xB110,0xBE10,0x0657,0xF3A6),
	DANCY_BLOB(0x7424,0x5F47,0x01DF,0x7211,0x81FF,0x0090,0x72EA,0x89E6),
	DANCY_BLOB(0x6683,0x0401,0x7203,0x4879,0xD580,0x2E07,0x0601,0x7280),
	DANCY_BLOB(0xBE9A,0x07E9,0x9A00,0x66BC,0x007C,0x0000,0xF645,0x1001),
	DANCY_BLOB(0x74E7,0x66FF,0x7504,0x66FF,0x3589,0xE6E8,0x3200,0x72F9),
	DANCY_BLOB(0x813E,0xFE7D,0x55AA,0x75D1,0x8A36,0x4B07,0x52B9,0x1200),
	DANCY_BLOB(0x0EE2,0xFD1F,0x07B5,0x01BE,0x0080,0xBF00,0x7CFC,0xF3A5),
	DANCY_BLOB(0x6661,0x5A88,0xF1B6,0x0066,0xBD47,0x5054,0x20E9,0xD074),
	DANCY_BLOB(0x5266,0xFF34,0x66FF,0x7404,0x31C0,0xB900,0x82BE,0x007E),
	DANCY_BLOB(0x89F7,0xFCF3,0xAAC6,0x0410,0xC644,0x0201,0xC644,0x0580),
	DANCY_BLOB(0x665B,0x6659,0x8A36,0x4B07,0x6601,0x4C08,0x6611,0x5C0C),
	DANCY_BLOB(0x7212,0xFECE,0x75F2,0xB442,0xCD13,0x7308,0xB400,0xCD13),
	DANCY_BLOB(0x5AF9,0xEB03,0x5AB6,0x04BE,0xA607,0xFECE,0x7402,0xFBC3),
	DANCY_BLOB(0xB40F,0x56CD,0x10FB,0xB307,0x5EAC,0x24FF,0xB40E,0x75F2),
	DANCY_BLOB(0xF4EB,0xFD21,0x204C,0x4241,0x0D00,0x3F20,0x4546,0x4920),
	DANCY_BLOB(0x5041,0x5254,0x0D00,0x2120,0x4844,0x4420,0x492F,0x4F0D),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x55AA)
};