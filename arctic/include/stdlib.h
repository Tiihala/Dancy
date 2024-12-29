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
 * stdlib.h
 *      The C Standard Library
 */

#ifndef __DANCY_STDLIB_H
#define __DANCY_STDLIB_H

#include <__dancy/core.h>

__Dancy_Header_Begin

void *aligned_alloc(size_t alignment, size_t size);
void *calloc(size_t nmemb, size_t size);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));

#undef EXIT_SUCCESS
#define EXIT_SUCCESS (0)

#undef EXIT_FAILURE
#define EXIT_FAILURE (1)

void __dancy_atexit_init(void);
void __dancy_atexit_fini(void);
int atexit(void (*func)(void));

int __dancy_ull(const char *str, char **end, int base, unsigned long long *r);

void abort(void);
int system(const char *command);

int abs(int i);
long int labs(long int i);
long long int llabs(long long int i);

void _Exit(int status);
void exit(int status);

char *getenv(const char *name);
char *realpath(const char *path, char *buffer);

double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);

float strtof(const char *str, char **endptr);
double strtod(const char *str, char **endptr);
long double strtold(const char *str, char **endptr);

long strtol(const char *str, char **endptr, int base);
long long strtoll(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);
unsigned long long strtoull(const char *str, char **endptr, int base);

__Dancy_Header_End

#endif
