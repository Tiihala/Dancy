/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/locale/conv.c
 *      The C Standard Library
 */

#include <limits.h>
#include <locale.h>

static struct lconv c;

static char decimal_point[] = ".";
static char empty_string[] = "";

struct lconv *localeconv(void)
{
	c.currency_symbol       = &empty_string[0];
	c.decimal_point         = &decimal_point[0];
	c.grouping              = &empty_string[0];
	c.int_curr_symbol       = &empty_string[0];
	c.mon_decimal_point     = &empty_string[0];
	c.mon_grouping          = &empty_string[0];
	c.mon_thousands_sep     = &empty_string[0];
	c.negative_sign         = &empty_string[0];
	c.positive_sign         = &empty_string[0];
	c.thousands_sep         = &empty_string[0];

	c.frac_digits           = CHAR_MAX;
	c.int_frac_digits       = CHAR_MAX;
	c.int_n_cs_precedes     = CHAR_MAX;
	c.int_n_sep_by_space    = CHAR_MAX;
	c.int_n_sign_posn       = CHAR_MAX;
	c.int_p_cs_precedes     = CHAR_MAX;
	c.int_p_sep_by_space    = CHAR_MAX;
	c.int_p_sign_posn       = CHAR_MAX;
	c.n_cs_precedes         = CHAR_MAX;
	c.n_sep_by_space        = CHAR_MAX;
	c.n_sign_posn           = CHAR_MAX;
	c.p_cs_precedes         = CHAR_MAX;
	c.p_sep_by_space        = CHAR_MAX;
	c.p_sign_posn           = CHAR_MAX;

	return &c;
}
