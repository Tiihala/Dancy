/*
 * Copyright (c) 2018 Antti Tiihala
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
 * dy-init/ia32.c
 *      Code for relocating init executable code
 */

#include <dancy.h>

const unsigned char ia32[416] = {
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x89E3,0x81E3,0x0000,0xFFFF,0x8DB3,0xA001,0x0000,0x8B0E),
	DANCY_BLOB(0x8B6E,0x1081,0xE94C,0x0104,0x0009,0xE975,0x408B,0x4310),
	DANCY_BLOB(0x83C0,0x0F83,0xE0F0,0x8986,0xA000,0x0000,0x8D7C,0x24F0),
	DANCY_BLOB(0x89FC,0xB904,0x0000,0x008D,0x5628,0x8B02,0x01D8,0x721D),
	DANCY_BLOB(0x8907,0x8D52,0x288D,0x7F04,0x83E9,0x0175,0xED8B,0x4E0C),
	DANCY_BLOB(0xBAFF,0xFF00,0x008B,0x7E08,0x8D3C,0x3BEB,0x03F4,0xEBFD),
	DANCY_BLOB(0x8B47,0x0C21,0xD074,0x3683,0xF804,0x7721,0x8B44,0x84FC),
	DANCY_BLOB(0x817F,0x005F,0x7374,0x6175,0x0F81,0x7F04,0x7274,0x0000),
	DANCY_BLOB(0x7506,0x8B6F,0x088D,0x2C28,0x0147,0x0872,0x108D,0x7F12),
	DANCY_BLOB(0x83E9,0x0175,0xCB8D,0x35B4,0x0100,0x00EB,0x03F4,0xEBFD),
	DANCY_BLOB(0x8B4C,0x3320,0x81E1,0xFFFF,0x0000,0x7434,0x8B44,0x3314),
	DANCY_BLOB(0x01D8,0x8B54,0x3318,0x01DA,0x894C,0x2400,0x8944,0x2404),
	DANCY_BLOB(0x8954,0x2408,0x8B83,0xA801,0x0000,0x01D8,0x8944,0x240C),
	DANCY_BLOB(0xE83B,0x0000,0x0083,0x4424,0x080A,0x832C,0x2401,0x75F0),
	DANCY_BLOB(0x8D76,0x2881,0xFE2C,0x0200,0x0072,0xB574,0x03F4,0xEBFD),
	DANCY_BLOB(0x83C4,0x1085,0xED74,0x0655,0xE803,0x0000,0x00F4,0xEBFD),
	DANCY_BLOB(0x31C0,0x31C9,0x31D2,0x31ED,0x31F6,0x31FF,0x83C4,0x04C3),
	DANCY_BLOB(0x5356,0xB812,0x0000,0x0031,0xC98D,0x5C24,0x0C8B,0x7308),
	DANCY_BLOB(0x660B,0x4E08,0x7447,0x8B7E,0x04F7,0xE78B,0x3E03,0x7B04),
	DANCY_BLOB(0x8B73,0x0C8D,0x3406,0x8B46,0x0883,0xF906,0x7506,0x0107),
	DANCY_BLOB(0x721E,0xEB29,0x83F9,0x0775,0x092B,0x4304,0x0107,0x7210),
	DANCY_BLOB(0xEB1B,0x83F9,0x1475,0x098D,0x4F04,0x29C8,0x0107,0xEB0D),
	DANCY_BLOB(0x8B5B,0x088B,0x038B,0x5304,0x31DB,0xF4EB,0xFD5E,0x5BC3),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000)
};
