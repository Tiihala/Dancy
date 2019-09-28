/*
 * Copyright (c) 2019 Antti Tiihala
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
 * uefi.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_UEFI_H
#define DANCY_UEFI_H

#if !defined(DANCY_64)
#error Definition of DANCY_64 is not available
#endif

#include <dancy/ctype.h>
#include <dancy/limits.h>
#include <dancy/stdarg.h>
#include <dancy/string.h>
#include <dancy/types.h>

int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

#endif
