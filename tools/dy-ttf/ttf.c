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

static unsigned long ttf_head_em;
static signed long   ttf_head_xmin;
static signed long   ttf_head_ymin;
static signed long   ttf_head_xmax;
static signed long   ttf_head_ymax;
static unsigned long ttf_head_locfmt;

static unsigned long ttf_hhea_metrics;

static unsigned long ttf_maxp_glyphs;
static unsigned long ttf_maxp_points;
static unsigned long ttf_maxp_contours;

static struct cmap   *ttf_cmap_array;
static unsigned long ttf_cmap_points;

static struct hmtx   *ttf_hmtx_array;
static unsigned long ttf_hmtx_points;

static struct loca   *ttf_loca_array;
static unsigned long ttf_loca_points;

static struct glyf   *ttf_glyf_array;
static unsigned long ttf_glyf_points;

static int ttf_read_head(void)
{
	unsigned char *table;
	unsigned long val;
	size_t size;

	if (table_find(TTF_TABLE_HEAD, &table, &size))
		return 1;

	ttf_head_em = BE16(&table[18]);

	val = BE16(&table[36]);
	ttf_head_xmin = BE16_TO_LONG(val);

	val = BE16(&table[38]);
	ttf_head_ymin = BE16_TO_LONG(val);

	val = BE16(&table[40]);
	ttf_head_xmax = BE16_TO_LONG(val);

	val = BE16(&table[42]);
	ttf_head_ymax = BE16_TO_LONG(val);

	ttf_head_locfmt = BE16(&table[50]);

	return 0;
}

static int ttf_read_hhea(void)
{
	unsigned char *table;
	size_t size;

	if (table_find(TTF_TABLE_HHEA, &table, &size))
		return 1;

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
			t1 = BE16(&p1[0]) * 2;
			t2 = BE16(&p2[0]) * 2;
		}

		if (t1 > t2 || t1 >= glyf_size || t2 > glyf_size)
			return 1;

		ttf_loca_array[i - 1].glyph = glyf_table + t1;
		ttf_loca_array[i - 1].size = (size_t)(t2 - t1);
	}

	return 0;
}

