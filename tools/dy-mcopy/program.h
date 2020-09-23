/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * dy-mcopy/program.h
 *      Program for adding files into FAT file system images
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#define PROGRAM_CMDNAME "dy-mcopy"

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
	const char *arg_i;
	const char *arg_t;
	int db_mode;
	int read_only;
	int verbose;
	void *fat;
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

/*
 * fat.c
 */
int fat_create(void **instance, int id);
int fat_delete(void *fat);

int fat_close(void *fat, int fd);
int fat_control(void *fat, int fd, int write, unsigned char record[32]);
int fat_eof(void *fat, int fd);
int fat_open(void *fat, int fd, const char *name, const char *mode);
int fat_read(void *fat, int fd, size_t *size, void *buf);
int fat_remove(void *fat, const char *name);
int fat_rename(void *fat, const char *old_name, const char *new_name);
int fat_seek(void *fat, int fd, int offset, int whence);
int fat_tell(void *fat, int fd, unsigned int *offset);
int fat_write(void *fat, int fd, size_t *size, const void *buf);

/*
 * mcopy.c
 */
int mcopy(struct options *opt);

/*
 * program.c
 */
extern size_t source_file_size;
extern unsigned char *source_file;

void fat_error(int r);

int fat_get_size(int id, size_t *block_size, size_t *block_total);
int fat_get_time(char iso_8601_format[19]);
int fat_io_read(int id, size_t lba, size_t *size, void *buf);
int fat_io_write(int id, size_t lba, size_t *size, const void *buf);

int program(struct options *opt);

#endif
