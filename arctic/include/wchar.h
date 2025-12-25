/*
 * Copyright (c) 2023 Antti Tiihala
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
 * wchar.h
 *      The C Standard Library
 */

#ifndef __DANCY_WCHAR_H
#define __DANCY_WCHAR_H

#include <__dancy/core.h>
#include <stddef.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_WINT_T
#define __DANCY_TYPEDEF_WINT_T
typedef unsigned int wint_t;
#endif

#ifndef __DANCY_TYPEDEF_MBSTATE_T
#define __DANCY_TYPEDEF_MBSTATE_T
typedef struct {
	unsigned int _state[4];
} mbstate_t;
#endif

#undef WEOF
#define WEOF (0x7FFFFFFFu)

int mbsinit(const mbstate_t *ps);
size_t mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);

size_t wcslen(const wchar_t *ws);
size_t wcrtomb(char *s, wchar_t w, mbstate_t *ps);

wint_t btowc(int c);
int wcwidth(wchar_t w);

int wcsncmp(const wchar_t *ws1, const wchar_t *ws2, size_t n);

__Dancy_Header_End

#endif
