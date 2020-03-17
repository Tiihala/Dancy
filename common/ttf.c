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
 * ttf.c
 *      TrueType glyph rendering (Dancy TTF)
 */

#include <limits.h>
#include <stddef.h>

int ttf_create(void **instance);
int ttf_delete(void *ttf);

int ttf_open(void *ttf, size_t size, const void *ttf_file);
int ttf_set_bitmap(void *ttf, size_t size, void *bitmap);
int ttf_render(void *ttf, unsigned int code_point, unsigned int *width);

extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *memcpy(void *s1, const void *s2, size_t n);
extern void *memset(void *s, int c, size_t n);

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

#define B8(a,b,c) (((unsigned int)((a)[(b)]) & 0xFFu) << (c))
#define BE16(a) (B8((a),0,8) | B8((a),1,0))
#define BE32(a) (B8((a),0,24) | B8((a),1,16) | B8((a),2,8) | B8((a),3,0))

#define W_BE16(a,d) ( \
	*((a) + 1) = (unsigned char)(((unsigned int)(d) >> 0) & 0xFFu), \
	*((a) + 0) = (unsigned char)(((unsigned int)(d) >> 8) & 0xFFu))

#define TTF_TABLE_CMAP (0x636D6170u)
#define TTF_TABLE_GLYF (0x676C7966u)
#define TTF_TABLE_HEAD (0x68656164u)
#define TTF_TABLE_HHEA (0x68686561u)
#define TTF_TABLE_HMTX (0x686D7478u)
#define TTF_TABLE_LOCA (0x6C6F6361u)
#define TTF_TABLE_MAXP (0x6D617870u)

struct ttf_glyph {
	unsigned int code_pnt;
	unsigned int loca_idx;
	unsigned int adv_width;

	unsigned int size;
	unsigned char *data;
};

struct ttf_instance {
	struct ttf_glyph *glyph_array;
	unsigned char *bitmap;
	unsigned char *buffer;
	unsigned char *glyf;

	size_t bitmap_size;
	size_t glyf_size;

	unsigned int em_value;
	unsigned int glyph_entries;
	unsigned int maxp_glyphs;
};

#define this_ttf ((struct ttf_instance *)(ttf))

int ttf_create(void **instance)
{
	struct ttf_instance *ttf;
	const size_t buffer_size = 65536;
	unsigned char *buffer;

	ttf = malloc(sizeof(*ttf));
	if ((*instance = ttf) == NULL)
		return 1;

	buffer = malloc(buffer_size);
	if (buffer == NULL)
		return free(ttf), 1;

	memset(ttf, 0, sizeof(*ttf));
	memset(buffer, 0, buffer_size);

	ttf->glyph_array = NULL;
	ttf->bitmap = NULL;
	ttf->buffer = buffer;
	ttf->glyf = NULL;

	return 0;
}

int ttf_delete(void *ttf)
{
	if (ttf == NULL)
		return 1;

	free(this_ttf->glyph_array);
	free(this_ttf->buffer);
	free(this_ttf->glyf);

	free(ttf);
	return 0;
}

static int handle_head(void *ttf, size_t size, const unsigned char *table)
{
	unsigned int xmin, ymin, xmax, ymax;

	if (size != 54 || BE16(&table[18]) != 2048)
		return 1;

	xmin = BE16(&table[36]);
	ymin = (BE16(&table[38]) + 512) & 0xFFFFu;
	xmax = BE16(&table[40]);
	ymax = (BE16(&table[42]) + 512) & 0xFFFFu;

	if (xmin > xmax || xmax > 2048)
		return 1;
	if (ymin > ymax || ymax > 2048)
		return 1;

	if (BE16(&table[50]) != 1)
		return 1;

	return (void)ttf, 0;
}

static int handle_maxp(void *ttf, size_t size, const unsigned char *table)
{
	unsigned int glyphs = (size == 32) ? BE16(&table[4]) : 0u;

	this_ttf->maxp_glyphs = glyphs;
	return (glyphs != 0) ? 0 : 1;
}

static int handle_hhea(void *ttf, size_t size, const unsigned char *table)
{
	if (size != 36 || BE16(&table[34]) != this_ttf->maxp_glyphs)
		return 1;
	return 0;
}

