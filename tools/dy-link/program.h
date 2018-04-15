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
#define PROGRAM_VERSION "0.1"

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
};

struct mfile {
	unsigned char *data;
	int size;
};

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

/*
 * dump.c
 */
void dump_ext(const char *name, const unsigned char *buf);
void dump_obj(const char *name, const unsigned char *buf);

/*
 * program.c
 */
int program(struct options *opt);

/*
 * validate.c
 */
int validate_obj(const char *name, const unsigned char *buf, int size);

#endif
