/*
 * Copyright (c) 2026 Antti Tiihala
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
 * search.h
 *      Search tables
 */

#ifndef __DANCY_SEARCH_H
#define __DANCY_SEARCH_H

#include <__dancy/core.h>

__Dancy_Header_Begin

struct entry {
	char *key;
	void *data;
};

typedef struct entry ENTRY;

typedef enum { FIND, ENTER } ACTION;
typedef enum { preorder, postorder, endorder, leaf } VISIT;

typedef void posix_tnode;

int hcreate(size_t nel);
void hdestroy(void);

ENTRY *hsearch(ENTRY item, ACTION action);

void insque(void *element, void *pred);
void remque(void *element);

void *lsearch(const void *key, void *base, size_t *nelp, size_t width,
	int (*compar)(const void *, const void *));

void *lfind(const void *key, const void *base, size_t *nelp, size_t width,
	int (*compar)(const void *, const void *));

void *tdelete(const void *key, posix_tnode **rootp,
	int (*compar)(const void *, const void *));

posix_tnode *tfind(const void *key, posix_tnode *const *rootp,
	int (*compar)(const void *, const void *));

posix_tnode *tsearch(const void *key, posix_tnode **rootp,
	int (*compar)(const void *, const void *));

void twalk(const posix_tnode *root,
	void (*action)(const posix_tnode *, VISIT, int));

__Dancy_Header_End

#endif
