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
 * init/bprint.c
 *      Use boot loader services to display text on screen
 */

#include <dancy.h>

int b_print(const char *format, ...)
{
	char buf[4096];
	char *ptr = &buf[0];
	unsigned size = 0;
	int ret, i;

	va_list va;
	va_start(va, format);
	ret = vsnprintf(buf, 4096, format, va);
	va_end(va);

	for (i = 0; i < ret; i++) {
		if (buf[i] == '\n') {
			if (i == ret - 1) {
				buf[i + 0] = '\r';
				buf[i + 1] = '\n';
				b_output_string(ptr, size + 2);
				return (ret > 0) ? 0 : 1;
			}
			if (size) {
				b_output_string(ptr, size);
				ptr += size;
				size = 0;
			}
			ptr += 1;
			b_output_string("\r\n", 2);
		} else {
			size += 1;
		}
	}
	if (size)
		b_output_string(ptr, size);
	return (ret > 0) ? 0 : 1;
}
