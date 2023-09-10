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
 * libc/time/_time.c
 *      The __dancy_time function
 */

#include <string.h>
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

void __dancy_time(time_t timer, struct tm *result)
{
	const unsigned long long max_seconds = 253402300799ull;
	unsigned long long seconds = 0;

	int year, mon, day;
	int hour, min, sec;
	int wday, yday;

	if (timer > 0)
		seconds = (unsigned long long)timer;

	if (seconds > max_seconds)
		seconds = max_seconds;

	wday = (int)(((seconds / 86400) + 4) % 7);

	for (year = 1970; /* void */; year++) {
		unsigned long long s;

		if (leap_year((int)year))
			s = (unsigned long long)(86400 * 366);
		else
			s = (unsigned long long)(86400 * 365);

		if (seconds < s)
			break;

		seconds -= s;
	}

	yday = (int)((unsigned int)seconds / 86400u);

	for (mon = 1; /* void */; mon++) {
		int t = days(year, mon);
		unsigned long long s;

		s = (unsigned long long)(86400 * t);

		if (seconds < s)
			break;

		seconds -= s;
	}

	day = ((int)seconds / 86400) + 1;
	seconds = (unsigned long long)((int)seconds % 86400);

	hour = (int)seconds / 3600;
	seconds = (unsigned long long)((int)seconds % 3600);

	min = (int)seconds / 60;
	sec = (int)seconds % 60;

	memset(result, 0, sizeof(*result));
	result->tm_isdst = -1;

	result->tm_sec  = sec;
	result->tm_min  = min;
	result->tm_hour = hour;
	result->tm_mday = day;
	result->tm_mon  = mon - 1;
	result->tm_year = year - 1900;
	result->tm_wday = wday;
	result->tm_yday = yday;
}
