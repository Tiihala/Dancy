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
 * epoch.c
 *      Thursday 1 January 1970 00:00:00
 */

#include <limits.h>
#include <stddef.h>

#if !defined (LLONG_MAX)
#error "Type long long is not available"
#endif

long long epoch_seconds(const char iso_8601_format[19]);
void epoch_convert(unsigned long long seconds, char iso_8601_format[19]);

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

static int read_number(const char *str, int len)
{
	int val = 0;
	int i;

	for (i = 0; i < len; i++) {
		char c = str[i];
		if (c < '0' || c > '9')
			return -1;
		val = (val * 10) + ((int)c - '0');
	}

	return val;
}

static void write_number(char *str, int len, unsigned int val)
{
	static const char *numbers = "0123456789";
	char buffer[4] = { '0', '0', '0', '0' };
	int i, j;

	if ((size_t)len > sizeof(buffer) / sizeof(buffer[0]))
		return;

	for (i = 0, j = 0; i < len; i++) {
		int r = (int)(val % 10);
		val = val / 10;
		if (val == 0 && r == 0)
			break;
		buffer[j++] = numbers[r];
	}

	for (i = 0, j = len - 1; j >= 0; i++, j--)
		str[i] = buffer[j];
}

long long epoch_seconds(const char iso_8601_format[19])
{
	int year, mon, day, hour, min, sec;
	long long unknown_format = LLONG_MIN, secs = 0;
	int err = 0;
	int i;

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	if (iso_8601_format[4] != '-' || iso_8601_format[7] != '-')
		return unknown_format;
	if (iso_8601_format[10] != 'T')
		return unknown_format;
	if (iso_8601_format[13] != ':' || iso_8601_format[16] != ':')
		return unknown_format;

	year = read_number(&iso_8601_format[0], 4);
	mon  = read_number(&iso_8601_format[5], 2);
	day  = read_number(&iso_8601_format[8], 2);
	hour = read_number(&iso_8601_format[11], 2);
	min  = read_number(&iso_8601_format[14], 2);
	sec  = read_number(&iso_8601_format[17], 2);

	err |= (mon  >= 1 && mon  <= 12) ? 0 : 1;
	err |= (day  >= 1 && day  <= 31) ? 0 : 1;
	err |= (hour >= 0 && hour <= 23) ? 0 : 1;
	err |= (min  >= 0 && min  <= 59) ? 0 : 1;
	err |= (sec  >= 0 && sec  <= 59) ? 0 : 1;

	if (err != 0 || year < 1970)
		return unknown_format;

	if (day > days(year,mon))
		return unknown_format;

	for (i = 1970; i < year; i++) {
		if (leap_year(i))
			secs += (long long)(86400 * 366);
		else
			secs += (long long)(86400 * 365);
	}

	for (i = 1; i < mon; i++)
		secs += (long long)(86400 * days(year, i));

	secs += (long long)(86400 * (day - 1));
	secs += (long long)(3600 * hour);
	secs += (long long)(60 * min);
	secs += (long long)(sec);

	return secs;
}

void epoch_convert(unsigned long long seconds, char iso_8601_format[19])
{
	unsigned int year = 9999, mon = 12, day = 31;
	unsigned int hour = 23, min = 59, sec = 59;

	if (seconds < 253402300799ull) {
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

		for (mon = 1; /* void */; mon++) {
			int t = days((int)year, (int)mon);
			unsigned long long s;

			s = (unsigned long long)(86400 * t);

			if (seconds < s)
				break;

			seconds -= s;
		}

		day = (unsigned int)((long)seconds / 86400L) + 1u;
		seconds = (unsigned long long)((long)seconds % 86400L);

		hour = (unsigned int)((long)seconds / 3600L);
		seconds = (unsigned long long)((long)seconds % 3600L);

		min = (unsigned int)((int)seconds / 60);
		sec = (unsigned int)((int)seconds % 60);
	}

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	iso_8601_format[4]  = '-';
	iso_8601_format[7]  = '-';
	iso_8601_format[10] = 'T';
	iso_8601_format[13] = ':';
	iso_8601_format[16] = ':';

	write_number(&iso_8601_format[0], 4, year);
	write_number(&iso_8601_format[5], 2, mon);
	write_number(&iso_8601_format[8], 2, day);
	write_number(&iso_8601_format[11], 2, hour);
	write_number(&iso_8601_format[14], 2, min);
	write_number(&iso_8601_format[17], 2, sec);
}