static int handle_cmap(void *ttf, size_t size, const unsigned char *table)
{
	const unsigned char *subtable = NULL;
	unsigned int i, val, offset = 0;

	struct cmap {
		unsigned int pnt;
		unsigned int idx;
	};

	struct cmap *cmap_array = NULL;
	unsigned int cmap_points = 0;

	val = BE16(&table[2]);
	if (size < (val * 8 + 4))
		return 1;

	for (i = 0; i < val; i++) {
		const unsigned char *p = &table[i * 8 + 4];

		if (BE32(&p[4]) > size - 4)
			return 1;

		if (BE16(&p[0]) == 0) {
			unsigned int cmap_off = BE32(&p[4]);
			const unsigned char *cmap_sub = table + cmap_off;

			if (BE16(&cmap_sub[0]) == 12) {
				offset = cmap_off;
				subtable = cmap_sub;
				break;
			}
		}
	}

	if (subtable != NULL) {
		unsigned int add, len, groups;

		if (BE16(&subtable[2]) != 0)
			return 1;
		if (offset > size - 16)
			return 1;

		len = BE32(&subtable[4]);
		if (len < 16 || len > size - offset)
			return 1;

		groups = BE32(&subtable[12]);
		if (groups == 0 || ((len - 16) / 12) < groups)
			return 1;

		for (i = 0; i < groups; i++) {
			const unsigned char *p = subtable + (i * 12) + 16;
			unsigned int t1, t2;

			t1 = BE32(&p[0]);
			t2 = BE32(&p[4]);

			if (t1 > t2)
				return 1;

			add = (t2 - t1) + 1;
			if (!(cmap_points < UINT_MAX - add))
				return 1;

			cmap_points += add;
		}

		if (cmap_points == 0 || cmap_points > 0x10FFFF)
			return 1;

		cmap_array = malloc(cmap_points * sizeof(struct cmap));
		if (cmap_array == NULL)
			return 1;

		for (add = 0, i = 0; i < groups; i++) {
			const unsigned char *p = subtable + (i * 12) + 16;
			unsigned int t1, t2;
			unsigned int c, glyph_idx;

			t1 = BE32(&p[0]);
			t2 = BE32(&p[4]);
			glyph_idx = BE32(&p[8]);

			for (c = t1; c <= t2; c++) {
				if (glyph_idx >= this_ttf->maxp_glyphs)
					return free(cmap_array), 1;

				if (add >= cmap_points)
					return free(cmap_array), 1;

				cmap_array[add].pnt = c;
				cmap_array[add].idx = glyph_idx++;
				add += 1;
			}
		}

		if (add != cmap_points)
			return free(cmap_array), 1;
	}

	if (cmap_points == 0)
		return free(cmap_array), 1;

	size = (cmap_points + 1) * sizeof(struct ttf_glyph);

	this_ttf->glyph_array = malloc(size);
	if (this_ttf->glyph_array == NULL)
		return free(cmap_array), 1;

	memset(this_ttf->glyph_array, 0, size);
	this_ttf->glyph_array[0].data = NULL;

	for (i = 0; i < cmap_points; i++) {
		this_ttf->glyph_array[i + 1].code_pnt = cmap_array[i].pnt;
		this_ttf->glyph_array[i + 1].loca_idx = cmap_array[i].idx;
		this_ttf->glyph_array[i + 1].data = NULL;
	}

	this_ttf->glyph_entries = cmap_points + 1;
	return free(cmap_array), 0;
}

static int handle_glyf(void *ttf, size_t size, const unsigned char *table)
{
	this_ttf->glyf = (size != 0) ? malloc(size) : NULL;
	if (this_ttf->glyf == NULL)
		return 1;

	memcpy(this_ttf->glyf, table, size);

	this_ttf->glyf_size = size;
	return 0;
}

static int handle_loca(void *ttf, size_t size, const unsigned char *table)
{
	unsigned char *glyf = this_ttf->glyf;
	size_t glyf_size = this_ttf->glyf_size;

	unsigned int glyph_entries = this_ttf->glyph_entries;
	unsigned int loca_entries = (unsigned int)(size / 4);
	unsigned int i;

	if (loca_entries != (this_ttf->maxp_glyphs + 1))
		return 1;

	for (i = 0; i < glyph_entries; i++) {
		unsigned int loca_idx = this_ttf->glyph_array[i].loca_idx;
		const unsigned char *p = table;
		unsigned int t1, t2;

		if (loca_entries < loca_idx + 1)
			return 1;

		p += loca_idx * 4;

		t1 = BE32(&p[0]);
		t2 = BE32(&p[4]);

		if (t1 > t2)
			return 1;
		if (t2 > glyf_size)
			return 1;

		this_ttf->glyph_array[i].data = &glyf[t1];
		this_ttf->glyph_array[i].size = t2 - t1;
	}
	return 0;
}

