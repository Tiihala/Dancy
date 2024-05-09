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
 * libc/glob/glob.c
 *      Pathname pattern-matching types
 */

#include <glob.h>
#include <stdlib.h>
#include <string.h>

static int append_gl_pathv(glob_t *pglob, const char *path)
{
	void *d = NULL;

	if (pglob->gl_offs + pglob->gl_pathc + 2 > pglob->_dancy) {
		const size_t add = 128;
		void *new_gl_pathv;

		if (pglob->_dancy > 0x10000000)
			return GLOB_NOSPACE;

		new_gl_pathv = malloc((pglob->_dancy + add) * sizeof(char *));

		if (new_gl_pathv == NULL)
			return GLOB_NOSPACE;

		if (pglob->gl_pathv != NULL) {
			size_t size = pglob->_dancy * sizeof(char *);
			memcpy(new_gl_pathv, pglob->gl_pathv, size);
			free(pglob->gl_pathv);
		}

		pglob->_dancy += add;
		pglob->gl_pathv = new_gl_pathv;
	}

	if (path != NULL)
		d = strdup(path);

	pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc + 0] = d;
	pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc + 1] = NULL;

	pglob->gl_pathc += (size_t)((d == NULL) ? 0 : 1);

	if (path != NULL && d == NULL)
		return GLOB_NOSPACE;

	return 0;
}

int glob(const char *pattern, int flags,
	int (*errfunc)(const char *epath, int eerrno), glob_t *pglob)
{
	if ((flags & GLOB_DOOFFS) == 0)
		pglob->gl_offs = 0;

	if (pglob->gl_offs > 0x10000000)
		return GLOB_NOSPACE;

	if ((flags & GLOB_APPEND) == 0) {
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		pglob->_dancy = 0;

		if (append_gl_pathv(pglob, NULL))
			return GLOB_NOSPACE;
	}

	/*
	 * A temporary implementation.
	 */
	{
		(void)errfunc;

		if ((flags & GLOB_NOCHECK) == 0)
			return GLOB_NOMATCH;

		if (append_gl_pathv(pglob, pattern))
			return GLOB_NOSPACE;
	}

	return 0;
}

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
