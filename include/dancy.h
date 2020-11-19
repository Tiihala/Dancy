/*
 * Copyright (c) 2017, 2018, 2019, 2020 Antti Tiihala
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
 * dancy.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_H
#define DANCY_H

#include <dancy/blob.h>

#if defined(DANCY_32) || defined(DANCY_64)

#include <dancy/ctype.h>
#include <dancy/keys.h>
#include <dancy/lib.h>
#include <dancy/limits.h>
#include <dancy/stdarg.h>
#include <dancy/stdio.h>
#include <dancy/stdlib.h>
#include <dancy/string.h>
#include <dancy/symbol.h>
#include <dancy/types.h>

enum dancy_error {

#define DANCY_X(a, b) a,
#include <dancy/error.h>
#undef DANCY_X

	dancy_error_end
};

#endif
#endif
