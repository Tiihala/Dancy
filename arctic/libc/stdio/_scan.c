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
 * libc/stdio/_scan.c
 *      Convert formatted input
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum length_modifier {
	length_modifier_none,
	length_modifier_hh,
	length_modifier_h,
	length_modifier_ell,
	length_modifier_ellell,
	length_modifier_j,
	length_modifier_z,
	length_modifier_tee,
	length_modifier_bell
};

struct scan_stream {
	int (*get)(void *);
	int (*unget)(int, void *);
	void *stream;
	size_t offset;
};

static int call_get(struct scan_stream *scan)
{
	int r = scan->get(scan->stream);

	if (r == '\0')
		return EOF;

	if (r != EOF)
		scan->offset += 1;

	return r;
}

static int call_unget(int c, struct scan_stream *scan)
{
	int r = scan->unget(c, scan->stream);

	if (r != EOF)
		scan->offset -= 1;

	return r;
}

static int get_format(const char *format, size_t *offset)
{
	int space = 0;
	int fc;

	while ((fc = (int)((unsigned char)format[*offset])) != '\0') {
		int fc_space = isspace(fc);

		space |= fc_space;
		*offset += 1;

		if (!fc_space) {
			if (space) {
				*offset -= 1;
				return ' ';
			}
			return fc;
		}
	}

	return (space) ? ' ' : EOF;
}

static int get_number(struct scan_stream *scan,
	char buffer[32], size_t field_width, int base)
{
	int c_min[3] = { '0', '0', '0' };
	int c_max[3] = { '9', '9', '9' };
	size_t width = 0;
	int r = 0;
	int i, j, c;

	buffer[0] = '\0';

	if (field_width == 0)
		field_width = __DANCY_SIZE_MAX;

	for (i = 0; /* void */; i++) {
		int status = 0;

		if (width >= field_width)
			return r;

		if ((c = call_get(scan)) == EOF)
			return r;

		width += 1;

		if (i == 0) {
			while (isspace(c)) {
				if ((c = call_get(scan)) == EOF)
					break;
			}

			if (c == '-' || c == '+') {
				buffer[i + 0] = (char)c;
				buffer[i + 1] = '\0';
				continue;
			}

			if (base == 0) {
				if (c == '0') {
					c_min[0] = '0',
					c_min[1] = '0';
					c_min[2] = '0';

					c_max[0] = '7',
					c_max[1] = '7';
					c_max[2] = '7';

					if (width >= field_width)
						return r;

					c = call_get(scan);
					if (c == EOF)
						return r;

					width += 1;

					if (c == 'x' || c == 'X') {
						c_min[0] = '0',
						c_min[1] = 'a';
						c_min[2] = 'A';

						c_max[0] = '9',
						c_max[1] = 'f';
						c_max[2] = 'F';

						if (width >= field_width)
							return r;

						c = call_get(scan);
						if (c == EOF)
							return r;

						width += 1;
					}
				}

			} else if (base == 8) {
				c_min[0] = '0',
				c_min[1] = '0';
				c_min[2] = '0';

				c_max[0] = '7',
				c_max[1] = '7';
				c_max[2] = '7';

			} else if (base == 16) {
				c_min[0] = '0',
				c_min[1] = 'a';
				c_min[2] = 'A';

				c_max[0] = '9',
				c_max[1] = 'f';
				c_max[2] = 'F';
			}
		}

		for (j = 0; j < 3; j++) {
			if (c >= c_min[j] && c <= c_max[j]) {
				status = 1;
				break;
			}
		}

		if (status == 0) {
			call_unget(c, scan);
			return r;
		}

		if (i < 28) {
			buffer[i + 0] = (char)c;
			buffer[i + 1] = '\0';
			r += 1;
		}
	}

	return r;
}

static void write_signed(int length, void *p, long long value)
{
	switch (length) {
		case length_modifier_none:
			*((int *)p) = (int)value;
			break;
		case length_modifier_hh:
			*((signed char *)p) = (signed char)value;
			break;
		case length_modifier_h:
			*((short *)p) = (short)value;
			break;
		case length_modifier_ell:
			*((long *)p) = (long)value;
			break;
		case length_modifier_ellell:
			*((long long *)p) = value;
			break;
		case length_modifier_j:
			*((long long *)p) = value;
			break;
		default:
			*((ssize_t *)p) = (ssize_t)value;
			break;
	}
}

static void write_unsigned(int length, void *p, unsigned long long value)
{
	switch (length) {
		case length_modifier_none:
			*((unsigned int *)p) = (unsigned int)value;
			break;
		case length_modifier_hh:
			*((unsigned char *)p) = (unsigned char)value;
			break;
		case length_modifier_h:
			*((unsigned short *)p) = (unsigned short)value;
			break;
		case length_modifier_ell:
			*((unsigned long *)p) = (unsigned long)value;
			break;
		case length_modifier_ellell:
			*((unsigned long long *)p) = value;
			break;
		case length_modifier_j:
			*((unsigned long long *)p) = value;
			break;
		default:
			*((size_t *)p) = (size_t)value;
			break;
	}
}

