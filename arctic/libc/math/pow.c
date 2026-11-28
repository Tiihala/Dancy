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
 * libc/math/pow.c
 *      Power functions
 */

#include <math.h>

static int not_implemented;

double pow(double x, double y)
{
	__dancy_debug("Error: pow() is not implemented\n");

	return (void)x, (void)y, (double)not_implemented;
}

float powf(float x, float y)
{
	__dancy_debug("Error: powf() is not implemented\n");

	return (void)x, (void)y, (float)not_implemented;
}

long double powl(long double x, long double y)
{
	__dancy_debug("Error: powl() is not implemented\n");

	return (void)x, (void)y, (long double)not_implemented;
}
