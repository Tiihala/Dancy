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
 * include/__dancy/seek.h
 *      The Arctic Dancy Header
 */

#ifndef ARCTIC_DANCY_INTERNAL_SEEK_H
#define ARCTIC_DANCY_INTERNAL_SEEK_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifdef SEEK_SET
#error "Unexpected SEEK_SET definition"
#endif

#ifdef SEEK_CUR
#error "Unexpected SEEK_CUR definition"
#endif

#ifdef SEEK_END
#error "Unexpected SEEK_END definition"
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

__Dancy_Header_End

#endif
