/*
 * Copyright (c) 2019, 2022 Antti Tiihala
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
 * libc/stdio/snprintf.c
 *      The C Standard Library
 */

#include <__dancy/ssize.h>
#include <limits.h>
#include <stdio.h>

int snprintf(char *s, size_t n, const char *format, ...)
{
	int ret;
	va_list va;
	va_start(va, format);
	ret = vsnprintf(s, n, format, va);
	va_end(va);
	return ret;
}

int vsnprintf(char *s, size_t n, const char *format, va_list arg)
{
	static const char *hex_normal = "0123456789abcdef";
	static const char *hex_upcase = "0123456789ABCDEF";
	size_t in = 0, out = 0;
	char buf[64];

	if (n-- < 1)
		return -1;

	while (out < n) {
		char c = format[in++];
		unsigned field_width, precision, precision_set;
		int flag_minus, flag_plus;
		int flag_space, flag_zero, flag_alt;
		int negative, nochars;

		long long ival;
		unsigned long long uval;

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
		} length;

		if (c == '\0')
			break;
		if (c != '%') {
			s[out++] = c;
			continue;
		}

		flag_minus = 0, flag_plus = 0;
		flag_space = 0, flag_zero = 0, flag_alt = 0;

		for (c = format[in++]; /* void */; c = format[in++]) {
			if (c == '\0')
				return s[out] = '\0', -1;
			else if (c == '-')
				flag_minus = 1;
			else if (c == '+')
				flag_plus = 1;
			else if (c == ' ')
				flag_space = 1;
			else if (c == '0')
				flag_zero = 1;
			else if (c == '#')
				flag_alt = 1;
			else
				break;
		}

		if (c == '%') {
			s[out++] = c;
			continue;
		}

		if (c == '*') {
			int val = va_arg(arg, int);
			if (val < 0) {
				if (val > INT_MIN)
					field_width = (unsigned)(0 - val);
				else
					field_width = (unsigned)INT_MAX;
				flag_minus = 1;
			} else {
				field_width = (unsigned)val;
			}
			if ((c = format[in++]) == '\0')
				return s[out] = '\0', -1;
		} else {
			for (field_width = 0; /* void */; c = format[in++]) {
				if (c == '\0')
					return s[out] = '\0', -1;
				if (!(c >= '0' && c <= '9'))
					break;
				field_width *= 10u;
				field_width += (unsigned)(c - '0');
			}
		}

		precision = 0, precision_set = 0;

		if (c == '.') {
			precision_set = 1;
			if ((c = format[in++]) == '\0')
				return s[out] = '\0', -1;
		}
		if (precision_set && c == '*') {
			int val = va_arg(arg, int);
			if (val < 0)
				precision_set = 0;
			else
				precision = (unsigned)val;
			if ((c = format[in++]) == '\0')
				return s[out] = '\0', -1;
		} else if (precision_set) {
			for (/* void */; /* void */; c = format[in++]) {
				if (c == '\0')
					return s[out] = '\0', -1;
				if (!(c >= '0' && c <= '9'))
					break;
				precision *= 10u;
				precision += (unsigned)(c - '0');
			}
		}

		length = length_modifier_none;

		if (c == 'h') {
			length = length_modifier_h;
			if (format[in] == 'h') {
				length = length_modifier_hh;
				in += 1;
			}
		} else if (c == 'l') {
			length = length_modifier_ell;
			if (format[in] == 'l') {
				length = length_modifier_ellell;
				in += 1;
			}
		} else if (c == 'j') {
			length = length_modifier_j;
		} else if (c == 'z') {
			length = length_modifier_z;
		} else if (c == 't') {
			length = length_modifier_tee;
		} else if (c == 'L') {
			length = length_modifier_bell;
		}

		if (length != length_modifier_none)
			c = format[in++];

		if (c == 'd' || c == 'i') {
			char pad = ' ', sign = '+';
			size_t base = out;
			unsigned len = 0, max;
			size_t i;

			if (length < length_modifier_ell)
				ival = va_arg(arg, int);
			else if (length == length_modifier_ell)
				ival = va_arg(arg, long);
			else if (length == length_modifier_ellell)
				ival = va_arg(arg, long long);
			else if (length == length_modifier_j)
				ival = va_arg(arg, long long);
			else
				ival = va_arg(arg, ssize_t);

			negative = 0;
			if (ival < 0) {
				sign = '-';
				negative = 1;
			}
			if (flag_space && !flag_plus) {
				if (!negative)
					sign = ' ';
				flag_plus = 1;
			}

			nochars = 0;
			if (precision_set) {
				if (!ival && !precision)
					nochars = 1;
				flag_zero = 0;
			}
			if (flag_zero && !flag_minus)
				pad = '0';

			for (i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
				int remainder = (int)(ival % 10);
				ival = ival / 10;
				if (i != 0 && ival == 0 && remainder == 0)
					break;
				if (remainder < 0)
					remainder = (0 - remainder);
				buf[len++] = hex_normal[remainder];
			}
			max = (len > precision) ? len : precision;

			if (!flag_minus && max < field_width) {
				unsigned pads = field_width - max;
				if (negative || flag_plus) {
					if (flag_zero && !flag_minus) {
						if (out < n)
							s[out++] = sign;
						sign = 0;
					}
					pads -= 1;
				}
				if (nochars)
					pads += 1;
				while (out < n && pads--)
					s[out++] = pad;
			}

			if (sign && (negative || flag_plus) && out < n)
				s[out++] = sign;

			for (i = 0; i < (size_t)(max - len) && out < n; i++)
				s[out++] = '0';

			while (!nochars && len-- && out < n)
				s[out++] = buf[len];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 'u') {
			char pad = ' ';
			size_t base = out;
			unsigned len = 0, max;
			size_t i;

			if (length < length_modifier_ell)
				uval = va_arg(arg, unsigned int);
			else if (length == length_modifier_ell)
				uval = va_arg(arg, unsigned long);
			else if (length == length_modifier_ellell)
				uval = va_arg(arg, unsigned long long);
			else if (length == length_modifier_j)
				uval = va_arg(arg, unsigned long long);
			else
				uval = va_arg(arg, size_t);

			nochars = 0;
			if (precision_set) {
				if (!uval && !precision)
					nochars = 1;
				flag_zero = 0;
			}
			if (flag_zero && !flag_minus)
				pad = '0';

			for (i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
				int remainder = (int)(uval % 10);
				uval = uval / 10;
				if (i != 0 && uval == 0 && remainder == 0)
					break;
				buf[len++] = hex_normal[remainder];
			}
			max = (len > precision) ? len : precision;

			if (!flag_minus && max < field_width) {
				unsigned pads = field_width - max;
				if (nochars)
					pads += 1;
				while (out < n && pads--)
					s[out++] = pad;
			}

			for (i = 0; i < (size_t)(max - len) && out < n; i++)
				s[out++] = '0';

			while (!nochars && len-- && out < n)
				s[out++] = buf[len];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 'x' || c == 'X' || c == 'p') {
			char pad = ' ';
			size_t base = out;
			unsigned len = 0, max;
			size_t i;

			if (c == 'p') {
				uval = va_arg(arg, size_t);
				precision = (unsigned)(sizeof(void *) * 2);
				precision_set = 1;
			} else if (length < length_modifier_ell) {
				uval = va_arg(arg, unsigned int);
			} else if (length == length_modifier_ell) {
				uval = va_arg(arg, unsigned long);
			} else if (length == length_modifier_ellell) {
				uval = va_arg(arg, unsigned long long);
			} else if (length == length_modifier_j) {
				uval = va_arg(arg, unsigned long long);
			} else {
				uval = va_arg(arg, size_t);
			}

			nochars = 0;
			if (precision_set) {
				if (!uval && !precision)
					nochars = 1;
				flag_zero = 0;
			}
			if (flag_zero && !flag_minus)
				pad = '0';

			for (i = 2; i < sizeof(buf) / sizeof(buf[0]); i++) {
				int remainder = (int)(uval % 16);
				uval = uval / 16;
				if (i != 2 && uval == 0 && remainder == 0)
					break;
				if (c == 'x')
					buf[len++] = hex_normal[remainder];
				else
					buf[len++] = hex_upcase[remainder];
			}
			if (flag_alt) {
				buf[len++] = (c == 'x') ? 'x' : 'X';
				buf[len++] = '0';
			}
			max = (len > precision) ? len : precision;

			if (!flag_minus && max < field_width) {
				unsigned pads = field_width - max;
				if (nochars)
					pads += 1;
				while (out < n && pads--)
					s[out++] = pad;
			}

			for (i = 0; i < (size_t)(max - len) && out < n; i++)
				s[out++] = '0';

			while (!nochars && len-- && out < n)
				s[out++] = buf[len];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 'o') {
			char pad = ' ';
			size_t base = out;
			unsigned len = 0, max;
			size_t i;

			if (length < length_modifier_ell)
				uval = va_arg(arg, unsigned int);
			else if (length == length_modifier_ell)
				uval = va_arg(arg, unsigned long);
			else if (length == length_modifier_ellell)
				uval = va_arg(arg, unsigned long long);
			else if (length == length_modifier_j)
				uval = va_arg(arg, unsigned long long);
			else
				uval = va_arg(arg, size_t);

			nochars = 0;
			if (precision_set) {
				if (!uval && !precision)
					nochars = 1;
				flag_zero = 0;
			}
			if (flag_zero && !flag_minus)
				pad = '0';

			for (i = 1; i < sizeof(buf) / sizeof(buf[0]); i++) {
				int remainder = (int)(uval % 8);
				uval = uval / 8;
				if (i != 1 && uval == 0 && remainder == 0)
					break;
				buf[len++] = hex_normal[remainder];
			}
			if (flag_alt && buf[len - 1] != '0')
				buf[len++] = '0';
			max = (len > precision) ? len : precision;

			if (!flag_minus && max < field_width) {
				unsigned pads = field_width - max;
				if (nochars)
					pads += 1;
				while (out < n && pads--)
					s[out++] = pad;
			}

			for (i = 0; i < (size_t)(max - len) && out < n; i++)
				s[out++] = '0';

			while (!nochars && len-- && out < n)
				s[out++] = buf[len];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 'c') {
			char pad = ' ';
			size_t base = out;
			unsigned len = 0;

			if (length == length_modifier_none)
				uval = (size_t)va_arg(arg, int);
			else
				uval = va_arg(arg, size_t);

			buf[len++] = (char)((unsigned char)uval);

			if (!flag_minus && len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = pad;
			}

			if (out < n)
				s[out++] = buf[0];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 's') {
			char pad = ' ';
			size_t base = out;
			unsigned len = 0, max = 0;
			const char *p;

			p = va_arg(arg, const char *);

			if (precision_set)
				max = precision;
			else
				max -= 1;

			while (p[len] != '\0' && len < max)
				len += 1;
			max = len;
			len = 0;

			if (!flag_minus && max < field_width) {
				unsigned pads = field_width - max;
				while (out < n && pads--)
					s[out++] = pad;
			}

			while (len < max && out < n)
				s[out++] = p[len++];

			len = (unsigned)(out - base);
			if (len < field_width) {
				unsigned pads = field_width - len;
				while (out < n && pads--)
					s[out++] = ' ';
			}
			continue;
		}

		if (c == 'n') {
			void *p = va_arg(arg, void *);
			int n_out = (out < INT_MAX) ? (int)out : INT_MAX;

			if (length == length_modifier_none) {
				*((int *)p) = n_out;
			} else if (length == length_modifier_hh) {
				if (n_out < CHAR_MAX) {
					*((char *)p) = (char)n_out;
				} else {
					*((char *)p) = CHAR_MAX;
				}
			} else if (length == length_modifier_h) {
				if (n_out < SHRT_MAX) {
					*((short *)p) = (short)n_out;
				} else {
					*((short *)p) = SHRT_MAX;
				}
			}  else if (length == length_modifier_ell) {
				*((long *)p) = (long)n_out;
			}  else if (length == length_modifier_ellell) {
				*((long long *)p) = (long long)n_out;
			}  else if (length == length_modifier_j) {
				*((long long *)p) = (long long)n_out;
			} else {
				*((ssize_t *)p) = (ssize_t)n_out;
			}
			continue;
		}

		/*
		 * Other conversion specifiers are not supported.
		 */
		return s[out] = '\0', -1;
	}

	s[out] = '\0';
	return (out < INT_MAX) ? (int)out : INT_MAX;
}