int __dancy_scanf(int (*get)(void *), int (*unget)(int, void *), void *stream,
	const char *format, va_list arg)
{
	struct scan_stream _scan;
	struct scan_stream *scan = &_scan;

	size_t offset = 0;
	int r = 0;
	int c, fc;

	scan->get = get;
	scan->unget = unget;
	scan->stream = stream;
	scan->offset = 0;

	while ((fc = get_format(format, &offset)) != EOF) {
		int test_matching_input = 0;
		int assignment_suppressing = 0;
		size_t field_width = 0;
		int length = length_modifier_none;

		/*
		 * A directive composed of white-space character(s).
		 */
		if (isspace(fc)) {
			while ((c = call_get(scan)) != EOF) {
				if (!isspace(c)) {
					call_unget(c, scan);
					break;
				}
			}
			continue;
		}

		if (fc == '%') {
			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
			if (fc == '%')
				test_matching_input = 1;
		} else {
			test_matching_input = 1;
		}

		if (test_matching_input) {
			if (isspace(fc)) {
				while ((c = call_get(scan)) != EOF) {
					if (!isspace(c)) {
						call_unget(c, scan);
						break;
					}
				}
				continue;
			}

			if ((c = call_get(scan)) == EOF || c != fc)
				return r;
			continue;
		}

		/*
		 * An optional assignment-suppressing character.
		 */
		if (fc == '*') {
			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
			assignment_suppressing = 1;
		}

		/*
		 * The maximum field width.
		 */
		while (fc >= '0' && fc <= '9') {
			size_t new_width = field_width;

			new_width *= 10;
			new_width += (size_t)(fc - '0');

			if (field_width > new_width)
				field_width = __DANCY_SIZE_MAX;
			else
				field_width = new_width;

			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
		}

		/*
		 * An optional length modifier.
		 */
		if (fc == 'h') {
			length = length_modifier_h;
			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
			if (fc == 'h')
				length = length_modifier_hh;
			else
				offset -= 1;
		} else if (fc == 'l') {
			length = length_modifier_ell;
			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
			if (fc == 'l')
				length = length_modifier_ellell;
			else
				offset -= 1;
		} else if (fc == 'j') {
			length = length_modifier_j;
		} else if (fc == 'z') {
			length = length_modifier_z;
		} else if (fc == 't') {
			length = length_modifier_tee;
		} else if (fc == 'L') {
			length = length_modifier_bell;
		}

		if (length != length_modifier_none) {
			if ((fc = get_format(format, &offset)) == EOF)
				return (r > 0) ? r : EOF;
		}

		if (fc == 'd') {
			long long value;
			char buffer[32];

			if (!get_number(scan, buffer, field_width, 10))
				return r;
			value = strtoll(&buffer[0], NULL, 10);

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_signed(length, p, value);
				r += 1;
			}

		} else if (fc == 'i') {
			long long value;
			char buffer[32];

			if (!get_number(scan, buffer, field_width, 0))
				return r;
			value = strtoll(&buffer[0], NULL, 0);

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_signed(length, p, value);
				r += 1;
			}

		} else if (fc == 'o') {
			long long value;
			char buffer[32];

			if (!get_number(scan, buffer, field_width, 8))
				return r;
			value = strtoll(&buffer[0], NULL, 8);

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_signed(length, p, value);
				r += 1;
			}

		} else if (fc == 'u') {
			unsigned long long value;
			char buffer[32];

			if (!get_number(scan, buffer, field_width, 10))
				return r;
			value = strtoull(&buffer[0], NULL, 10);

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_unsigned(length, p, value);
				r += 1;
			}

		} else if (fc == 'x' || fc == 'X') {
			unsigned long long value;
			char buffer[32];

			if (!get_number(scan, buffer, field_width, 16))
				return r;
			value = strtoull(&buffer[0], NULL, 16);

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_unsigned(length, p, value);
				r += 1;
			}

		} else if (fc == 's') {
			unsigned char *p = NULL;
			size_t width = 0;

			if (!assignment_suppressing) {
				p = va_arg(arg, unsigned char *);
				r += 1;
			}

			for (;;) {
				if ((c = call_get(scan)) == EOF)
					break;
				if (!isspace(c))
					break;
			}

			while (c != EOF) {
				if (field_width && width >= field_width)
					break;

				if (p != NULL)
					*p++ = (unsigned char)c;

				if ((c = call_get(scan)) == EOF)
					break;
				if (isspace(c))
					break;
			}

			if (p != NULL)
				*p = 0;

		} else if (fc == 'n') {
			long long value = (long long)scan->offset;

			if (!assignment_suppressing) {
				void *p = va_arg(arg, void *);
				write_signed(length, p, value);
			}

		} else {
			/*
			 * An unknown conversion specifier.
			 */
			const char *e = "__dancy_scanf: unknown specifier";
			fprintf(stderr, "%s \"%c\"\n", e, fc);
			exit(EXIT_FAILURE);
		}
	}

	return r;
}
