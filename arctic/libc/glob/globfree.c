/*
 * Copyright (c) 2024 Antti Tiihala
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
 * libc/glob/globfree.c
 *      Free all resources allocated by glob()
 */

#include <glob.h>
#include <stdlib.h>

void globfree(glob_t *pglob)
{
	size_t i;

	for (i = 0; i < pglob->gl_offs; i++)
		pglob->gl_pathv[i] = NULL;

	for (i = 0; i < pglob->gl_pathc; i++) {
		char **p = &pglob->gl_pathv[pglob->gl_offs + i];
		free(*p), *p = NULL;
	}

	free(pglob->gl_pathv);

	pglob->gl_pathc = 0;
	pglob->gl_pathv = NULL;
	pglob->gl_offs = 0;
	pglob->_dancy = 0;
}
