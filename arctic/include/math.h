/*
 * Copyright (c) 2023 Antti Tiihala
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
 * math.h
 *      The C Standard Library
 */

#ifndef __DANCY_MATH_H
#define __DANCY_MATH_H

#include <__dancy/core.h>

__Dancy_Header_Begin

double fabs(double x);
float fabsf(float x);

double ldexp(double x, int e);
float ldexpf(float x, int e);
long double ldexpl(long double x, int e);

double frexp(double x, int *e);
float frexpf(float x, int *e);
long double frexpl(long double x, int *e);

#define FP_INFINITE  (0)
#define FP_NAN       (1)
#define FP_NORMAL    (2)
#define FP_SUBNORMAL (3)
#define FP_ZERO      (4)

#if defined(__GNUC__)

#define fpclassify(x)         (__builtin_fpclassify(1, 0, 2, 3, 4, (x)))
#define isfinite(x)           (__builtin_isfinite((x)))
#define isgreater(x, y)       (__builtin_isgreater((x), (y)))
#define isgreaterequal(x, y)  (__builtin_isgreaterequal((x), (y)))
#define isinf(x)              (__builtin_isinf_sign((x)))
#define isless(x, y)          (__builtin_isless((x), (y)))
#define islessequal(x, y)     (__builtin_islessequal((x), (y)))
#define islessgreater(x, y)   (__builtin_islessgreater((x), (y)))
#define isnan(x)              (__builtin_isnan((x)))
#define isnormal(x)           (__builtin_isnormal((x)))
#define isunordered(x, y)     (__builtin_isunordered((x), (y)))
#define signbit(x)            (__builtin_signbit((x)))

#else

#define fpclassify(x)         (__dancy_debug("Error: fpclassify()\n"), 0)
#define isfinite(x)           (__dancy_debug("Error: isfinite()\n"), 0)
#define isgreater(x, y)       (__dancy_debug("Error: isgreater()\n"), 0)
#define isgreaterequal(x, y)  (__dancy_debug("Error: isgreaterequal()\n"), 0)
#define isinf(x)              (__dancy_debug("Error: isinf()\n"), 0)
#define isless(x, y)          (__dancy_debug("Error: isless()\n"), 0)
#define islessequal(x, y)     (__dancy_debug("Error: islessequal()\n"), 0)
#define islessgreater(x, y)   (__dancy_debug("Error: islessgreater()\n"), 0)
#define isnan(x)              (__dancy_debug("Error: isnan()\n"), 0)
#define isnormal(x)           (__dancy_debug("Error: isnormal()\n"), 0)
#define isunordered(x, y)     (__dancy_debug("Error: isunordered()\n"), 0)
#define signbit(x)            (__dancy_debug("Error: signbit()\n"), 0)

#endif

double acos(double x);
float acosf(float x);
double acosh(double x);
float acoshf(float x);
long double acoshl(long double x);
long double acosl(long double x);

double asin(double x);
float asinf(float x);
double asinh(double x);
float asinhf(float x);
long double asinhl(long double x);
long double asinl(long double x);

double atan(double x);
float atanf(float x);
double atanh(double x);
float atanhf(float x);
long double atanhl(long double x);
long double atanl(long double x);

double atan2(double y, double x);
float atan2f(float y, float x);
long double atan2l(long double y, long double x);

double cbrt(double x);
float cbrtf(float x);
long double cbrtl(long double x);

double ceil(double x);
float ceilf(float x);
long double ceill(long double x);

double cos(double x);
float cosf(float x);
double cosh(double x);
float coshf(float x);
long double coshl(long double x);
long double cosl(long double x);

double exp(double x);
float expf(float x);
long double expl(long double x);

double exp2(double x);
float exp2f(float x);
long double exp2l(long double x);

double floor(double x);
float floorf(float x);
long double floorl(long double x);

double fmod(double x, double y);
float fmodf(float x, float y);
long double fmodl(long double x, long double y);

double log(double x);
float logf(float x);
long double logl(long double x);

double log10(double x);
float log10f(float x);
long double log10l(long double x);

double modf(double x, double *ptr);
float modff(float x, float *ptr);
long double modfl(long double x, long double *ptr);

double pow(double x, double y);
float powf(float x, float y);
long double powl(long double x, long double y);

double sin(double x);
float sinf(float x);
long double sinl(long double x);

double sinh(double x);
float sinhf(float x);
long double sinhl(long double x);

double sqrt(double x);
float sqrtf(float x);
long double sqrtl(long double x);

double tan(double x);
float tanf(float x);
long double tanl(long double x);

double tanh(double x);
float tanhf(float x);
long double tanhl(long double x);

__Dancy_Header_End

#endif
