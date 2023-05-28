/*
 * Copyright (c) 2022 Antti Tiihala
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

__Dancy_Header_End

#endif
