/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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
 * stdio.h
 *      The C Standard Library
 */

#ifndef __DANCY_STDIO_H
#define __DANCY_STDIO_H

#include <__dancy/core.h>
#include <__dancy/seek.h>
#include <__dancy/ssize.h>

#include <stdarg.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

struct __dancy_fpos_t {
	__dancy_off_t __position;
};

#define __DANCY_FILE_STATIC_BUFFER (1u << 16)
#define __DANCY_FILE_STATIC_NAME   (1u << 17)
#define __DANCY_FILE_WRITTEN_BYTES (1u << 18)
#define __DANCY_FILE_MALLOC_STRUCT (1u << 19)

struct __dancy_FILE {
	int __reserved;
	int __fd;

	int __i;
	int __error;
	int __eof;
	int __ungetc;

	unsigned int __mode;
	unsigned int __state;

	__dancy_mtx_t __mtx;

	int __buffer_start;
	int __buffer_end;
	unsigned char *__buffer;
	size_t __buffer_size;

	char *__name;
};

typedef struct __dancy_fpos_t fpos_t;
typedef struct __dancy_FILE FILE;

#define BUFSIZ (4096)
#define EOF    (-1)

#define _IOFBF (0)
#define _IOLBF (1)
#define _IONBF (2)

#define FILENAME_MAX (255)
#define FOPEN_MAX    (32)
#define TMP_MAX      (10000)

#define L_ctermid (16)
#define L_tmpnam  (16)

#define P_tmpdir "/tmp"

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

extern FILE *__dancy_io_array[];
extern __dancy_mtx_t __dancy_io_array_mtx;

void __dancy_stdio_init(void);
void __dancy_stdio_fini(void);
int __dancy_internal_fflush(FILE *stream);

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);
int fflush(FILE *stream);

size_t fread(void *buffer, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t nmemb, FILE *stream);

int fgetc(FILE *stream);
int fputc(int c, FILE *stream);

void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);

int printf(const char *format, ...);

int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int rename(const char *old_path, const char *new_path);

__Dancy_Header_End

#endif
