/*
 * Copyright (c) 2020 Antti Tiihala
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
 * init/smp/start64.c
 *      Symmetric multiprocessing (64-bit)
 */

#include <boot/init.h>

const unsigned char smp_trampoline[512] = {
	DANCY_BLOB(0x2E0F,0x011E,0x0201,0x0F09,0xB841,0x54EB,0x03F4,0xEBFD),
	DANCY_BLOB(0xFA8C,0xCB8E,0xDB66,0x81E3,0xFFFF,0x0000,0x66C1,0xE304),
	DANCY_BLOB(0x6689,0xD866,0x0510,0x0100,0x0066,0xA30C,0x0166,0x89D8),
	DANCY_BLOB(0x6605,0x5000,0x0000,0x66A3,0x3001,0x0F20,0xC083,0xC801),
	DANCY_BLOB(0x0F01,0x160A,0x010F,0x22C0,0xE900,0x0066,0xFF2E,0x3001),
	DANCY_BLOB(0xB841,0x54EB,0xFEB8,0x1000,0x0000,0x8EC0,0x8ED8,0x8EE0),
	DANCY_BLOB(0x8EE8,0x8ED0,0x8BA3,0xF001,0x0000,0x8DBB,0xF801,0x0000),
	DANCY_BLOB(0xB800,0x0000,0x00BA,0x0100,0x0000,0xF00F,0xB117,0x75F0),
	DANCY_BLOB(0xB802,0x0000,0x00BA,0x0300,0x0000,0xF00F,0xB117,0x7410),
	DANCY_BLOB(0x833F,0x020F,0x84E7,0xFFFF,0xFFF3,0x90E9,0xF0FF,0xFFFF),
	DANCY_BLOB(0x0F20,0xE00F,0xBAE8,0x050F,0x22E0,0x8B83,0xF401,0x0000),
	DANCY_BLOB(0x0F22,0xD8B9,0x8000,0x00C0,0x0F32,0x0FBA,0xE808,0x0F30),
	DANCY_BLOB(0x89D9,0x81C1,0xE000,0x0000,0x89DF,0x81C7,0x3601,0x0000),
	DANCY_BLOB(0x890F,0x0F20,0xC181,0xC900,0x0000,0x800F,0x22C1,0xFF2F),
	DANCY_BLOB(0xFC31,0xC083,0xE4F8,0x4887,0x0424,0xFFE0,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x1F00,0x1001,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0xFFFF,0x0000,0x009B,0xCF00),
	DANCY_BLOB(0xFFFF,0x0000,0x0093,0xCF00,0xFFFF,0x0000,0x009B,0xAF00),
	DANCY_BLOB(0x0000,0x0000,0x0800,0x0000,0x0000,0x1800,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000)
};
