/*
 * Copyright (c) 2025 Antti Tiihala
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
 * libc/regex/regerror.c
 *      Map error codes to printable strings
 */

#include <regex.h>
#include <string.h>

size_t regerror(int errcode, const regex_t *preg,
	char *errbuf, size_t errbuf_size)
{
	const char *m = "UNSPECIFIED";

	(void)preg;

	switch (errcode) {
		case REG_BADBR:    m = "REG_BADBR"    ; break;
		case REG_BADPAT:   m = "REG_BADPAT"   ; break;
		case REG_BADRPT:   m = "REG_BADRPT"   ; break;
		case REG_EBRACE:   m = "REG_EBRACE"   ; break;
		case REG_EBRACK:   m = "REG_EBRACK"   ; break;
		case REG_ECOLLATE: m = "REG_ECOLLATE" ; break;
		case REG_ECTYPE:   m = "REG_ECTYPE"   ; break;
		case REG_EESCAPE:  m = "REG_EESCAPE"  ; break;
		case REG_EPAREN:   m = "REG_EPAREN"   ; break;
		case REG_ERANGE:   m = "REG_ERANGE"   ; break;
		case REG_ESPACE:   m = "REG_ESPACE"   ; break;
		case REG_ESUBREG:  m = "REG_ESUBREG"  ; break;
		case REG_NOMATCH:  m = "REG_NOMATCH"  ; break;
	}

	if (errbuf != NULL && errbuf_size != 0) {
		size_t i = 0;

		while (m[i] != '\0' && i + 1 < errbuf_size)
			errbuf[i] = m[i], i += 1;

		errbuf[i] = '\0';
	}

	return strlen(m) + (size_t)1;
}
