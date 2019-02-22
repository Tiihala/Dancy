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
 * dy-link/program.h
 *      Linker for Dancy operating system
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#define PROGRAM_CMDNAME "dy-link"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined (CHAR_BIT) || CHAR_BIT != 8
#error Definition of CHAR_BIT is not compatible
#endif

#if !defined (INT_MAX) || INT_MAX < 2147483647
#error Definition of INT_MAX is not compatible
#endif

struct options {
	char **operands;
	const char *error;
	const char *arg_f;
	const char *arg_o;
	int verbose;
	int dump;
	int dump_ext;
	int nr_mfiles;
	struct mfile *mfiles;
	unsigned long alignbits_t;
	unsigned long alignbits_r;
	unsigned long alignbits_d;
	unsigned long alignbits_b;
};

struct mfile {
	unsigned char *data;
	int size;
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

#define AT_HEADER_SIZE (0x0020)
#define IN_HEADER_SIZE (0x01A0)

/*
 * dump.c
 */
void dump_ext(const char *name, const unsigned char *buf);
void dump_obj(const char *name, const unsigned char *buf);

/*
 * link.c
 */
int link_main(struct options *opt);

/*
 * program.c
 */
int program(struct options *opt);

/*
 * section.c
 */
int section_group(struct options *opt);
int section_check_sizes(struct options *opt);
int section_copy_d(struct options *opt, const char *name, unsigned char *out);
int section_copy_r(struct options *opt, const char *name, unsigned char *out);
int section_reloc(struct options *opt, int obj, int sec, unsigned long *addr);
int section_data_size(struct options *opt, const char *name);
int section_reloc_size(struct options *opt, const char *name);

/*
 * symbol.c
 */
int symbol_check_sizes(struct options *opt);
int symbol_copy_table(struct options *opt, unsigned char *out);
int symbol_process(struct options *opt, unsigned char *obj);
int symbol_string_size(struct options *opt);
int symbol_table_size(struct options *opt);

/*
 * validate.c
 */
int validate_obj(const char *name, const unsigned char *buf, int size);

#endif
