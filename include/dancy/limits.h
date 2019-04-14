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
 * dancy/limits.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_LIMITS_H
#define DANCY_LIMITS_H

#include <limits.h>

#if !defined (CHAR_BIT) || CHAR_BIT != 8
#error Definition of CHAR_BIT is not compatible
#endif

#if !defined (SCHAR_MIN) || SCHAR_MIN != -128
#error Definition of SCHAR_MIN is not compatible
#endif

#if !defined (SCHAR_MAX) || SCHAR_MAX != 127
#error Definition of SCHAR_MAX is not compatible
#endif

#if !defined (USHRT_MAX) || USHRT_MAX != 65535u
#error Definition of USHRT_MAX is not compatible
#endif

#if !defined (UINT_MAX) || UINT_MAX != 4294967295ul
#error Definition of UINT_MAX is not compatible
#endif

#if !defined (ULONG_MAX)
#error Definition of ULONG_MAX is not available
#endif

#if defined (DANCY_32) && ULONG_MAX != 4294967295ul
#error Definition of ULONG_MAX is not compatible
#endif

#if !defined (ULLONG_MAX)
#error Definition of ULLONG_MAX is not available
#endif

#if ULLONG_MAX != 18446744073709551615ull
#error Definition of ULLONG_MAX is not compatible
#endif

#endif
