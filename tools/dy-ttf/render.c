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
 * dy-ttf/render.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static unsigned long raw_width;
static unsigned long raw_height;

static size_t raw_size;
static unsigned char *raw_data;

static int write_bmp_file(struct options *opt)
{
	size_t row_size = (size_t)(raw_width * 4);
	size_t size = raw_size + 138;
	unsigned char *out, *p;
	unsigned long i;

	out = malloc(size);
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;

	memset(out, 0, size);

	W_LE16(&out[0x00], 0x4D42ul);
	W_LE32(&out[0x02], (unsigned long)size);
	W_LE32(&out[0x0A], 0x0000008Aul);
	W_LE32(&out[0x0E], 0x0000007Cul);
	W_LE32(&out[0x12], (unsigned long)raw_width);
	W_LE32(&out[0x16], (unsigned long)raw_height);
	W_LE16(&out[0x1A], 0x0001ul);
	W_LE16(&out[0x1C], 0x0020ul);
	W_LE32(&out[0x1E], 0x00000003ul);
	W_LE32(&out[0x22], (unsigned long)raw_size);
	W_LE32(&out[0x36], 0x000000FFul);
	W_LE32(&out[0x3A], 0x0000FF00ul);
	W_LE32(&out[0x3E], 0x00FF0000ul);
	W_LE32(&out[0x42], 0xFF000000ul);
	W_LE32(&out[0x46], 0x73524742ul);
	W_LE32(&out[0x7A], 0x00000002ul);

	p = out + 138;

	for (i = raw_height; i > 0; i--) {
		unsigned char *line = raw_data + ((i - 1) * row_size);

		memcpy(p, line, row_size);
		p += row_size;
	}

	if (write_file(opt, out, size))
		return free(out), 1;

	return free(out), 0;
}

static void draw_point(int bold, long x, long y, unsigned long color)
{
	long offset = (long)((bold > 0 && bold < 10) ? bold : 0);
	unsigned char *p1, *p2;
	long i, j;

	if (x < offset || (unsigned long)(x + offset) >= raw_width)
		return;
	if (y < offset || (unsigned long)(y + offset) >= raw_height)
		return;

	y = raw_height - y - 1;

	for (i = -offset; i <= offset; i++) {
		p1 = raw_data + ((unsigned long)(y + i) * raw_width * 4);

		for (j = -offset; j <= offset; j++) {
			p2 = p1 + ((unsigned long)(x + j) * 4);
			W_LE32(&p2[0], color);
		}
	}
}

static void draw_line(long x0, long y0, long x1, long y1)
{
	const unsigned long color = 0xFFFFFFFFul;
	long dx, dy;
	long sx, sy;
	long t1, t2;

	if (x0 < x1)
		dx = (x1 - x0), sx = 1;
	else
		dx = (x0 - x1), sx = -1;

	if (y0 < y1)
		dy = -(y1 - y0), sy = 1;
	else
		dy = -(y0 - y1), sy = -1;

	t1 = dx + dy;

	for (;;) {
		t2 = t1 * 2;
		draw_point(1, x0, y0, color);

		if (t2 >= dy) {
			if (x0 == x1)
				break;
			x0 += sx;
			t1 += dy;
		}
		if (t2 <= dx) {
			if (y0 == y1)
				break;
			y0 += sy;
			t1 += dx;
		}
	}
}

int render(struct options *opt, unsigned long points, struct glyf *array)
{
	long xmin = 0, ymin = 0, xmax = 0, ymax = 0;
	long x_off, y_off;
	unsigned long i, j;
	int ret;

	if (points != 0) {
		xmin = array[0].x;
		ymin = array[0].y;
		xmax = array[0].x;
		ymax = array[0].y;

		for (i = 0; i < points; i++) {
			if (xmin > array[i].x)
				xmin = array[i].x;
			if (ymin > array[i].y)
				ymin = array[i].y;

			if (xmax < array[i].x)
				xmax = array[i].x;
			if (ymax < array[i].y)
				ymax = array[i].y;
		}
	}

	x_off = 32 - xmin;
	y_off = 32 - ymin;

	/*
	 * Print contour information in "ttx" format.
	 */
	if (opt->verbose) {
		char name[2] = { '\0', '\0' };
		int state = 0;

		if (opt->code_point > 0x20 && opt->code_point < 0x7F)
			name[0] = (char)opt->code_point;

		printf("\n<TTGlyph name=\"%s\"", &name[0]);
		printf(" xMin=\"%ld\"", xmin);
		printf(" yMin=\"%ld\"", ymin);
		printf(" xMax=\"%ld\"", xmax);
		printf(" yMax=\"%ld\"", ymax);
		printf(">\n");

		for (i = 0; i < points; i++) {
			if (state == 0) {
				printf("  <contour>\n");
				state = 1;
			}

			printf("%7s", "<pt");
			printf(" x=\"%ld\"", array[i].x);
			printf(" y=\"%ld\"", array[i].y);
			printf(" on=\"%u\"", (unsigned)(array[i].flag & 1));
			printf("/>\n");

			if ((array[i].flag & 0x0100ul) != 0) {
				printf("  </contour>\n");
				state = 0;
			}
		}
		if (state == 1)
			printf("  </contour><!-- ERROR -->\n");
		printf("</TTGlyph\n");
	}

	raw_width = (unsigned long)(xmax - xmin) + 64;
	raw_height = (unsigned long)(ymax - ymin) + 64;

	if (raw_width > 8192 || raw_height > 8192)
		return fputs("Error: rendering size\n", stderr), 1;

	raw_size = (raw_width * raw_height * 4);
	raw_data = malloc(raw_size);

	if (raw_data == NULL)
		return fputs("Error: not enough memory\n", stderr), 1;

	for (i = 0; i < raw_height; i++) {
		unsigned char *line = raw_data + (i * raw_width * 4);
		for (j = 0; j < raw_width; j++) {
			unsigned char *p = line + (j * 4);
			W_LE32(&p[0], 0xFF000000ul);
		}
	}

	if (points != 0) {
		unsigned long contour_start = 0;

		for (i = 1; i <= points; i++) {
			long x0 = x_off + array[i - 1].x;
			long y0 = y_off + array[i - 1].y;
			long x1 = (i < points) ? x_off + array[i - 0].x : 0;
			long y1 = (i < points) ? y_off + array[i - 0].y : 0;

			if ((array[i - 1].flag & 0x0100) != 0) {
				x1 = x_off + array[contour_start].x;
				y1 = y_off + array[contour_start].y;
				contour_start = i;
			}
			draw_line(x0, y0, x1, y1);
		}
	}

	for (i = 0; i < points; i++) {
		long x = x_off + array[i].x;
		long y = y_off + array[i].y;
		int err;

		if ((array[i].flag & 1ul) != 0)
			draw_point(4, x, y, 0xFF00AA00ul);
		else
			draw_point(4, x, y, 0xFF0000AAul);
	}

	if ((ret = write_bmp_file(opt)) != 0)
		return free(raw_data), ret;

	return free(raw_data), 0;
}
