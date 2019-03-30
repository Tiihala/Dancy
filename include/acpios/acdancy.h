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
 * acpios/acdancy.h
 *      Header of ACPI Component Architecture
 */

#ifndef ACPIOS_ACDANCY_H
#define ACPIOS_ACDANCY_H

#ifndef va_arg
#include <dancy/stdarg.h>
#endif
#include <dancy/ctype.h>
#include <dancy/limits.h>
#include <dancy/string.h>
#include <dancy/types.h>

#if defined (DANCY_32)
#define ACPI_MACHINE_WIDTH 32

#elif defined (DANCY_64)
#define ACPI_MACHINE_WIDTH 64

#else
#error Define DANCY_32 or DANCY_64
#endif

#if !defined (ULLONG_MAX)
#error The platform does not support unsigned long long
#endif

#define ACPI_USE_NATIVE_MATH64
#define ACPI_USE_NATIVE_DIVIDE

#define ACPI_USE_SYSTEM_CLIBRARY

char *strcat(char *s1, const char *s2);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *s1, const char *s2, size_t n);

#endif