static int ttf_read_glyf(unsigned long point, unsigned long index)
{
	unsigned long i = (point != 0) ? ttf_search_cmap(point) : index;
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

	for (i = 0; i < count; i++) {
		const unsigned char *p = table + (i * 12) + 6;

		/*
		 * Platform 3 and English language.
		 */
		if (BE16(&p[0]) != 3 || BE16(&p[4]) != 0x0409)
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
	struct render glyph;
	int ret;

	if (i != 0) {
		if ((ret = ttf_read_glyf(opt->code_point, 0)) != 0) {
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
	unsigned char *p = &output_data[offset];
	unsigned long i, val, groups = 0, groups_format4 = 0;
	size_t size;

	struct {
		unsigned long start;
		unsigned long end;
		unsigned long index;
		unsigned long delta;
	} *group_array;

	size = (size_t)ttf_cmap_points + 1;
	group_array = calloc(size, sizeof(*group_array));

	if (group_array == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return 0;
	}

	for (i = 0; i < ttf_cmap_points; i++) {
		unsigned long point = ttf_cmap_array[i].point;
		unsigned long index = ttf_cmap_array[i].index;
		unsigned long delta = (index - point) & 0xFFFFul;
		int repeat = 0;

		if (i != 0 && ttf_cmap_array[i - 1].point + 1 == point) {
			if (ttf_cmap_array[i - 1].index + 1 == index)
				repeat = 1;
		}

		if (repeat) {
			group_array[--groups].end = point;
		} else {
			group_array[groups].start = point;
			group_array[groups].end = point;
			group_array[groups].index = index;
			group_array[groups].delta = delta;
		}

		groups += 1;
	}

	for (i = 0; i < groups; i++) {
		if (group_array[i].end <= 0xFFFFul)
			groups_format4 += 1;
	}

	/*
	 * Check the buffer size using the worst case scenario plus
	 * enough extra for the headers. There should be enough free
	 * memory because this is the first table in the final file.
	 */
	size = (groups * 20) + 4096;

	if (output_size - offset < size) {
		fputs("Error: cmap table overflow\n", stderr);
		return free(group_array), 0u;
	}

	if (groups_format4 == 0) {
		fputs("Error: empty cmap table\n", stderr);
		return free(group_array), 0u;
	}

	W_BE16(&p[0], 0);
	W_BE16(&p[2], 2);

	W_BE16(&p[4], 0);
	W_BE16(&p[6], 4);
	W_BE32(&p[8], 20);

	W_BE16(&p[12], 3);
	W_BE16(&p[14], 1);
	W_BE32(&p[16], 0);

	/*
	 * Format 12.0.
	 */
	W_BE16(&p[20], 12);
	W_BE16(&p[22], 0);

	val = 16 + (groups * 12);
	W_BE32(&p[24], val);

	W_BE32(&p[28], 0);
	W_BE32(&p[32], groups);

	size = 36;
	p += size;

	for (i = 0; i < groups; i++) {
		unsigned long s = group_array[i].start;
		unsigned long e = group_array[i].end;
		unsigned long index = group_array[i].index;

		W_BE32(&p[0], s);
		W_BE32(&p[4], e);
		W_BE32(&p[8], index);

		p += 12;
	}

	size += (groups * 12);

	p = &output_data[offset];
	W_BE32(&p[16], ((unsigned long)size));
	p += size;

	/*
	 * Format 4.
	 */
	W_BE16(&p[0], 0x0004);

	if (group_array[groups_format4 - 1].end != 0xFFFF) {
		group_array[groups_format4].start = 0xFFFF;
		group_array[groups_format4].end = 0xFFFF;
		group_array[groups_format4].index = 0;
		group_array[groups_format4].delta = 1;
		groups_format4 += 1;
	}

	val = (groups_format4 * 8) + 16;
	W_BE16(&p[2], val);
	W_BE16(&p[4], 0);
	size += val;

	val = groups_format4 * 2;
	W_BE16(&p[6], val);

	/*
	 * searchRange, entrySelector, and rangeShift.
	 */
	{
		val = 2 * table_power_of_two((unsigned)groups_format4);
		W_BE16(&p[8], val);

		val = table_log2((unsigned)(val / 2));
		W_BE16(&p[10], val);

		val = (groups_format4 * 2) - BE16(&p[8]);
		W_BE16(&p[12], val);
	}
	p += 14;

	for (i = 0; i < groups_format4; i++) {
		val = group_array[i].end;
		W_BE16(&p[0], val);
		p += 2;
	}

	W_BE16(&p[0], 0x0000);
	p += 2;

	for (i = 0; i < groups_format4; i++) {
		val = group_array[i].start;
		W_BE16(&p[0], val);
		p += 2;
	}

	for (i = 0; i < groups_format4; i++) {
		val = group_array[i].delta;
		W_BE16(&p[0], val);
		p += 2;
	}

	for (i = 0; i < groups_format4; i++) {
		W_BE16(&p[0], 0);
		p += 2;
	}

	return free(group_array), size;
}

static size_t ttf_build_glyf(size_t offset)
{
	long head_xmin = LONG_MAX, head_ymin = LONG_MAX;
	long head_xmax = 0, head_ymax = 0;

	unsigned long maxp_points = 0, maxp_contours = 0;
	long ymin_limit = -((long)ttf_head_em / 4L);
	long ymax_limit = (long)ttf_head_em - ((long)ttf_head_em / 4L);

	unsigned char *p = &output_data[offset];
	size_t total_size = 0;
	unsigned long i, j;

	for (i = 0; i < ttf_loca_points; i++) {
		long xmin = LONG_MAX, ymin = LONG_MAX, xmax = 0, ymax = 0;
		unsigned long val, contours = 0;
		size_t size = 0;

		if (ttf_read_glyf(0, i)) {
			fputs("Error: could not read glyf data\n", stderr);
			return 0;
		}

		for (j = 0; j < ttf_glyf_points; j++) {
			long x = ttf_glyf_array[j].x;
			long y = ttf_glyf_array[j].y;

			if (xmin > x)
				xmin = x;
			if (xmax < x)
				xmax = x;
			if (ymin > y)
				ymin = y;
			if (ymax < y)
				ymax = y;
		}

		/*
		 * Scale the x-axis if slightly below zero.
		 */
		if (xmin < 0 && xmax >= (8 * (-xmin))) {
			for (j = 0; j < ttf_glyf_points; j++) {
				long x = ttf_glyf_array[j].x;

				x += (-xmin);
				x *= (xmax + (-xmin));
				x /= (xmax + (-xmin) + (-xmin));
				ttf_glyf_array[j].x = x;
			}
		}

		/*
		 * Scale the y-axis if needed.
		 */
		{
			long y_mul = 1, y_div = 1;

			if (ymin < ymin_limit) {
				y_mul = (-ymin_limit);
				y_div = (-ymin);
			}

			if (ymax > ymax_limit) {
				y_mul = ymax_limit;
				y_div = ymax;
			}

			if (y_mul != 1) {
				for (j = 0; j < ttf_glyf_points; j++) {
					long y = ttf_glyf_array[j].y;

					y *= y_mul;
					y /= y_div;
					ttf_glyf_array[j].y = y;
				}
			}
		}

		xmin = LONG_MAX, ymin = LONG_MAX, xmax = 0, ymax = 0;

		/*
		 * The master outline resolution is decreased.
		 */
		for (j = 0; j < ttf_glyf_points; j++) {
			long x = ttf_glyf_array[j].x / 8;
			long y = ttf_glyf_array[j].y / 8;

			ttf_glyf_array[j].x = x * 8;
			ttf_glyf_array[j].y = y * 8;
		}

		for (j = 0; j < ttf_glyf_points; j++) {
			long x = ttf_glyf_array[j].x;
			long y = ttf_glyf_array[j].y;
			int skip = 0;

			if ((ttf_glyf_array[j].flag & 0x0100) != 0)
				contours += 1;

			if (x < 0 || x > (long)ttf_head_em)
				skip = 1;
			if (y < ymin_limit || y > ymax_limit)
				skip = 1;

			if (skip) {
				xmin = 0, xmax = 0;
				ttf_glyf_points = 0;
				contours = 0;
				break;
			}

			if (xmin > x)
				xmin = x;
			if (xmax < x)
				xmax = x;
			if (ymin > y)
				ymin = y;
			if (ymax < y)
				ymax = y;

			if (head_xmin > x)
				head_xmin = x;
			if (head_xmax < x)
				head_xmax = x;
			if (head_ymin > y)
				head_ymin = y;
			if (head_ymax < y)
				head_ymax = y;
		}

		if (xmin > xmax)
			xmin = 0;
		if (ymin > ymax)
			ymin = 0;

		if (i < ttf_hmtx_points) {
			ttf_hmtx_array[i].xmin = xmin;
			ttf_hmtx_array[i].xmax = xmax;
		}

		if (maxp_points < ttf_glyf_points)
			maxp_points = ttf_glyf_points;
		if (maxp_contours < contours)
			maxp_contours = contours;

		if (ttf_glyf_points != 0)
			size = (contours * 2) + (ttf_glyf_points * 5) + 12;

		if (output_size - offset - total_size < size) {
			fputs("Error: glyf table overflow\n", stderr);
			return 0;
		}

		ttf_loca_array[i].ttf_new_offset = total_size;
		ttf_loca_array[i].ttf_new_size = size;

		total_size += size;

		if (ttf_glyf_points == 0)
			continue;

		/*
		 * Simple glyph format.
		 */
		W_BE16(&p[0], contours);
		{
			val = LONG_TO_UNSIGNED(xmin);
			W_BE16(&p[2], val);

			val = LONG_TO_UNSIGNED(ymin);
			W_BE16(&p[4], val);

			val = LONG_TO_UNSIGNED(xmax);
			W_BE16(&p[6], val);

			val = LONG_TO_UNSIGNED(ymax);
			W_BE16(&p[8], val);
		}
		p += 10;

		/*
		 * Endpoints of contours.
		 */
		for (j = 0; j < ttf_glyf_points; j++) {
			if ((ttf_glyf_array[j].flag & 0x0100) != 0) {
				W_BE16(&p[0], j);
				p += 2;
			}
		}

		/*
		 * Instructions are not copied.
		 */
		W_BE16(&p[0], 0);
		p += 2;

		/*
		 * Array of flags. Only values 0 and 1 are used.
		 */
		for (j = 0; j < ttf_glyf_points; j++) {
			if ((ttf_glyf_array[j].flag & 1) != 0)
				*p++ = 1;
			else
				*p++ = 0;
		}

		/*
		 * X coordinates.
		 */
		for (j = 0; j < ttf_glyf_points; j++) {
			long x = ttf_glyf_array[j].x;

			if (j > 0)
				x -= (ttf_glyf_array[j - 1].x);

			val = LONG_TO_UNSIGNED(x);
			W_BE16(&p[0], val);
			p += 2;
		}

		/*
		 * Y coordinates.
		 */
		for (j = 0; j < ttf_glyf_points; j++) {
			long y = ttf_glyf_array[j].y;

			if (j > 0)
				y -= (ttf_glyf_array[j - 1].y);

			val = LONG_TO_UNSIGNED(y);
			W_BE16(&p[0], val);
			p += 2;
		}
	}

	if ((output_data + offset + total_size) != p) {
		fputs("Error: glyf table inconsistency\n", stderr);
		return 0;
	}

	if (head_xmin > head_xmax || head_ymin > head_ymax) {
		fputs("Error: bounding box not set\n", stderr);
		return 0;
	}

	/*
	 * Normalize advance widths and left side bearings.
	 */
	for (i = 0; i < ttf_hmtx_points; i++) {
		unsigned long width = ttf_hmtx_array[i].width;
		long xmin = ttf_hmtx_array[i].xmin;
		long xmax = ttf_hmtx_array[i].xmax;
		long xlen = xmax - xmin;

		if (xlen < 0 || xmin < 0 || xmax > (long)ttf_head_em) {
			fputs("Error: hmtx table inconsistency\n", stderr);
			return 0;
		}

		if (xmax > (long)width)
			width = (unsigned long)xmax + 1;

		if (width > ttf_head_em)
			width = ttf_head_em;

		ttf_hmtx_array[i].width = width;
		ttf_hmtx_array[i].lsb = (xlen > 0) ? xmin : 0L;
	}

	ttf_head_xmin = head_xmin;
	ttf_head_ymin = head_ymin;
	ttf_head_xmax = head_xmax;
	ttf_head_ymax = head_ymax;

	ttf_maxp_points = maxp_points;
	ttf_maxp_contours = maxp_contours;

	return total_size;
}

static size_t ttf_build_head(size_t offset)
{
	unsigned char *p = &output_data[offset];
	unsigned char *table;
	unsigned long val;
	size_t size;

	if (table_find(TTF_TABLE_HEAD, &table, &size))
		return 0;

	size = 54;

	if (output_size - offset < size) {
		fputs("Error: head table overflow\n", stderr);
		return 0;
	}

	memset(&p[0], 0, size);

	W_BE16(&p[0], 0x0001);

	val = BE32(&table[4]);
	W_BE32(&p[4], val);
	W_BE32(&p[12], 0x5F0F3CF5);

	/*
	 * Flags are fixed.
	 */
	W_BE16(&p[16], 0x000B);

	memcpy(&p[18], &table[18], 18);

	val = LONG_TO_UNSIGNED(ttf_head_xmin);
	W_BE16(&p[36], val);

	val = LONG_TO_UNSIGNED(ttf_head_ymin);
	W_BE16(&p[38], val);

	val = LONG_TO_UNSIGNED(ttf_head_xmax);
	W_BE16(&p[40], val);

	val = LONG_TO_UNSIGNED(ttf_head_ymax);
	W_BE16(&p[42], val);

	W_BE16(&p[46], 16);
	W_BE16(&p[50], 1);

	return size;
}

static size_t ttf_build_hhea(size_t offset)
{
	unsigned char *p = &output_data[offset];
	long minlsb = 0, minrsb = 0, maxextent = 0;
	unsigned long i, val, maxwid = 0;
	size_t size = 36;

	if (output_size - offset < size) {
		fputs("Error: hhea table overflow\n", stderr);
		return 0;
	}

	for (i = 0; i < ttf_hmtx_points; i++) {
		unsigned long width = ttf_hmtx_array[i].width;
		long lsb = ttf_hmtx_array[i].lsb;
		long xmin = ttf_hmtx_array[i].xmin;
		long xmax = ttf_hmtx_array[i].xmax;
		long rsb = (long)width - lsb - (xmax - xmin);

		if (lsb < 0 || rsb < 0) {
			fputs("Error: hmtx table inconsistency\n", stderr);
			return 0;
		}

		if (maxwid < width)
			maxwid = width;
		if (minlsb > lsb)
			minlsb = lsb;
		if (minrsb > rsb)
			minrsb = rsb;

		if (maxextent < (lsb + (xmax - xmin)))
			maxextent = (lsb + (xmax - xmin));
	}

	memset(&p[0], 0, size);

	W_BE16(&p[0], 0x0001);

	val = LONG_TO_UNSIGNED((long)ttf_head_em - ((long)ttf_head_em / 4L));
	W_BE16(&p[4], val);

	val = LONG_TO_UNSIGNED(-((long)ttf_head_em / 4L));
	W_BE16(&p[6],val);

	val = LONG_TO_UNSIGNED((long)ttf_head_em / 32L);
	W_BE16(&p[8], val);

	W_BE16(&p[10], maxwid);

	val = LONG_TO_UNSIGNED(minlsb);
	W_BE16(&p[12], val);

	val = LONG_TO_UNSIGNED(minrsb);
	W_BE16(&p[14], val);

	val = LONG_TO_UNSIGNED(maxextent);
	W_BE16(&p[16], val);

	W_BE16(&p[18], 1);
	W_BE16(&p[34], ttf_maxp_glyphs);

	return size;
}

static size_t ttf_build_hmtx(size_t offset)
{
	unsigned char *p = &output_data[offset];
	size_t size = (size_t)(ttf_hmtx_points * 4);
	unsigned long i, val;

	if (output_size - offset < size) {
		fputs("Error: hmtx table overflow\n", stderr);
		return 0;
	}

	for (i = 0; i < ttf_hmtx_points; i++) {
		unsigned long width = ttf_hmtx_array[i].width;
		long lsb = ttf_hmtx_array[i].lsb;

		W_BE16(&p[0], width);

		val = LONG_TO_UNSIGNED(lsb);
		W_BE16(&p[2], val);

		p += 4;
	}

	return size;
}

static size_t ttf_build_loca(size_t offset)
{
	unsigned char *p = &output_data[offset];
	size_t size = (size_t)(ttf_loca_points * 4);
	unsigned long i, val = 0;

	if (size < 8 || output_size - offset < size) {
		fputs("Error: incompatible loca table\n", stderr);
		return 0;
	}

	for (i = 0; i < ttf_loca_points - 1; i++) {
		val = ttf_loca_array[i].ttf_new_offset;
		W_BE32(&p[0], val);
		p += 4;
	}

	val = val + ttf_loca_array[ttf_loca_points - 2].ttf_new_size;
	W_BE32(&p[0], val);

	return size;
}

static size_t ttf_build_maxp(size_t offset)
{
	unsigned char *p = &output_data[offset];
	size_t size = 32;

	if (output_size - offset < size) {
		fputs("Error: maxp table overflow\n", stderr);
		return 0;
	}

	memset(&p[0], 0, size);

	W_BE16(&p[0], 0x0001);
	W_BE16(&p[4], ttf_maxp_glyphs);
	W_BE16(&p[6], ttf_maxp_points);
	W_BE16(&p[8], ttf_maxp_contours);

	return size;
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
	unsigned char *p = &output_data[offset];
	unsigned long italic_angle = 0;
	unsigned char *table;
	size_t size;

	if (table_find(TTF_TABLE_POST, &table, &size) == 0) {
		if (size >= 32)
			italic_angle = BE32(&table[4]);
	}

	size = 32;

	if (output_size - offset < size) {
		fputs("Error: post table overflow\n", stderr);
		return 0;
	}

	memset(&p[0], 0, size);

	/*
	 * Format 3, a very simple format.
	 */
	W_BE16(&p[0], 0x0003);
	W_BE32(&p[4], italic_angle);
	W_BE16(&p[10], 10);

	return size;
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
			printf("ttf_name: ");
			for (i = 0; i < length; i += 2) {
				unsigned c = (unsigned)BE16(&name[i]);
				if (c >= 0x20 && c < 0x7F)
					printf("%c", (int)c);
			}
			printf("\n\n");
		}
	}

	if (opt->render) {
		ret = ttf_render(opt);
		return ttf_free(), ret;
	}

	/*
	 * Dancy-compatible .ttf file requires a specific "em" size.
	 */
	if (ttf_head_em != 2048) {
		fputs("Error: the em size is not 2048\n", stderr);
		return ttf_free(), 1;
	}

	/*
	 * Allocate output buffer. Use fixed size for now, although
	 * the "ttf_build" functions can increase the size if needed.
	 */
	output_size = 0x200000;
	output_data = malloc(output_size);

	if (output_data == NULL) {
		fputs("Error: not enough memory\n", stderr);
		return ttf_free(), 1;
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
