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
 * dy-png/program.h
 *      Convert .bmp files to Dancy-compatible .png files
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#define PROGRAM_CMDNAME "dy-png"

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
	const char *arg_p;
	int bmp;
	int verbose;
	int pl_ncolors;
};

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

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

unsigned long crc32(const void *obj, size_t len);

struct bitarray {
	unsigned char *data;
	size_t size;
	unsigned state[2];
	size_t written;
	int (*callback)(struct bitarray *b);
};

struct huffman {
	unsigned lengths[16];
	unsigned *symbols;
	unsigned completed;
};

/*
 * bitarray.c
 */
void bitarray_init(struct bitarray *b, unsigned char *data, size_t size);
void bitarray_callback(struct bitarray *b, int (*func)(struct bitarray *b));
void bitarray_clear(struct bitarray *b);
long bitarray_aligned_fetch(struct bitarray *b, unsigned bits, void **data);
long bitarray_fetch(struct bitarray *b, unsigned bits);
int bitarray_shove(struct bitarray *b, unsigned bits, unsigned val);
int bitarray_written(struct bitarray *b, size_t *written);

/*
 * convert.c
 */
int convert(struct options *opt, unsigned char *bmp_file, size_t bmp_size);

/*
 * deflate.c
 */
int deflate_compress(unsigned char *data, size_t *size);

/*
 * huffman.c
 */
int huffman_init(struct huffman *h, unsigned *symbols, int n);
int huffman_fetch(struct huffman *h, struct bitarray *b);
int huffman_table(struct huffman *h, unsigned *table, int n);

/*
 * palette.c
 */
void pl_color(int i, unsigned char *r, unsigned char *g, unsigned char *b);
unsigned pl_find_color(unsigned char r, unsigned char g, unsigned char b);
int pl_init(struct options *opt);

/*
 * program.c
 */
int program(struct options *opt);
int read_file(const char *name, unsigned char **out, size_t *size);
int write_file(struct options *opt, unsigned char *out, size_t size);

#endif
