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
 * dy-link/symbol.c
 *      Linker for Dancy operating system
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"

#define NR_BUCKETS 1024
static struct symbol *symbol_buckets[NR_BUCKETS];
static unsigned char *symbol_buffer;

void symbol_add(int obj, int idx)
{
	struct symbol *sym = &(((struct symbol *)symbol_buffer)[obj]);
	unsigned long offset = LE32(sym->data + 8);
	unsigned long symbols = LE32(sym->data + 12);

}

void *symbol_find(int obj, int idx)
{
	return NULL;
}

void symbol_init(void *buf)
{
	symbol_buffer = (unsigned char *)buf;
}

unsigned symbol_hash(const char *key)
{
	return 0u;
}
