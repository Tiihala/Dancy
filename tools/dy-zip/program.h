/*
 * Copyright (c) 2018 Antti Tiihala
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
 * dy-zip/program.h
 *      Program for creating Zip archives
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#define PROGRAM_CMDNAME "dy-zip"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if INT_MAX < 0x7FFFFFFF
#error Definition of INT_MAX is not compatible
#endif

#if !defined(SIZE_MAX)
#define SIZE_MAX (0x7FFFFFFFul)
#endif

struct options {
	char **operands;
	const char *error;
	const char *arg_o;
	const char *arg_t;
	int verbose;
};

struct state {
	unsigned char *output;
	unsigned char **fdata;
	size_t *fsize;
	size_t size;
	size_t fnum;
	unsigned m_date;
	unsigned m_time;
	int split;
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

unsigned long crc32(const void *obj, size_t len);

/*
 * deflate.c
 */
int deflate_compress(unsigned char *data, size_t *size);

/*
 * program.c
 */
int program(struct options *opt);

#endif
