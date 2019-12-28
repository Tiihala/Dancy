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
 * dy-ttf/ttf.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static unsigned long ttf_head_size;
static unsigned long ttf_head_flags;
static unsigned long ttf_head_em;
static signed long   ttf_head_xmin;
static signed long   ttf_head_ymin;
static signed long   ttf_head_xmax;
static signed long   ttf_head_ymax;
static unsigned long ttf_head_lowest;
static unsigned long ttf_head_locfmt;

static unsigned long ttf_maxp_glyphs;

static struct cmap   *ttf_cmap_array;
static unsigned long ttf_cmap_points;

static struct loca   *ttf_loca_array;
static unsigned long ttf_loca_points;

static struct glyf   *ttf_glyf_array;
static unsigned long ttf_glyf_points;

static void ttf_dump(void)
{
	printf("%-20s%lu\n", "ttf_head_size:", ttf_head_size);
	printf("%-20s%04lX\n", "ttf_head_flags:", ttf_head_flags);
	printf("%-20s%lu\n", "ttf_head_em:", ttf_head_em);
	printf("\n");
	printf("%-20s%ld\n", "ttf_head_xmin:", ttf_head_xmin);
	printf("%-20s%ld\n", "ttf_head_ymin:", ttf_head_ymin);
	printf("%-20s%ld\n", "ttf_head_xmax:", ttf_head_xmax);
	printf("%-20s%ld\n", "ttf_head_ymax:", ttf_head_ymax);
	printf("\n");
	printf("%-20s%ld\n", "ttf_head_lowest:", ttf_head_lowest);
	printf("%-20s%ld\n", "ttf_head_locfmt:", ttf_head_locfmt);
	printf("\n");

	printf("%-20s%ld\n", "ttf_maxp_glyphs:", ttf_maxp_glyphs);
	printf("%-20s%ld\n", "ttf_loca_points:", ttf_loca_points);
	printf("\n");
}

static int ttf_read_head(void)
{
	unsigned char *table;
	unsigned long val;
	size_t size;

	if (table_find(TTF_TABLE_HEAD, &table, &size))
		return 1;

	ttf_head_size = (unsigned long)size;
	ttf_head_flags = BE16(&table[16]);
	ttf_head_em = BE16(&table[18]);

	val = BE16(&table[36]);
	ttf_head_xmin = BE16_TO_LONG(val);

	val = BE16(&table[38]);
	ttf_head_ymin = BE16_TO_LONG(val);

	val = BE16(&table[40]);
	ttf_head_xmax = BE16_TO_LONG(val);

	val = BE16(&table[42]);
	ttf_head_ymax = BE16_TO_LONG(val);

	ttf_head_lowest = BE16(&table[46]);
	ttf_head_locfmt = BE16(&table[50]);

	return 0;
}

static int ttf_read_maxp(void)
{
	unsigned char *table;
	size_t size;

	if (table_find(TTF_TABLE_MAXP, &table, &size))
		return 1;

	ttf_maxp_glyphs = BE16(&table[4]);

	if (ttf_maxp_glyphs == 0)
		return 1;

	return 0;
}

static int ttf_cmap_qsort(const void *a, const void *b)
{
	const struct cmap *cmap_a = a, *cmap_b = b;

	if (cmap_a->point > cmap_b->point)
		return 1;
	if (cmap_a->point < cmap_b->point)
		return -1;

	if (cmap_a->index > cmap_b->index)
		return 1;
	if (cmap_a->index < cmap_b->index)
		return -1;

	return 0;
}

