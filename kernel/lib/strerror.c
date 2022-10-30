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
 * lib/strerror.c
 *      Maps the number in errnum to a message string
 */

#include <dancy.h>

static const char *error_array[] = {

#define DANCY_X(a, b) b,
#include <kernel/error.h>
#undef DANCY_X

};

char *strerror(int errnum)
{
	static const char *unknown_error = "**** unknown error ****";
	int size = (int)(sizeof(error_array) / sizeof(error_array[0]));

	if (errnum < 0 || errnum >= size)
		return (char *)((addr_t)unknown_error);

	return (char *)((addr_t)error_array[errnum]);
}
