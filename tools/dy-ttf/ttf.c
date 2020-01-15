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
 * dy-ttf/ttf.c
 *      Create Dancy-compatible .ttf files
 */

#include "program.h"

static unsigned char *output_data;
static size_t        output_size;

static unsigned long ttf_head_size;
static unsigned long ttf_head_flags;
static unsigned long ttf_head_em;
static signed long   ttf_head_xmin;
static signed long   ttf_head_ymin;
static signed long   ttf_head_xmax;
static signed long   ttf_head_ymax;
static unsigned long ttf_head_lowest;
static unsigned long ttf_head_locfmt;

static signed long   ttf_hhea_ascent;
static signed long   ttf_hhea_descent;
static signed long   ttf_hhea_linegap;
static unsigned long ttf_hhea_maxwid;
static signed long   ttf_hhea_minlsb;
static signed long   ttf_hhea_minrsb;
static unsigned long ttf_hhea_metrics;

static unsigned long ttf_maxp_glyphs;

static struct cmap   *ttf_cmap_array;
static unsigned long ttf_cmap_points;

static struct hmtx   *ttf_hmtx_array;
static unsigned long ttf_hmtx_points;

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

	printf("%-20s%ld\n", "ttf_hhea_ascent:", ttf_hhea_ascent);
	printf("%-20s%ld\n", "ttf_hhea_descent:", ttf_hhea_descent);
	printf("%-20s%ld\n", "ttf_hhea_linegap:", ttf_hhea_linegap);
	printf("%-20s%lu\n", "ttf_hhea_maxwid:", ttf_hhea_maxwid);
	printf("%-20s%ld\n", "ttf_hhea_minlsb:", ttf_hhea_minlsb);
	printf("%-20s%ld\n", "ttf_hhea_minrsb:", ttf_hhea_minrsb);
	printf("%-20s%lu\n", "ttf_hhea_metrics:", ttf_hhea_metrics);
	printf("\n");

	printf("%-20s%ld\n", "ttf_maxp_glyphs:", ttf_maxp_glyphs);
	printf("%-20s%ld\n", "ttf_hmtx_points:", ttf_hmtx_points);
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

static int ttf_read_hhea(void)
{
	unsigned char *table;
	unsigned long val;
	size_t size;

	if (table_find(TTF_TABLE_HHEA, &table, &size))
		return 1;

	val = BE16(&table[4]);
	ttf_hhea_ascent = BE16_TO_LONG(val);

	val = BE16(&table[6]);
	ttf_hhea_descent = BE16_TO_LONG(val);

	val = BE16(&table[8]);
	ttf_hhea_linegap = BE16_TO_LONG(val);

	ttf_hhea_maxwid = BE16(&table[10]);

	val = BE16(&table[12]);
	ttf_hhea_minlsb = BE16_TO_LONG(val);

	val = BE16(&table[14]);
	ttf_hhea_minrsb = BE16_TO_LONG(val);

	ttf_hhea_metrics = BE16(&table[34]);

	return 0;
}

