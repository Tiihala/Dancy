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
 * libc/spawn/sigdef.c
 *      The spawn attribute functions
 */

#include <spawn.h>

int posix_spawnattr_getsigdefault(
	const posix_spawnattr_t *attrp, sigset_t *sigdefault)
{
	return (*sigdefault = (sigset_t)attrp->__sigdef), 0;
}

int posix_spawnattr_setsigdefault(
	posix_spawnattr_t *attrp, const sigset_t *sigdefault)
{
	return (attrp->__sigdef = (unsigned long long)(*sigdefault)), 0;
}
