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
 * libc/spawn/pgroup.c
 *      The spawn attribute functions
 */

#include <errno.h>
#include <spawn.h>

int posix_spawnattr_getpgroup(const posix_spawnattr_t *attrp, pid_t *pgroup)
{
	return (*pgroup = (pid_t)attrp->_pgroup), 0;
}

int posix_spawnattr_setpgroup(posix_spawnattr_t *attrp, pid_t pgroup)
{
	if (pgroup < 0)
		return (errno = EINVAL), EINVAL;

	return (attrp->_pgroup = (unsigned long long)pgroup), 0;
}
