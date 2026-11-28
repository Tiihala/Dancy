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
 * libc/math/ceil.c
 *      Ceiling value functions
 */

#include <math.h>

static int not_implemented;

double ceil(double x)
{
	__dancy_debug("Error: ceil() is not implemented\n");

	return (void)x, (double)not_implemented;
}

float ceilf(float x)
{
	__dancy_debug("Error: ceilf() is not implemented\n");

	return (void)x, (float)not_implemented;
}

long double ceill(long double x)
{
	__dancy_debug("Error: ceill() is not implemented\n");

	return (void)x, (long double)not_implemented;
}