static int handle_hmtx(void *ttf, size_t size, const unsigned char *table)
{
	unsigned int glyph_entries = this_ttf->glyph_entries;
	unsigned int maxp_glyphs = this_ttf->maxp_glyphs;
	unsigned int i;

	if (size == 0 || size != maxp_glyphs * 4)
		return 1;

	for (i = 0; i < glyph_entries; i++) {
		const unsigned char *p = table;
		unsigned int loca_idx = this_ttf->glyph_array[i].loca_idx;

		if (loca_idx >= maxp_glyphs)
			return 1;

		p += loca_idx * 4;
		this_ttf->glyph_array[i].adv_width = BE16(&p[0]);
	}
	return 0;
}

int ttf_open(void *ttf, size_t size, const void *ttf_file)
{
	const unsigned char *p = ttf_file;
	struct {
		unsigned int name;
		int (*func)(void *, size_t, const unsigned char *);
	} tables[] = {
		{ TTF_TABLE_HEAD, handle_head },
		{ TTF_TABLE_MAXP, handle_maxp },
		{ TTF_TABLE_HHEA, handle_hhea },
		{ TTF_TABLE_CMAP, handle_cmap },
		{ TTF_TABLE_GLYF, handle_glyf },
		{ TTF_TABLE_LOCA, handle_loca },
		{ TTF_TABLE_HMTX, handle_hmtx }
	};
	int handled = 0;
	size_t i, j;

	if (this_ttf->glyph_array != NULL)
		free(this_ttf->glyph_array);

	this_ttf->glyph_array = NULL;
	this_ttf->glyph_entries = 0;

	if (this_ttf->glyf != NULL)
		free(this_ttf->glyf);

	this_ttf->glyf = NULL;
	this_ttf->glyf_size = 0;

	if (size < 156 || p == NULL)
		return this_ttf->glyph_entries = 0, 1;

	if (BE32(&p[0]) != 0x00010000 || BE16(&p[4]) != 0x0009)
		return this_ttf->glyph_entries = 0, 1;

	for (i = 0; i < sizeof(tables) / sizeof(tables[0]); i++) {
		for (j = 12; j < 156; j++) {
			if (BE32(&p[j]) == tables[i].name) {
				unsigned int off = BE32(&p[j + 8]);
				unsigned int len = BE32(&p[j + 12]);

				if (!(off < UINT_MAX - len))
					return this_ttf->glyph_entries = 0, 1;
				if (off + len > size)
					return this_ttf->glyph_entries = 0, 1;

				if (tables[i].func(ttf, (size_t)len, p + off))
					return this_ttf->glyph_entries = 0, 2;

				handled += 1;
				break;
			}
		}
	}

	if (handled != (int)(sizeof(tables) / sizeof(tables[0])))
		return this_ttf->glyph_entries = 0, 3;
	return 0;
}

int ttf_set_bitmap(void *ttf, size_t size, void *bitmap)
{
	size_t em_value;

	for (em_value = 16; em_value <= 2048; em_value++) {
		if (em_value * em_value == size) {
			this_ttf->bitmap = bitmap;
			this_ttf->bitmap_size = size;
			this_ttf->em_value = (unsigned int)em_value;
			return 0;
		}
	}

	this_ttf->bitmap = NULL;
	this_ttf->bitmap_size = 0;
	this_ttf->em_value = 0;
	return 1;
}

