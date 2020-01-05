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
 * dy-ttf/render.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static unsigned long raw_width;
static unsigned long raw_height;

static size_t raw_size;
static unsigned char *raw_data;

static unsigned long vga_colors[16] = {
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

static int write_bmp_file(struct options *opt)
{
	size_t size = (raw_size * 4) + 138;
	unsigned char *out, *p;
	unsigned long i, j;

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
	W_LE32(&out[0x22], (unsigned long)(raw_size * 4));
	W_LE32(&out[0x36], 0x000000FFul);
	W_LE32(&out[0x3A], 0x0000FF00ul);
	W_LE32(&out[0x3E], 0x00FF0000ul);
	W_LE32(&out[0x42], 0xFF000000ul);
	W_LE32(&out[0x46], 0x73524742ul);
	W_LE32(&out[0x7A], 0x00000002ul);

	p = out + 138;

	for (i = raw_height; i > 0; i--) {
		unsigned char *line = raw_data + ((i - 1) * raw_width);

		for (j = 0; j < raw_width; j++) {
			unsigned long color = vga_colors[(line[j] & 0x0Fu)];

			color |= 0xFF000000ul;
			W_LE32(&p[0], color);
			p += 4;
		}
	}

	if (write_file(opt, out, size))
		return free(out), 1;

	return free(out), 0;
}

static void draw_point(long x, long y, unsigned color)
{
	unsigned char *p1, *p2;

	if (x < 0 || (unsigned long)x >= raw_width)
		return;
	if (y < 0 || (unsigned long)y >= raw_height)
		return;

	y = (long)raw_height - y - 1;

	p1 = raw_data + ((unsigned long)y * raw_width);
	p2 = p1 + (unsigned long)x;

	/*
	 * Color indices have been carefully selected
	 * to make this "or" logic work as intended.
	 */
	p2[0] = (unsigned char)((unsigned)p2[0] | color);
}

static void draw_point_bold(long x, long y, unsigned color)
{
	const long offset = 2;
	unsigned char *p1, *p2;
	long i, j;

	if (x < offset || (unsigned long)(x + offset) >= raw_width)
		return;
	if (y < offset || (unsigned long)(y + offset) >= raw_height)
		return;

	y = (long)raw_height - y - 1;

	for (i = -offset; i <= offset; i++) {
		p1 = raw_data + ((unsigned long)(y + i) * raw_width);

		for (j = -offset; j <= offset; j++) {
			p2 = p1 + (unsigned long)(x + j);
			p2[0] = (unsigned char)color;
		}
	}
}

static void draw_line(long x0, long y0, long x1, long y1, int y_dir)
{
	unsigned color = ((y_dir == 0) ? 0x04 : ((y_dir < 0) ? 0x05 : 0x06));
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
		draw_point(x0, y0, color);

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

static void draw_bezier(long x0, long y0, long x1, long y1, long x2, long y2)
{
	const long points = 10;
	double prev_y = (double)y0;

	long line_x0, line_y0;
	long line_x1, line_y1;
	int y_dir;
	long i;

	line_x0 = x0;
	line_y0 = y0;

	if (y0 == y1 && y1 == y2)
		y_dir = 0;
	else if (y0 < y1 || (y0 == y1 && y1 < y2))
		y_dir = -1;
	else
		y_dir = 1;

	for (i = 1; i < points; i++) {
		double t = ((double)i / (double)points);

		double x = (((1.0 - t) * (1.0 - t)) * (double)x0
			+ 2.0 * (1.0 - t) * t * (double)x1
			+ (t * t) * (double)x2);

		double y = (((1.0 - t) * (1.0 - t)) * (double)y0
			+ 2.0 * (1.0 - t) * t * (double)y1
			+ (t * t) * (double)y2);

		line_x1 = (long)x;
		line_y1 = (long)y;

		if (y_dir != 0)
			y_dir = (prev_y < y) ? -1 : 1;

		draw_line(line_x0, line_y0, line_x1, line_y1, y_dir);
		prev_y = y;

		line_x0 = line_x1;
		line_y0 = line_y1;
	}

	line_x1 = x2;
	line_y1 = y2;

	draw_line(line_x0, line_y0, line_x1, line_y1, y_dir);
}

static void fill_contours(void)
{
	unsigned long i, j, k;

	for (i = 0; i < raw_height; i++) {
		unsigned char *line = raw_data + (i * raw_width);

		for (j = 0; j < raw_width; j++) {
			if (line[j] != 0)
				break;
		}

		while (j < raw_width) {
			unsigned long size = 1;

			if (line[j] == 0) {
				unsigned color = 0;
				int add = 0, state = 0;

				for (k = j + 1; k < raw_width; k++) {
					if (line[k] != 0)
						break;
					size += 1;
				}

				for (k = j + 1; k < raw_width; k++) {
					if (color != 0 && line[k] == 0) {
						if (add != 0x7FFF) {
							if (add < 0)
								state -= 1;
							else if (add > 0)
								state += 1;
						}
						add = 0;
					}
					if (color != line[k]) {
						color = (unsigned)line[k];

						if (add != 0x7FFF) {
							if (color == 5)
								add -= 1;
							else if (color == 6)
								add += 1;
							else if (color == 7)
								add = 0x7FFF;
						}
					}
				}
				if (state != 0)
					memset(&line[j], 0x08, (size_t)size);
			}
			j += size;
		}
	}
}

static void midpoint(long x0, long y0, long x1, long y1, long *x, long *y)
{
	*x = x0 + ((x1 - x0) / 2);
	*y = y0 + ((y1 - y0) / 2);
}

int render_glyph(struct options *opt, struct render *glyph)
{
	struct glyf *array = glyph->array;
	unsigned long points = glyph->points;

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
		printf("</TTGlyph>\n");

		printf("\n<hmtx>\n  <mtx");
		printf(" name=\"%s\"", &name[0]);
		printf(" width=\"%lu\"", glyph->advance);
		printf(" lsb=\"%ld\"", glyph->lsb);
		printf("/>\n</hmtx>\n");
	}

	raw_width = (unsigned long)(xmax - xmin) + 64;
	raw_height = (unsigned long)(ymax - ymin) + 64;

	if (raw_width > 8192 || raw_height > 8192)
		return fputs("Error: rendering size\n", stderr), 1;

	raw_size = (raw_width * raw_height);
	raw_data = malloc(raw_size);

	if (raw_data == NULL)
		return fputs("Error: not enough memory\n", stderr), 1;

	memset(raw_data, 0, raw_size);

	if (points != 0) {
		unsigned long contour_start = 0;
		long x0, y0, x1, y1, x2, y2;

		for (i = 0; i < points; i++) {
			unsigned long flag = array[i].flag;
			struct glyf *prev = &array[((i > 0) ? i - 1 : 0)];
			struct glyf *next = &array[((i + 1) % points)];

			if (contour_start == i) {
				for (j = i; j < points; j++) {
					if ((prev->flag & 0x0100) != 0)
						break;
					prev = &array[j];
				}
			}

			if ((flag & 0x0100) != 0) {
				next = &array[contour_start];
				contour_start = i + 1;
			}

			if ((flag & 1) == 0) {
				x0 = prev->x;
				y0 = prev->y;
				x1 = array[i].x;
				y1 = array[i].y;

				if ((prev->flag & 1) == 0)
					midpoint(x0, y0, x1, y1, &x0, &y0);

				x2 = next->x;
				y2 = next->y;

				if ((next->flag & 1) == 0)
					midpoint(x1, y1, x2, y2, &x2, &y2);

				x0 += x_off, y0 += y_off;
				x1 += x_off, y1 += y_off;
				x2 += x_off, y2 += y_off;

				draw_bezier(x0, y0, x1, y1, x2, y2);

			} else if ((next->flag & 1) != 0) {
				int y_dir;

				x0 = x_off + array[i].x;
				y0 = y_off + array[i].y;
				x1 = x_off + next->x;
				y1 = y_off + next->y;

				y_dir = (y0 == y1) ? 0 : ((y0 < y1) ? -1 : 1);
				draw_line(x0, y0, x1, y1, y_dir);
			}
		}
	}

	fill_contours();

	for (i = 0; i < points; i++) {
		long x = x_off + array[i].x;
		long y = y_off + array[i].y;

		if ((array[i].flag & 1) != 0)
			draw_point_bold(x, y, 2);
		else
			draw_point_bold(x, y, 4);
	}

	if (ymin <= 0) {
		size_t offset = (unsigned long)(-ymin) * raw_width;

		offset = (raw_width * (raw_height - 33)) - offset;

		if (offset < raw_size - raw_width) {
			for (i = 16; i < raw_width - 16; i++) {
				if (raw_data[offset + i] == 0)
					raw_data[offset + i] = 14;
			}
		}
	}

	if ((ret = write_bmp_file(opt)) != 0)
		return free(raw_data), ret;

	return free(raw_data), 0;
}
