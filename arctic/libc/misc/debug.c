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
 * libc/misc/debug.c
 *      Print formatted output and trap to debugger
 */

#include <__dancy/core.h>
#include <stdio.h>

void __dancy_debug(const char *format, ...)
{
	char buffer[4096];
	int r;

	va_list va;
	va_start(va, format);

	r = vsnprintf(&buffer[0], sizeof(buffer), format, va);
	va_end(va);

	if (r >= (int)sizeof(buffer))
		r = (int)sizeof(buffer) - 1;

	if (r > 0)
		(void)fwrite(&buffer[0], 1, (size_t)r, stderr);

	(void)fflush(NULL);

	__dancy_debug_asm();
}
