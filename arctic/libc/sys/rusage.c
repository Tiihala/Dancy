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
 * libc/sys/rusage.c
 *      Get information about resource utilization
 */

#include <errno.h>
#include <string.h>
#include <sys/resource.h>

static int not_implemented(struct rusage *r_usage)
{
	if (r_usage != NULL)
		memset(r_usage, 0, sizeof(*r_usage));
	return 0;
}

int getrusage(int who, struct rusage *r_usage)
{
	if (who == RUSAGE_SELF || who == RUSAGE_CHILDREN)
		return not_implemented(r_usage);

	return (errno = EINVAL), -1;
}
