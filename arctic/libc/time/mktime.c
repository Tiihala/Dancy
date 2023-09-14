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
 * libc/time/mktime.c
 *      The C Standard Library
 */

#include <errno.h>
#include <time.h>

static int leap_year(int year)
{
	if ((year % 4) != 0)
		return 0;
	else if ((year % 100) != 0)
		return 1;
	else if ((year % 400) != 0)
		return 0;

	return 1;
}

static int days(int year, int month)
{
	int d;

	switch (month) {
		case  1: d = 31; break;
		case  2: d = leap_year(year) ? 29 : 28; break;
		case  3: d = 31; break;
		case  4: d = 30; break;
		case  5: d = 31; break;
		case  6: d = 30; break;
		case  7: d = 31; break;
		case  8: d = 31; break;
		case  9: d = 30; break;
		case 10: d = 31; break;
		case 11: d = 30; break;
		case 12: d = 31; break;
		default: d = 0;  break;
	}

	return d;
}

static int check_overflow_range(int *output, int input)
{
	if (input < 0 || input > 0x7FFF)
		return (*output = 0), 1;
	return (*output = input), 0;
}

time_t mktime(struct tm *timeptr)
{
	unsigned long long r = 0;
	int year, mon, day, hour, min, sec;
	int i = 0;

	i += check_overflow_range(&sec, timeptr->tm_sec);
	i += check_overflow_range(&min, timeptr->tm_min);
	i += check_overflow_range(&hour, timeptr->tm_hour);

	i += check_overflow_range(&day, timeptr->tm_mday);
	i += check_overflow_range(&mon, timeptr->tm_mon);
	i += check_overflow_range(&year, timeptr->tm_year);

	if (i != 0)
		return (errno = EOVERFLOW), -1;

	mon  = mon + 1;
	year = year + 1900;

	if (year < 1970 || day < 1)
		return (errno = EOVERFLOW), -1;

	for (i = 1970; i < year; i++) {
		if (leap_year(i))
			r += (unsigned long long)(86400 * 366);
		else
			r += (unsigned long long)(86400 * 365);
	}

	for (i = 1; i < mon; i++) {
		int d_year = year + (i / 12);
		int d_month = ((i - 1) % 12) + 1;

		r += (unsigned long long)(86400 * days(d_year, d_month));
	}

	r += (unsigned long long)(86400 * (day - 1));
	r += (unsigned long long)(3600 * hour);
	r += (unsigned long long)(60 * min);
	r += (unsigned long long)(sec);

	if (r > 253402300799ull)
		return (errno = EOVERFLOW), -1;

	__dancy_time((time_t)r, timeptr);

	return (time_t)r;
}