static void bresenham(void *ttf, int x0, int y0, int x1, int y1, int y_dir)
{
	unsigned int color = ((y_dir == 0) ? 4u : ((y_dir < 0) ? 5u : 6u));
	int dx, dy;
	int sx, sy;
	int t1, t2;

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
		unsigned int x = (unsigned int)x0 / 8u;
		unsigned int y = (unsigned int)y0 / 8u;
		unsigned int offset = (x + (y * 256u)) & 0xFFFFu;
		unsigned int c = this_ttf->buffer[offset];

		this_ttf->buffer[offset] = (unsigned char)(c | color);

		t2 = t1 * 2;

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

static void bezier(void *ttf, int x0, int y0, int x1, int y1, int x2, int y2)
{
	int prev_y = y0 * 1024;

	int line_x0 = x0, line_y0 = y0;
	int line_x1, line_y1;

	int y_dir = 1;
	int i;

	if (y0 == y1 && y1 == y2)
		y_dir = 0;
	else if (y0 < y1 || (y0 == y1 && y1 < y2))
		y_dir = -1;

	for (i = 1; i < 32; i++) {
		int t1 = (i * 32) / 32;
		int t2 = 32 - t1;
		int t3 = t2 * t2;
		int t4 = 2 * t2 * t1;
		int t5 = t1 * t1;

		int x = t3 * x0 + t4 * x1 + t5 * x2;
		int y = t3 * y0 + t4 * y1 + t5 * y2;

		line_x1 = x / 1024;
		line_y1 = y / 1024;

		if (y_dir != 0)
			y_dir = (prev_y < y) ? -1 : 1;

		bresenham(ttf, line_x0, line_y0, line_x1, line_y1, y_dir);
		prev_y = y;

		line_x0 = line_x1;
		line_y0 = line_y1;
	}

	line_x1 = x2;
	line_y1 = y2;

	bresenham(ttf, line_x0, line_y0, line_x1, line_y1, y_dir);
}

static int draw_glyph(void *ttf, int points, const unsigned char *p)
{
	const unsigned char *px = p + points;
	const unsigned char *py = p + (points * 3);

	int contour_start = 0;
	int x0, y0, x1, y1, x2, y2;
	int i, j;

	memset(this_ttf->buffer, 0, 65536);

	for (i = 0; i < points; i++) {
		unsigned int flag = p[i];
		int prev = (i > 0) ? i - 1 : 0;
		int next = (i + 1) % points;

		if (contour_start == i) {
			for (j = i; j < points; j++) {
				if ((p[prev] & 0x80u) != 0)
					break;
				prev = j;
			}
		}

		if ((flag & 0x80u) != 0) {
			next = contour_start;
			contour_start = i + 1;
		}

		if ((flag & 0x01u) == 0) {
			x0 = (int)BE16(&px[prev * 2]);
			y0 = (int)BE16(&py[prev * 2]);
			x1 = (int)BE16(&px[i * 2]);
			y1 = (int)BE16(&py[i * 2]);

			if ((p[prev] & 0x01u) == 0) {
				x0 = x0 + ((x1 - x0) / 2);
				y0 = y0 + ((y1 - y0) / 2);
			}

			x2 = (int)BE16(&px[next * 2]);
			y2 = (int)BE16(&py[next * 2]);

			if ((p[next] & 0x01u) == 0) {
				x2 = x1 + ((x2 - x1) / 2);
				y2 = y1 + ((y2 - y1) / 2);
			}

			bezier(ttf, x0, y0, x1, y1, x2, y2);

		} else if ((p[next] & 0x01u) != 0) {
			int y_dir;

			x0 = (int)BE16(&px[i * 2]);
			y0 = (int)BE16(&py[i * 2]);
			x1 = (int)BE16(&px[next * 2]);
			y1 = (int)BE16(&py[next * 2]);

			y_dir = (y0 == y1) ? 0 : ((y0 < y1) ? -1 : 1);
			bresenham(ttf, x0, y0, x1, y1, y_dir);
		}
	}
	return 0;
}

static void fill_contours(void *ttf)
{
	unsigned int i, j, k;

	for (i = 0; i < 256; i++) {
		unsigned char *line = this_ttf->buffer + (i * 256);

		for (j = 0; j < 256; j++) {
			if (line[j] != 0)
				break;
		}

		while (j < 256) {
			unsigned int size = 1;

			if (line[j] == 0) {
				unsigned int color = 0;
				int add = 0, state = 0;

				for (k = j + 1; k < 256; k++) {
					if (line[k] != 0)
						break;
					size += 1;
				}

				for (k = j + 1; k < 256; k++) {
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
						color = (unsigned int)line[k];

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

int ttf_render(void *ttf, unsigned int code_point, unsigned int *width)
{
	unsigned int em_value = this_ttf->em_value;
	unsigned int glyph_entries = this_ttf->glyph_entries;

	unsigned int is_processed = 0;
	unsigned int offset = 0, last_point = 0;

	unsigned int adv_width;
	unsigned int contours;
	unsigned int i;

	unsigned char *glyph;
	size_t size;

	if (width != NULL)
		*width = 0;

	if (this_ttf->bitmap_size != 0)
		memset(this_ttf->bitmap, 0, this_ttf->bitmap_size);

	if (this_ttf->glyph_entries == 0)
		return 1;

	glyph = this_ttf->glyph_array[0].data;
	size = this_ttf->glyph_array[0].size;
	adv_width = this_ttf->glyph_array[0].adv_width;

	for (i = 1; i < glyph_entries; i++) {
		if (this_ttf->glyph_array[i].code_pnt == code_point) {
			glyph = this_ttf->glyph_array[i].data;
			size = this_ttf->glyph_array[i].size;
			adv_width = this_ttf->glyph_array[i].adv_width;
			break;
		}
	}

	if (adv_width > 2048 || em_value == 0 || em_value > 2048)
		return 1;

	if (width != NULL)
		*width = (adv_width * em_value) / 2048;

	if (size < 2 || this_ttf->bitmap_size == 0)
		return 0;

	if ((contours = BE16(&glyph[0])) >= 0x8000ul)
		return 1;
	if (size < (12 + contours * 2))
		return 1;

	if (contours == 0)
		return 0;

	offset += 10;

	/*
	 * Find the last point index.
	 */
	for (i = 0; i < contours; i++) {
		unsigned char *p = &glyph[offset];

		if (last_point < BE16(&p[0]))
			last_point = BE16(&p[0]);
		offset += 2;
	}

	if (last_point == 0)
		return 0;

	/*
	 * Check instructions.
	 */
	if (BE16(&glyph[offset]) != 0)
		return 1;
	offset += 2;

	if (size != offset + (last_point + 1) * 5)
		return 1;

	/*
	 * Check flags. Bit 7 is used internally.
	 */
	for (i = 0; i <= last_point; i++) {
		unsigned int flg = glyph[offset + i];

		if ((flg & 0x80u) != 0) {
			is_processed = 1;
			flg &= 0x7Fu;
		}
		if (flg > 1)
			return 1;
	}

	/*
	 * Use bit 7 for "last points of each contour" and translate all
	 * point coordinates. If X or Y is exactly 2048, it is changed to
	 * 2040 so that the range is from 0 to 2040 and when divided by 8,
	 * the ranges are from 0 to 255.
	 *
	 * If x = 255 and y = 255,
	 *
	 *     buffer[x   + (y   * 256)] = value
	 *     buffer[255 + (255 * 256)] = value
	 *
	 *     buffer[0xFFFF] = value
	 *
	 */
	if (is_processed == 0) {
		unsigned int points = last_point + 1;
		unsigned int x = 0, y = 512;

		for (i = 0; i < contours; i++) {
			unsigned char *p = glyph + (10 + (i * 2));
			unsigned int pnt = BE16(&p[0]);
			unsigned int flg = glyph[offset + pnt];

			glyph[offset + pnt] = (unsigned char)(flg | 0x80u);
		}

		for (i = 0; i < points; i++) {
			unsigned char *px, *py;
			unsigned int wx, wy;

			px = glyph + (offset + (points * 1) + (i * 2));
			wx = x = (x + BE16(&px[0])) & 0xFFFFu;

			py = glyph + (offset + (points * 3) + (i * 2));
			wy = y = (y + BE16(&py[0])) & 0xFFFFu;

			if ((x & 0xF807u) != 0) {
				if (x != 2048)
					return glyph[offset] = 0x7F, 1;
				wx = 2040;
			}
			if ((y & 0xF807u) != 0) {
				if (y != 2048)
					return glyph[offset] = 0x7F, 1;
				wy = 2040;
			}

			W_BE16(px, wx);
			W_BE16(py, wy);
		}
	}

	if (draw_glyph(ttf, (int)(last_point + 1), &glyph[offset]))
		return 1;

	fill_contours(ttf);

	/*
	 * Copy the glyph image.
	 */
	{
		const unsigned char *src = this_ttf->buffer;
		unsigned char *dst = this_ttf->bitmap;

		int x_em = (int)(adv_width * em_value) / 2048;
		int y_em = (int)em_value;

		unsigned int x_off, y_off;
		int x, y;

		for (y = y_em - 1; y >= 0; y--) {
			for (x = 0; x < x_em; x++) {
				x_off = ((unsigned int)x * 256) / em_value;
				y_off = ((unsigned int)y * 256) / em_value;

				if (src[x_off + y_off * 256] != 0)
					dst[x] = 1;
			}
			dst += em_value;
		}
	}
	return 0;
}
