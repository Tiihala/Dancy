/*
 * Copyright (c) 2017, 2019 Antti Tiihala
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
 * dancy/blob.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_BLOB_H
#define DANCY_BLOB_H

#define DANCY_BLOB(a,b,c,d,e,f,g,h) \
(unsigned)(a) >> 8 & 0xFFu, (unsigned)(a) & 0xFFu, \
(unsigned)(b) >> 8 & 0xFFu, (unsigned)(b) & 0xFFu, \
(unsigned)(c) >> 8 & 0xFFu, (unsigned)(c) & 0xFFu, \
(unsigned)(d) >> 8 & 0xFFu, (unsigned)(d) & 0xFFu, \
(unsigned)(e) >> 8 & 0xFFu, (unsigned)(e) & 0xFFu, \
(unsigned)(f) >> 8 & 0xFFu, (unsigned)(f) & 0xFFu, \
(unsigned)(g) >> 8 & 0xFFu, (unsigned)(g) & 0xFFu, \
(unsigned)(h) >> 8 & 0xFFu, (unsigned)(h) & 0xFFu

#endif