static int ttf_read_cmap(void)
{
	unsigned char *table, *subtable = NULL;
	unsigned long i, val, offset = 0;
	size_t size;

	if (table_find(TTF_TABLE_CMAP, &table, &size))
		return 1;

	val = BE16(&table[2]);
	if (size < (val * 8 + 4))
		return 1;

	for (i = 0; i < val; i++) {
		const unsigned char *p = &table[i * 8 + 4];

		if (BE32(&p[4]) > size - 4)
			return 1;

		if (BE16(&p[0]) == 0) {
			offset = BE32(&p[4]);
			subtable = table + offset;
		}
	}

	/*
	 * Format 4
	 */
	if (subtable != NULL && BE16(&subtable[0]) == 4) {
		unsigned long len = BE16(&subtable[2]);
		unsigned long add, segs;

		if (len < 16 ||len > size - offset)
			return 1;

		segs = BE16(&subtable[6]) / 2;
		if (segs == 0 || len < (16 + (segs * 2) * 4))
			return 1;

		ttf_cmap_points = 0;

		for (i = 0; i < segs; i++) {
			const unsigned char *p;
			unsigned long t1, t2;

			p = subtable + (i * 2) + (segs * 2) + 16;
			t1 = BE16(&p[0]);

			p = subtable + (i * 2) + 14;
			t2 = BE16(&p[0]);

			if (t1 > t2)
				return 1;

			add = (t2 - t1) + 1;
			if (!(ttf_cmap_points < ULONG_MAX - add))
				return 1;

			ttf_cmap_points += add;
		}

		if (ttf_cmap_points == 0)
			return 1;

		ttf_cmap_array = calloc(ttf_cmap_points, sizeof(struct cmap));

		if (ttf_cmap_array == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return 1;
		}

		for (add = 0, i = 0; i < segs; i++) {
			const unsigned char *p;
			unsigned long t1, t2, t3, t4, t5;
			unsigned long c, glyph_index;

			p = subtable + (i * 2) + (segs * 2) + 16;
			t1 = BE16(&p[0]);

			p = subtable + (i * 2) + 14;
			t2 = BE16(&p[0]);

			p = subtable + (i * 2) + (segs * 4) + 16;
			t3 = BE16(&p[0]);

			t4 = (i * 2) + (segs * 6) + 16;
			p = subtable + t4;
			t5 = BE16(&p[0]);

			for (c = t1; c <= t2; c++) {
				if (t5) {
					unsigned long off;

					off = t4 + t5 + ((c - t1) * 2);
					if (off > len)
						return 1;

					p = subtable + off;
					glyph_index = BE16(&p[0]) + t3;
				} else {
					glyph_index = c + t3;
				}

				glyph_index &= 0xFFFFul;

				if (glyph_index >= ttf_maxp_glyphs)
					return 1;

				if (add >= ttf_cmap_points)
					return 1;

				ttf_cmap_array[add].point = c;
				ttf_cmap_array[add].index = glyph_index;
				add += 1;
			}
		}

		if (add != ttf_cmap_points)
			return 1;
	}

	if (ttf_cmap_array == NULL || ttf_cmap_points == 0)
		return 1;

	qsort(ttf_cmap_array, ttf_cmap_points,
		sizeof(struct cmap), ttf_cmap_qsort);

	return 0;
}

static int ttf_cmap_bsearch(const void *a, const void *b)
{
	const struct cmap *cmap_a = a, *cmap_b = b;

	if (cmap_a->point > cmap_b->point)
		return 1;
	if (cmap_a->point < cmap_b->point)
		return -1;

	return 0;
}

static unsigned long ttf_search_cmap(unsigned long point)
{
	const struct cmap *cmap_entry;
	struct cmap key;

	key.point = point, key.index = 0;

	if (ttf_cmap_array == NULL || ttf_cmap_points == 0)
		return 0;

	cmap_entry = bsearch(&key, ttf_cmap_array, ttf_cmap_points,
		sizeof(struct cmap), ttf_cmap_bsearch);

	return (cmap_entry != NULL) ? cmap_entry->index : 0;
}

static int ttf_read_loca(void)
{
	size_t entry_size = (ttf_head_locfmt != 0) ? 4 : 2;
	unsigned char *glyf_table, *loca_table;
	size_t glyf_size, loca_size;
	unsigned long i;

	if (table_find(TTF_TABLE_GLYF, &glyf_table, &glyf_size))
		return 1;
	if (table_find(TTF_TABLE_LOCA, &loca_table, &loca_size))
		return 1;

	ttf_loca_points = ttf_maxp_glyphs + 1;

	if (ttf_loca_points > (loca_size / entry_size))
		return 1;

	ttf_loca_array = calloc(ttf_loca_points, sizeof(struct loca));

	if (ttf_loca_array == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}

	for (i = 1; i < ttf_loca_points; i++) {
		const unsigned char *p1 = &loca_table[(i - 1) * entry_size];
		const unsigned char *p2 = &loca_table[(i - 0) * entry_size];
		unsigned long t1, t2;

		if (entry_size == 4) {
			t1 = BE32(&p1[0]);
			t2 = BE32(&p2[0]);
		} else {
			t1 = BE16(&p1[0]);
			t2 = BE16(&p2[0]);
		}

		if (t1 > t2 || t1 >= glyf_size || t2 > glyf_size)
			return 1;

		ttf_loca_array[i - 1].glyph = glyf_table + t1;
		ttf_loca_array[i - 1].size = (size_t)(t2 - t1);
	}

	return 0;
}

