/*
 * Copyright (c) 2024 Antti Tiihala
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
 * glob.h
 *      Pathname pattern-matching types
 */

#ifndef __DANCY_GLOB_H
#define __DANCY_GLOB_H

#include <__dancy/core.h>

__Dancy_Header_Begin

typedef struct {
	size_t gl_pathc;
	char **gl_pathv;
	size_t gl_offs;
	size_t _dancy;
} glob_t;

#define GLOB_ERR        1
#define GLOB_MARK       2
#define GLOB_NOSORT     4
#define GLOB_DOOFFS     8
#define GLOB_NOCHECK    16
#define GLOB_APPEND     32
#define GLOB_NOESCAPE   64

#define GLOB_NOSPACE    1
#define GLOB_ABORTED    2
#define GLOB_NOMATCH    3

int glob(const char *pattern, int flags,
	int (*errfunc)(const char *epath, int eerrno), glob_t *pglob);

void globfree(glob_t *pglob);

__Dancy_Header_End

#endif
