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
 * dlfcn.h
 *      Dynamic linking
 */

#ifndef __DANCY_DLFCN_H
#define __DANCY_DLFCN_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define RTLD_LAZY   (0x001)
#define RTLD_NOW    (0x002)

#define RTLD_GLOBAL (0x100)
#define RTLD_LOCAL  (0x000)

void *dlopen(const char *path, int flags);
int dlclose(void *handle);

char *dlerror(void);
void *dlsym(void *handle, const char *symbol);

__Dancy_Header_End

#endif
