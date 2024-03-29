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
 * dy-ttf/program.h
 *      Create Dancy-compatible .ttf files
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#define PROGRAM_CMDNAME "dy-ttf"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	unsigned long code_point;
	int created;
	int render;
	int verbose;
};

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))
#define BE16(a) (B8((a),0,8) | B8((a),1,0))
#define BE32(a) (B8((a),0,24) | B8((a),1,16) | B8((a),2,8) | B8((a),3,0))

#define W_LE16(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_LE32(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

#define W_BE16(a,d) ( \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_BE32(a,d) ( \
	*((a) + 3) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
	*((a) + 2) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
	*((a) + 1) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
	*((a) + 0) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

#define BE16_TO_LONG(a) \
	(((a) >= 0x8000ul) ? -((long)((~(a) + 1ul) & 0xFFFFul)) : (long)(a))

#define LONG_TO_UNSIGNED(a) \
	((((a) < 0) ? ~((unsigned)(-(a))) + 1u : (unsigned)(a)) & 0xFFFFu)

#define DANCY_BLOB(a,b,c,d,e,f,g,h) \
	(unsigned)(a) >> 8 & 0xFFu, (unsigned)(a) & 0xFFu, \
	(unsigned)(b) >> 8 & 0xFFu, (unsigned)(b) & 0xFFu, \
	(unsigned)(c) >> 8 & 0xFFu, (unsigned)(c) & 0xFFu, \
	(unsigned)(d) >> 8 & 0xFFu, (unsigned)(d) & 0xFFu, \
	(unsigned)(e) >> 8 & 0xFFu, (unsigned)(e) & 0xFFu, \
	(unsigned)(f) >> 8 & 0xFFu, (unsigned)(f) & 0xFFu, \
	(unsigned)(g) >> 8 & 0xFFu, (unsigned)(g) & 0xFFu, \
	(unsigned)(h) >> 8 & 0xFFu, (unsigned)(h) & 0xFFu

/*
 * The recognized TTF tables.
 */
#define TTF_TABLE_CMAP (0x636D6170ul)
#define TTF_TABLE_GLYF (0x676C7966ul)
#define TTF_TABLE_HEAD (0x68656164ul)
#define TTF_TABLE_HHEA (0x68686561ul)
#define TTF_TABLE_HMTX (0x686D7478ul)
#define TTF_TABLE_KERN (0x6B65726Eul)
#define TTF_TABLE_LOCA (0x6C6F6361ul)
#define TTF_TABLE_MAXP (0x6D617870ul)
#define TTF_TABLE_NAME (0x6E616D65ul)
#define TTF_TABLE_POST (0x706F7374ul)

struct cmap {
	unsigned long point;
	unsigned long p_idx;
};

struct hmtx {
	unsigned long width;
	signed long lsb;
	signed long xmin;
	signed long xmax;
};

struct glyf {
	unsigned long flag;
	signed long x;
	signed long y;
};

struct loca {
	unsigned char *glyph;
	size_t size;
	size_t ttf_new_offset;
	size_t ttf_new_size;
};

struct render {
	struct glyf *array;
	unsigned long points;
	signed long head_ymin;
	signed long head_ymax;
	unsigned long advance;
	signed long lsb;
};

/*
 * program.c
 */
int program(struct options *opt);
int read_file(const char *name, unsigned char **out, size_t *size);
int write_file(struct options *opt, unsigned char *out, size_t size);

/*
 * render.c
 */
int render_glyph(struct options *opt, struct render *glyph);

/*
 * table.c
 */
unsigned long table_checksum(const unsigned char *data, size_t size);
int table_find(unsigned long name, unsigned char **table, size_t *size);
int table_init(unsigned char *data, size_t size);
unsigned table_log2(unsigned num);
unsigned table_power_of_two(unsigned num);

/*
 * ttf.c
 */
int ttf_main(struct options *opt);

#endif
