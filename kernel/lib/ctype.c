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
 * lib/ctype.c
 *      C standard library functions
 */

#include <dancy.h>

int isalnum(int c)
{
	if (isalpha(c) || isdigit(c))
		return 1;
	return 0;
}

int isalpha(int c)
{
	if (islower(c) || isupper(c))
		return 1;
	return 0;
}

int isblank(int c)
{
	if (c == ' ' || c == '\t')
		return 1;
	return 0;
}

int iscntrl(int c)
{
	if ((c >= 0x00 && c <= 0x1F) || c == 0x7F)
		return 1;
	return 0;
}

int isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}

int isgraph(int c)
{
	if (c != ' ' && isprint(c))
		return 1;
	return 0;
}

int islower(int c)
{
	if (c >= 'a' && c <= 'z')
		return 1;
	return 0;
}

int isprint(int c)
{
	if (c >= 0x20 && c <= 0x7E)
		return 1;
	return 0;
}

int ispunct(int c)
{
	if (isprint(c) && !isalnum(c) && !isspace(c))
		return 1;
	return 0;
}

int isspace(int c)
{
	if (c == ' ' || c == '\f' || c == '\n')
		return 1;
	if (c == '\r' || c == '\t' || c == '\v')
		return 1;
	return 0;
}

int isupper(int c)
{
	if (c >= 'A' && c <= 'Z')
		return 1;
	return 0;
}

int isxdigit(int c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'a' && c <= 'f')
		return 1;
	if (c >= 'A' && c <= 'F')
		return 1;
	return 0;
}

int tolower(int c)
{
	if (!isupper(c))
		return c;
	return c + 32;
}

int toupper(int c)
{
	if (!islower(c))
		return c;
	return c - 32;
}
