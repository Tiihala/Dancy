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
	__dancy_off_t _position;
};

#define __DANCY_FILE_STATIC_BUFFER (1u << 16)
#define __DANCY_FILE_WRITTEN_BYTES (1u << 17)
#define __DANCY_FILE_MALLOC_STRUCT (1u << 18)

struct __dancy_FILE {
	int _zero;
	int _fd;

	int _i;
	int _error;
	int _eof;
	int _ungetc;

	unsigned int _mode;
	unsigned int _state;

	__dancy_mtx_t _mtx;

	int _buffer_start;
	int _buffer_end;
	unsigned char *_buffer;
	size_t _buffer_size;
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

int __dancy_scanf(int (*get)(void *), int (*unget)(int, void *), void *stream,
	const char *format, va_list arg);

FILE *fopen(const char *path, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *stream);

int fclose(FILE *stream);
int fflush(FILE *stream);
int fileno(FILE *stream);

size_t fread(void *buffer, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *buffer, size_t size, size_t nmemb, FILE *stream);

char *fgets(char *s, int n, FILE *stream);

int fgetc(FILE *stream);
int fputc(int c, FILE *stream);

int fputs(const char *s, FILE *stream);
int puts(const char *s);

int getc(FILE *stream);
int getchar(void);

int putc(int c, FILE *stream);
int putchar(int c);

void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);

void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arg);

int sprintf(char *s, const char *format, ...);
int snprintf(char *s, size_t n, const char *format, ...);
int vsprintf(char *s, const char *format, va_list arg);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

int remove(const char *path);
int rename(const char *old_path, const char *new_path);
void perror(const char *s);

long int ftell(FILE *stream);
int fseek(FILE *stream, long offset, int whence);

off_t ftello(FILE *stream);
int fseeko(FILE *stream, off_t offset, int whence);

int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);

void rewind(FILE *stream);
int ungetc(int c, FILE *stream);

__Dancy_Header_End

#endif
