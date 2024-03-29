/*
 * Copyright (c) 2017, 2019, 2020 Antti Tiihala
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
 * common/blob.h
 *      Header of Dancy Operating System
 */

#ifndef COMMON_BLOB_H
#define COMMON_BLOB_H

#define DANCY_BLOB(a,b,c,d,e,f,g,h) \
(unsigned)(a) >> 8 & 0xFFu, (unsigned)(a) & 0xFFu, \
(unsigned)(b) >> 8 & 0xFFu, (unsigned)(b) & 0xFFu, \
(unsigned)(c) >> 8 & 0xFFu, (unsigned)(c) & 0xFFu, \
(unsigned)(d) >> 8 & 0xFFu, (unsigned)(d) & 0xFFu, \
(unsigned)(e) >> 8 & 0xFFu, (unsigned)(e) & 0xFFu, \
(unsigned)(f) >> 8 & 0xFFu, (unsigned)(f) & 0xFFu, \
(unsigned)(g) >> 8 & 0xFFu, (unsigned)(g) & 0xFFu, \
(unsigned)(h) >> 8 & 0xFFu, (unsigned)(h) & 0xFFu

#define B8(a,b,c) (((unsigned long)((a)[(b)]) & 0xFFul) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

#define BE16(a) (B8((a),0,8) | B8((a),1,0))
#define BE32(a) (B8((a),0,24) | B8((a),1,16) | B8((a),2,8) | B8((a),3,0))

#define W_LE16(a,d) ( \
*((a) + 0) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
*((a) + 1) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_LE32(a,d) ( \
*((a) + 0) = (unsigned char)(((unsigned long)(d) >>  0) & 0xFFul), \
*((a) + 1) = (unsigned char)(((unsigned long)(d) >>  8) & 0xFFul), \
*((a) + 2) = (unsigned char)(((unsigned long)(d) >> 16) & 0xFFul), \
*((a) + 3) = (unsigned char)(((unsigned long)(d) >> 24) & 0xFFul))

#endif
