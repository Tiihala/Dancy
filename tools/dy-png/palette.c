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
 * dy-png/palette.c
 *      Convert .bmp files to Dancy-compatible .png files
 */

#include "program.h"

static unsigned pl_ncolors = 0;
static unsigned long pl_colors[256];

void pl_color(int i, unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned long c = (i >= 0 && i <= (int)pl_ncolors) ? pl_colors[i] : 0;

	*r = (unsigned char)(c & 0xFF);
	*g = (unsigned char)((c >> 8) & 0xFF);
	*b = (unsigned char)((c >> 16) & 0xFF);
}

unsigned pl_find_color(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned long nearest_dist = 0xFFFFFFFFul;
	unsigned i, ret = 0;

	for (i = 0; i < pl_ncolors; i++) {
		unsigned long c = pl_colors[i];
		unsigned long pl_r = (unsigned char)(c & 0xFF);
		unsigned long pl_g = (unsigned char)((c >> 8) & 0xFF);
		unsigned long pl_b = (unsigned char)((c >> 16) & 0xFF);
		unsigned long dist;

		pl_r = (pl_r > r) ? (pl_r - r) : (r - pl_r);
		pl_g = (pl_g > g) ? (pl_g - g) : (g - pl_g);
		pl_b = (pl_r > b) ? (pl_b - b) : (b - pl_b);

		dist = (pl_r * pl_r) + (pl_g * pl_g) + (pl_b * pl_b);

		if (dist == 0)
			return i;

		if (nearest_dist > dist) {
			nearest_dist = dist;
			ret = i;
		}
	}

	return ret;
}

int pl_init(struct options *opt)
{
	static const char *err_msg = "Error: unsupported gpl-file\n";
	unsigned char *gpl_data = NULL;
	char *line = NULL;
	size_t gpl_size = 0;
	size_t i, j;

	if (!opt->arg_p)
		return 0;

	if (read_file(opt->arg_p, &gpl_data, &gpl_size))
		return 1;

	if (gpl_size > 0x10000) {
		fputs(err_msg, stderr);
		return free(gpl_data), 1;
	}

	line = (char *)gpl_data;

	for (i = 0; i < gpl_size; i++) {
		char c = (char)((gpl_data[i] < 0x80) ? gpl_data[i] : 0u);
		int skip = 0;

		if (c == '\0') {
			fputs(err_msg, stderr);
			return free(gpl_data), 1;
		}
		if (c == '\t' || c == '\r')
			gpl_data[i] = 0;

		if (c != '\n')
			continue;

		gpl_data[i] = 0;

		if (line == (char *)gpl_data) {
			if (!strstr(line, "GIMP Palette")) {
				fputs(err_msg, stderr);
				return free(gpl_data), 1;
			}
			skip = 1;
		}

		if (!skip && strstr(line, "Name:"))
			skip = 1;

		if (!skip && strstr(line, "Columns:"))
			skip = 1;

		if (!skip && line[0] == '#')
			skip = 1;

		if (!skip) {
			char *next = NULL;
			unsigned long r, g, b;
			int digits_found = 0;

			for (j = 0; line[j] != '\0'; j++) {
				if (line[j] == ' ')
					continue;
				if (!isdigit((int)line[j])) {
					digits_found = 0;
					break;
				}
				digits_found += 1;
			}

			if (!digits_found) {
				fputs(err_msg, stderr);
				return free(gpl_data), 1;
			}

			r = strtoul(line, &next, 10);
			g = strtoul(next, &next, 10);
			b = strtoul(next, &next, 10);

			if (r > 255 || g > 255 || b > 255) {
				fputs(err_msg, stderr);
				return free(gpl_data), 1;
			}

			pl_colors[pl_ncolors++] = r | (g << 8) | (b << 16);

			if (pl_ncolors > 256) {
				fputs("Error: too many colors\n", stderr);
				return free(gpl_data), 1;
			}
		}

		line = (char *)&gpl_data[i + 1];
	}

	opt->pl_ncolors = (int)pl_ncolors;

	return free(gpl_data), 0;
}
