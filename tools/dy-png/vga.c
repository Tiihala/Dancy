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
 * dy-png/vga.c
 *      Convert .bmp files to Dancy-compatible .png files
 */

#include "program.h"

#define VGA_COLORS 16

static unsigned long vga_colors[VGA_COLORS] = {
	0x00000000, /* 0x00 */
	0x00AA0000, /* 0x01 */
	0x0000AA00, /* 0x02 */
	0x00AAAA00, /* 0x03 */
	0x000000AA, /* 0x04 */
	0x00AA00AA, /* 0x05 */
	0x000055AA, /* 0x06 */
	0x00AAAAAA, /* 0x07 */
	0x00555555, /* 0x08 */
	0x00FF5555, /* 0x09 */
	0x0055FF55, /* 0x0A */
	0x00FFFF55, /* 0x0B */
	0x005555FF, /* 0x0C */
	0x00FF55FF, /* 0x0D */
	0x0055FFFF, /* 0x0E */
	0x00FFFFFF  /* 0x0F */
};

void vga_color(int i, unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned long c = (i >= 0 && i <= VGA_COLORS) ? vga_colors[i] : 0;

	*r = (unsigned char)(c & 0xFF);
	*g = (unsigned char)((c >> 8) & 0xFF);
	*b = (unsigned char)((c >> 16) & 0xFF);
}

unsigned vga_find_color(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned long nearest_dist = 0xFFFFFFFFul;
	unsigned i, ret = 0;

	for (i = 0; i < VGA_COLORS; i++) {
		unsigned long c = vga_colors[i];
		unsigned long vga_r = (unsigned char)(c & 0xFF);
		unsigned long vga_g = (unsigned char)((c >> 8) & 0xFF);
		unsigned long vga_b = (unsigned char)((c >> 16) & 0xFF);
		unsigned long dist;

		vga_r = (vga_r > r) ? (vga_r - r) : (r - vga_r);
		vga_g = (vga_g > g) ? (vga_g - g) : (g - vga_g);
		vga_b = (vga_r > b) ? (vga_b - b) : (b - vga_b);

		dist = (vga_r * vga_r) + (vga_g * vga_g) + (vga_b * vga_b);

		if (dist == 0)
			return i;

		if (nearest_dist > dist) {
			nearest_dist = dist;
			ret = i;
		}
	}

	return ret;
}
