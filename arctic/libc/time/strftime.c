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
 * libc/time/strftime.c
 *      The C Standard Library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct record {
	const char *short_name;
	const char *name;
};

static struct record days[8] = {
	{ "Sun", "Sunday" },
	{ "Mon", "Monday" },
	{ "Tue", "Tuesday" },
	{ "Wed", "Wednesday" },
	{ "Thu", "Thursday" },
	{ "Fri", "Friday" },
	{ "Sat", "Saturday" },
	{ NULL, NULL },
};

static struct record months[16] = {
	{ "Jan", "January" },
	{ "Feb", "February" },
	{ "Mar", "March" },
	{ "Apr", "April" },
	{ "May", "May" },
	{ "Jun", "June" },
	{ "Jul", "July" },
	{ "Aug", "August" },
	{ "Sep", "September" },
	{ "Oct", "October" },
	{ "Nov", "November" },
	{ "Dec", "December" },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
};

static void write_output(char *s, size_t size, size_t *out, const char *data)
{
	while (data != NULL) {
		char c = *data++;

		if (*out >= size) {
			s[0] = '\0';
			break;
		}

		if ((s[*out] = c) == '\0')
			break;

		*out += 1;
	}
}

size_t strftime(char *s, size_t size,
	const char *format, const struct tm *timeptr)
{
	size_t out = 0;
	char buffer[32];
	int i, j;

	if (size == 0)
		return 0;

	s[0] = '\0';

	for (;;) {
		char c = *format++;

		if (c == '\0')
			break;

		if (c != '%') {
			char data[2] = { c, '\0' };
			write_output(s, size, &out, &data[0]);
			continue;
		}

		if ((c = *format++) == '\0')
			break;

		while (c == 'E' || c == '0') {
			if ((c = *format++) == '\0')
				return strlen(s);
		}

		if (c == '%') {
			write_output(s, size, &out, "%");
			continue;
		}

		if (c == 'a') {
			i = timeptr->tm_wday & 7;
			write_output(s, size, &out, days[i].short_name);
			continue;
		}

		if (c == 'A') {
			i = timeptr->tm_wday & 7;
			write_output(s, size, &out, days[i].name);
			continue;
		}

		if (c == 'b' || c == 'h') {
			i = timeptr->tm_mon & 15;
			write_output(s, size, &out, months[i].short_name);
			continue;
		}

		if (c == 'B') {
			i = timeptr->tm_mon & 15;
			write_output(s, size, &out, months[i].name);
			continue;
		}

		if (c == 'c') {
			i = timeptr->tm_wday & 7;
			j = timeptr->tm_mon & 15;

			snprintf(&buffer[0], sizeof(buffer),
				"%s %s %2d %02d:%02d:%02d %04d",
				days[i].short_name, months[j].short_name,
				timeptr->tm_mday, timeptr->tm_hour,
				timeptr->tm_min, timeptr->tm_sec,
				timeptr->tm_year + 1900);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'C') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", (timeptr->tm_year + 1900) / 100);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'd') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_mday);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'D') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d/%02d/%02d", timeptr->tm_mon + 1,
				timeptr->tm_mday, timeptr->tm_year % 100);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'e') {
			snprintf(&buffer[0], sizeof(buffer),
				"%2d", timeptr->tm_mday);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'F') {
			snprintf(&buffer[0], sizeof(buffer),
				"%04d-%02d-%02d", timeptr->tm_year + 1900,
				timeptr->tm_mon + 1, timeptr->tm_mday);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'H') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_hour);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'I') {
			int _12_hour_clock = 12;

			if (timeptr->tm_hour != 0 && timeptr->tm_hour != 12)
				_12_hour_clock = timeptr->tm_hour % 12;

			snprintf(&buffer[0], sizeof(buffer),
				"%02d", _12_hour_clock);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'j') {
			snprintf(&buffer[0], sizeof(buffer),
				"%03d", timeptr->tm_yday + 1);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'm') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_mon + 1);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'M') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_min);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'n') {
			write_output(s, size, &out, "\n");
			continue;
		}

		if (c == 'p') {
			if (timeptr->tm_hour < 12)
				write_output(s, size, &out, "AM");
			else
				write_output(s, size, &out, "PM");
			continue;
		}

		if (c == 'r') {
			int _12_hour_clock = 12;

			if (timeptr->tm_hour != 0 && timeptr->tm_hour != 12)
				_12_hour_clock = timeptr->tm_hour % 12;

			snprintf(&buffer[0], sizeof(buffer),
				"%02d:%02d:%02d %s", _12_hour_clock,
				timeptr->tm_min, timeptr->tm_sec,
				(timeptr->tm_hour < 12) ? "AM" : "PM");
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'R') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d:%02d", timeptr->tm_hour,
				timeptr->tm_min);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'S') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_sec);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 't') {
			write_output(s, size, &out, "\t");
			continue;
		}

		if (c == 'T') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d:%02d:%02d", timeptr->tm_hour,
				timeptr->tm_min, timeptr->tm_sec);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'u') {
			int weekday = 7;

			if (timeptr->tm_wday != 0)
				weekday = timeptr->tm_wday;

			snprintf(&buffer[0], sizeof(buffer),
				"%d", weekday);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'w') {
			snprintf(&buffer[0], sizeof(buffer),
				"%d", timeptr->tm_wday);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'x') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d/%02d/%02d", timeptr->tm_mon + 1,
				timeptr->tm_mday, timeptr->tm_year % 100);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'X') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d:%02d:%02d", timeptr->tm_hour,
				timeptr->tm_min, timeptr->tm_sec);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'y') {
			snprintf(&buffer[0], sizeof(buffer),
				"%02d", timeptr->tm_year % 100);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'Y') {
			snprintf(&buffer[0], sizeof(buffer),
				"%04d", timeptr->tm_year + 1900);
			write_output(s, size, &out, &buffer[0]);
			continue;
		}

		if (c == 'z') {
			write_output(s, size, &out, "+0000");
			continue;
		}

		if (c == 'Z') {
			write_output(s, size, &out, "");
			continue;
		}

		/*
		 * Other unsupported conversion specifiers.
		 */
		{
			const char *e = "strftime: unknown specifier";
			fprintf(stderr, "%s \"%c\"\n", e, c);
			exit(EXIT_FAILURE);
		}
	}

	return strlen(s);
}