static int ttf_read_hmtx(void)
{
	unsigned char *table;
	unsigned long i, val;
	size_t size;

	if (table_find(TTF_TABLE_HMTX, &table, &size))
		return 1;

	ttf_hmtx_array = calloc(ttf_maxp_glyphs, sizeof(struct hmtx));

	if (ttf_hmtx_array == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}

	ttf_hmtx_points = ttf_maxp_glyphs;

	if (ttf_hhea_metrics == 0)
		return 1;

	val = ttf_hhea_metrics * 4;
	if (size < val)
		return 1;

	size -= val;
	size /= 2;

	if (ttf_hhea_metrics + size != ttf_hmtx_points)
		return 1;

	for (i = 0; i < ttf_hhea_metrics; i++) {
		const unsigned char *p = table + (i * 4);

		val = BE16(&p[2]);
		ttf_hmtx_array[i].lsb = BE16_TO_LONG(val);

		val = BE16(&p[0]);
		ttf_hmtx_array[i].width = val;
	}

	if (size != 0) {
		const unsigned char *p = table + (i * 4);

		/*
		 * Repeat the last width value.
		 */
		while (size--) {
			unsigned long lsb = BE16(&p[2]);

			ttf_hmtx_array[i].width = val;
			ttf_hmtx_array[i].lsb = BE16_TO_LONG(lsb);
			i += 1, p += 2;
		}
	}

	if (i != ttf_hmtx_points)
		return 1;

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

		/*
		 * Primary: "Platform ID" 0 (Unicode).
		 */
		if (BE16(&p[0]) == 0) {
			unsigned long cmap_offset = BE32(&p[4]);
			unsigned char *cmap_subtable = table + cmap_offset;

			if (subtable == NULL) {
				offset = cmap_offset;
				subtable = cmap_subtable;
			}
			if (BE16(&cmap_subtable[0]) == 12) {
				offset = cmap_offset;
				subtable = cmap_subtable;
			}
			if (BE16(&subtable[0]) != 12) {
				offset = cmap_offset;
				subtable = cmap_subtable;
			}
		}

		/*
		 * Secondary: "Platform ID" 3 (UCS-2).
		 */
		if (subtable == NULL) {
			if (BE16(&p[0]) == 3 && BE16(&p[2]) == 1) {
				offset = BE32(&p[4]);
				subtable = table + offset;
			}
		}
	}

	/*
	 * Format 4
	 */
	if (subtable != NULL && BE16(&subtable[0]) == 4) {
		unsigned long len = BE16(&subtable[2]);
		unsigned long add, segs;

		if (len < 16 || len > size - offset)
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

		if (ttf_cmap_points == 0 || ttf_cmap_points > 0x10FFFF)
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

	/*
	 * Format 12
	 */
	} else if (subtable != NULL && BE16(&subtable[0]) == 12) {
		unsigned long add, len, groups;

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

		ttf_cmap_points = 0;

		for (i = 0; i < groups; i++) {
			const unsigned char *p = subtable + (i * 12) + 16;
			unsigned long t1, t2;

			t1 = BE32(&p[0]);
			t2 = BE32(&p[4]);

			if (t1 > t2)
				return 1;

			add = (t2 - t1) + 1;
			if (!(ttf_cmap_points < ULONG_MAX - add))
				return 1;

			ttf_cmap_points += add;
		}

		if (ttf_cmap_points == 0 || ttf_cmap_points > 0x10FFFF)
			return 1;

		ttf_cmap_array = calloc(ttf_cmap_points, sizeof(struct cmap));

		if (ttf_cmap_array == NULL) {
			fputs("Error: not enough memory\n", stderr);
			return 1;
		}

		for (add = 0, i = 0; i < groups; i++) {
			const unsigned char *p = subtable + (i * 12) + 16;
			unsigned long t1, t2;
			unsigned long c, glyph_index;

			t1 = BE32(&p[0]);
			t2 = BE32(&p[4]);
			glyph_index = BE32(&p[8]);

			for (c = t1; c <= t2; c++) {
				if (glyph_index >= ttf_maxp_glyphs)
					return 1;

				if (add >= ttf_cmap_points)
					return 1;

				ttf_cmap_array[add].point = c;
				ttf_cmap_array[add].index = glyph_index++;
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

static const void *ttf_read_name(int id, size_t *length)
{
	unsigned char *table;
	unsigned long i, count, offset;
	size_t size;

	*length = 0;

	if (table_find(TTF_TABLE_NAME, &table, &size))
		return NULL;

	count = BE16(&table[2]);
	offset = BE16(&table[4]);

	if (id < 0 || id > 14)
		return NULL;

	if (size < (count * 12) + 6)
		return NULL;

	for (i = 0; i < count; i++) {
		const unsigned char *p = table + (i * 12) + 6;

		/*
		 * Unicode and English language.
		 */
		if (BE16(&p[0]) != 0 || BE16(&p[4]) != 0)
			continue;

		if (BE16(&p[6]) != (unsigned)id)
			continue;

		if (size < offset + BE16(&p[8]) + BE16(&p[10]))
			return NULL;

		*length = (size_t)BE16(&p[8]);
		return (table + offset + BE16(&p[10]));
	}

	return NULL;
}

static int ttf_render(struct options *opt)
{
	unsigned long i = ttf_search_cmap(opt->code_point);
	long glyph_divisor = 1;
	struct render glyph;
	int ret;

	if (i != 0) {
		if ((ret = ttf_read_glyf(opt->code_point)) != 0) {
			fputs("Error: could not read glyf data\n", stderr);
			return ret;
		}
	}

	if (i == 0 || ttf_glyf_points == 0) {
		printf("No glyph for code point 0x%04lX\n", opt->code_point);
		return 0;
	}

	memset(&glyph, 0, sizeof(glyph));

	glyph.array = ttf_glyf_array;
	glyph.points = ttf_glyf_points;
	glyph.head_ymin = ttf_head_ymin;
	glyph.head_ymax = ttf_head_ymax;
	glyph.advance = ttf_hmtx_array[i].width;
	glyph.lsb = ttf_hmtx_array[i].lsb;

	if (opt->arg_s) {
		unsigned long gd = strtoul(opt->arg_s, NULL, 0);

		if (gd == 0 || gd > 128) {
			fputs("Warning: invalid scale divisor\n", stderr);
			return 1;
		}
		glyph_divisor = (long)gd;
	}

	glyph.glyph_divisor = glyph_divisor;

	return render_glyph(opt, &glyph);
}

static void ttf_write_header(size_t tables, unsigned char *out)
{
	unsigned long number_of_tables = (unsigned long)tables;
	unsigned long val;

	W_BE32(&out[0], 0x00010000);
	W_BE16(&out[4], number_of_tables);

	val = table_power_of_two((unsigned)number_of_tables) * 16;
	W_BE16(&out[6], val);

	val = table_power_of_two((unsigned)number_of_tables);
	val = table_log2(table_power_of_two((unsigned)val));
	W_BE16(&out[8], val);

	val = (number_of_tables * 16) - BE16(&output_data[6]);
	W_BE16(&out[10], val);
}

static void ttf_free(void)
{
	if (ttf_cmap_array) {
		ttf_cmap_points = 0;
		free(ttf_cmap_array);
		ttf_cmap_array = NULL;
	}

	if (ttf_hmtx_array) {
		ttf_hmtx_points = 0;
		free(ttf_hmtx_array);
		ttf_hmtx_array = NULL;
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

	if (output_data) {
		output_size = 0;
		free(output_data);
		output_data = NULL;
	}
}

static size_t ttf_build_cmap(size_t offset)
{
	return 0;
}

static size_t ttf_build_glyf(size_t offset)
{
	return 0;
}

static size_t ttf_build_head(size_t offset)
{
	return 0;
}

static size_t ttf_build_hhea(size_t offset)
{
	return 0;
}

static size_t ttf_build_hmtx(size_t offset)
{
	return 0;
}

static size_t ttf_build_loca(size_t offset)
{
	return 0;
}

static size_t ttf_build_maxp(size_t offset)
{
	return 0;
}

static size_t ttf_build_name(size_t offset)
{
	unsigned char *p = &output_data[offset];
	unsigned long count = 0, total = 0;
	size_t length;
	int i;

	struct {
		const unsigned char *data;
		unsigned long length;
		unsigned long offset;
	} entries[15];

	int number_of_entries = (int)(sizeof(entries) / sizeof(entries[0]));

	for (i = 0; i < number_of_entries; i++) {
		entries[i].data = ttf_read_name(i, &length);
		entries[i].length = (unsigned long)length;
		entries[i].offset = total;

		if (length == 0)
			entries[i].data = NULL;

		if (entries[i].data != NULL) {
			total += ((unsigned long)length + 1);
			total &= 0xFFFFFFFEul;
			count += 1;
		}
	}

	count *= 2;
	total += ((count * 12) + 6);

	if (output_size - offset < total) {
		fputs("Error: name table overflow\n", stderr);
		return 0;
	}

	W_BE16(&p[0], 0);
	W_BE16(&p[2], count);
	W_BE16(&p[4], ((count * 12) + 6));
	p += 6;

	for (i = 0; i < number_of_entries; i++) {
		if (entries[i].data == NULL)
			continue;

		W_BE16(&p[0], 0);
		W_BE16(&p[2], 4);
		W_BE16(&p[4], 0);
		W_BE16(&p[6], ((unsigned long)i));
		W_BE16(&p[8], (entries[i].length));
		W_BE16(&p[10], (entries[i].offset));
		p += 12;
	}

	for (i = 0; i < number_of_entries; i++) {
		if (entries[i].data == NULL)
			continue;

		W_BE16(&p[0], 3);
		W_BE16(&p[2], 1);
		W_BE16(&p[4], 0x0409);
		W_BE16(&p[6], ((unsigned long)i));
		W_BE16(&p[8], (entries[i].length));
		W_BE16(&p[10], (entries[i].offset));
		p += 12;
	}

	for (i = 0; i < number_of_entries; i++) {
		if (entries[i].data == NULL)
			continue;

		memcpy(p, entries[i].data, (size_t)entries[i].length);
		p += entries[i].length;
	}

	if ((output_data + offset + total) != p)
		return 0;

	return total;
}

static size_t ttf_build_post(size_t offset)
{
	return 0;
}

int ttf_main(struct options *opt)
{
	size_t offset;
	int ret;

	if ((ret = ttf_read_head()) != 0) {
		fputs("Error: head table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_hhea()) != 0) {
		fputs("Error: hhea table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_maxp()) != 0) {
		fputs("Error: maxp table\n", stderr);
		return ttf_free(), ret;
	}

	if ((ret = ttf_read_hmtx()) != 0) {
		fputs("Error: hmtx table\n", stderr);
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

	if (opt->verbose) {
		const unsigned char *name;
		size_t i, length;

		if ((name = ttf_read_name(4, &length)) != NULL) {
			printf("\n%-20s", "ttf_name:");
			for (i = 0; i < length; i += 2) {
				unsigned c = (unsigned)BE16(&name[i]);
				if (c >= 0x20 && c < 0x7F)
					printf("%c", (int)c);
			}
			printf("\n\n");
		}
		ttf_dump();
	}

	if (opt->render) {
		ret = ttf_render(opt);
		return ttf_free(), ret;
	}

	/*
	 * Allocate output buffer. Use fixed size for now, although
	 * the "ttf_build" functions can increase the size if needed.
	 */
	output_size = 0x200000;
	output_data = malloc(output_size);

	if (output_data == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 1;
	}

	memset(output_data, 0, output_size);
	offset = 0;

	/*
	 * Write to the output buffer.
	 */
	{
		static struct {
			unsigned long name;
			size_t offset;
			size_t size;
			size_t (*callback)(size_t);
		} tables[] = {
			{ TTF_TABLE_CMAP, 0, 0, ttf_build_cmap },
			{ TTF_TABLE_GLYF, 0, 0, ttf_build_glyf },
			{ TTF_TABLE_HEAD, 0, 0, ttf_build_head },
			{ TTF_TABLE_HHEA, 0, 0, ttf_build_hhea },
			{ TTF_TABLE_HMTX, 0, 0, ttf_build_hmtx },
			{ TTF_TABLE_LOCA, 0, 0, ttf_build_loca },
			{ TTF_TABLE_MAXP, 0, 0, ttf_build_maxp },
			{ TTF_TABLE_NAME, 0, 0, ttf_build_name },
			{ TTF_TABLE_POST, 0, 0, ttf_build_post }
		};
		size_t number_of_tables = sizeof(tables) / sizeof(tables[0]);
		size_t i, size;

		offset = number_of_tables * 16 + 12;

		for (i = 0; i < number_of_tables; i++) {
			if ((size = tables[i].callback(offset)) == 0)
				return ttf_free(), 1;

			tables[i].offset = offset;
			tables[i].size = size;

			if (output_size - 128 < offset)
				return ttf_free(), 1;

			offset += (size + 3);
			offset &= 0xFFFFFFFCul;
		}

		output_size = offset;

		for (i = 0; i < number_of_tables; i++) {
			unsigned char *p = output_data + (i * 16) + 12;
			unsigned long off, val;

			val = tables[i].name;
			W_BE32(&p[0], val);

			off = tables[i].offset;
			size = tables[i].size;

			val = table_checksum(output_data + off, size);
			W_BE32(&p[4], val);
			W_BE32(&p[8], off);
			W_BE32(&p[12], size);
		}

		ttf_write_header(number_of_tables, output_data);

		/*
		 * Update 'head' checksum.
		 */
		for (i = 0; i < number_of_tables; i++) {
			if (tables[i].name == TTF_TABLE_HEAD) {
				unsigned char *p;
				unsigned long s;

				s = table_checksum(output_data, output_size);
				s = (0xB1B0AFBAul - s) & 0xFFFFFFFFul;

				p = output_data + tables[i].offset;
				W_BE32(&p[8], s);
			}
		}
	}

	ret = write_file(opt, output_data, output_size);
	return ttf_free(), ret;
}