static int ttf_read_glyf(unsigned long point)
{
	unsigned long i = ttf_search_cmap(point);
	const unsigned char *glyph = ttf_loca_array[i].glyph;
	size_t size = ttf_loca_array[i].size;

	unsigned long offset = 0, last_point = 0;
	unsigned long contours;

	ttf_glyf_points = 0;

	/*
	 * Only simple glyphs are supported.
	 */
	if (size < 2 || (contours = BE16(&glyph[0])) >= 0x8000ul)
		return 0;
	if (!contours)
		return 0;
	if (size < (12 + contours * 2))
		return 1;

	offset += 10;

	/*
	 * Find the last point index.
	 */
	for (i = 0; i < contours; i++) {
		const unsigned char *p = &glyph[offset];

		if (last_point < BE16(&p[0]))
			last_point = BE16(&p[0]);
		offset += 2;
	}

	if (!last_point)
		return 0;

	/*
	 * Skip instructions.
	 */
	offset += (BE16(&glyph[offset]) + 2);
	if (size <= offset)
		return 1;

	/*
	 * Process flags.
	 */
	{
		unsigned repeat = 0;

		for (i = 0; i <= last_point; i++) {
			unsigned long flag = (unsigned long)glyph[offset];

			ttf_glyf_array[i].flag = (flag & 0xF7ul);
			ttf_glyf_array[i].x = 0;
			ttf_glyf_array[i].y = 0;

			if (size <= offset + 2)
				return 1;

			if ((flag & 0x08ul) != 0) {
				if (repeat == (unsigned)glyph[offset + 1]) {
					repeat &= 0u;
					offset += 1;
				} else {
					repeat += 1;
					offset -= 1;
				}
			}

			offset += 1;
		}
	}

	/*
	 * Process X coordinates.
	 */
	for (i = 0; i <= last_point; i++) {
		int size_bit = (ttf_glyf_array[i].flag & 0x02ul) != 0 ? 1 : 0;
		int misc_bit = (ttf_glyf_array[i].flag & 0x10ul) != 0 ? 1 : 0;
		long x = (i != 0) ? ttf_glyf_array[i - 1].x : 0;

		if (size_bit == 0 && misc_bit == 0) {
			unsigned long d;

			if (size <= offset + 1) {
				return 1;
			}

			d = BE16(&glyph[offset]);
			x += BE16_TO_LONG(d);
			offset += 2;

		} else if (size_bit == 1 && misc_bit == 0) {
			if (size <= offset)
				return 1;
			x += (-((long)((unsigned long)glyph[offset])));
			offset += 1;

		} else if (size_bit == 1 && misc_bit == 1) {
			if (size <= offset)
				return 1;
			x += (long)((unsigned long)glyph[offset]);
			offset += 1;
		}

		ttf_glyf_array[i].x = x;
	}

	/*
	 * Process Y coordinates.
	 */
	for (i = 0; i <= last_point; i++) {
		int size_bit = (ttf_glyf_array[i].flag & 0x04ul) != 0 ? 1 : 0;
		int misc_bit = (ttf_glyf_array[i].flag & 0x20ul) != 0 ? 1 : 0;
		long y = (i != 0) ? ttf_glyf_array[i - 1].y : 0;

		if (size_bit == 0 && misc_bit == 0) {
			unsigned long d;

			if (size <= offset + 1) {
				return 1;
			}

			d = BE16(&glyph[offset]);
			y += BE16_TO_LONG(d);
			offset += 2;

		} else if (size_bit == 1 && misc_bit == 0) {
			if (size <= offset)
				return 1;
			y += (-((long)((unsigned long)glyph[offset])));
			offset += 1;

		} else if (size_bit == 1 && misc_bit == 1) {
			if (size <= offset)
				return 1;
			y += (long)((unsigned long)glyph[offset]);
			offset += 1;
		}

		ttf_glyf_array[i].y = y;
	}

	/*
	 * Set the last points of contours.
	 */
	for (i = 0; i < contours; i++) {
		const unsigned char *p = &glyph[10 + i * 2];

		ttf_glyf_array[BE16(&p[0])].flag |= 0x0100ul;
	}

	ttf_glyf_points = last_point + 1;
	return 0;
}

static void ttf_free(void)
{
	if (ttf_cmap_array) {
		ttf_cmap_points = 0;
		free(ttf_cmap_array);
		ttf_cmap_array = NULL;
	}

	if (ttf_loca_array) {
		ttf_loca_points = 0;
		free(ttf_loca_array);
		ttf_loca_array = NULL;
	}

	if (ttf_glyf_array) {
		ttf_glyf_points = 0;
		free(ttf_glyf_array);
		ttf_glyf_array = NULL;
	}
}

int ttf_main(struct options *opt)
{
	int ret;

	if ((ret = ttf_read_head()) != 0) {
		fputs("Error: head table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_maxp()) != 0) {
		fputs("Error: maxp table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_cmap()) != 0) {
		fputs("Error: cmap table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_loca()) != 0) {
		fputs("Error: loca table\n", stderr);
		return ttf_free(), ret;
	}

	/*
	 * Allocate memory for the glyf data.
	 */
	ttf_glyf_array = calloc(65536, sizeof(struct glyf));
	if (ttf_glyf_array == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return ttf_free(), 1;
	}

	if (opt->dump) {
		ret = (ttf_dump(), 0);
		if (opt->arg_o == NULL)
			return ttf_free(), ret;
	}

	if (opt->render) {
		if ((ret = ttf_read_glyf(opt->code_point)) != 0) {
			fputs("Error: could not read glyf data\n", stderr);
			return ttf_free(), ret;
		}
		ret = render(opt, ttf_glyf_points, ttf_glyf_array);
		return ttf_free(), ret;
	}

	return ttf_free(), 0;
}
