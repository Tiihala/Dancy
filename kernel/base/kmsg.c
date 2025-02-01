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
 * base/kmsg.c
 *      Ring buffer for kernel messages
 */

#include <dancy.h>

static void kmsg_write_char_locked(int c)
{
	int i = kernel->kmsg.state >> 1;

	kernel->kmsg.buffer[i] = (char)c;

	if (i < (int)kernel->kmsg.size)
		kernel->kmsg.state += 2;
	else
		kernel->kmsg.state = 1;
}

static int kmsg_write_locked(const char *m)
{
	static char prefix[64];

	unsigned int level = 6;
	int i, r;

	if (*m >= '0' && *m <= '9') {
		level = 0;
		do {
			level *= 10;
			level += (unsigned int)(*m - '0');
			m += 1;
		} while (*m >= '0' && *m <= '9');
	}

	if (*m == '\0' || *m == '\n' || *m == ' ')
		return 0;

	r = snprintf(&prefix[0], sizeof(prefix),
		"\t%u,%llu,%llu,-;", level, kernel->kmsg.counter,
		(unsigned long long)timer_read() * 1000ull);

	if (r < 9 || r >= (int)sizeof(prefix))
		return 0;

	for (i = 0; i < r; i++)
		kmsg_write_char_locked((int)prefix[i]);

	while (r < 256) {
		int c = (int)(*((unsigned char *)m));
		int write_hex = 0;

		if (c == '\0')
			break;

		m += 1;

		if (c == '\n' && *m == '\0')
			break;

		if (c < 0x20 || c > 0x7E)
			write_hex = 1;

		if (c == ' ' && (*m == '\0' || *m == '\n'))
			write_hex = 1;

		if (write_hex) {
			static const char *hex = "0123456789abcdef";

			kmsg_write_char_locked('\\');
			kmsg_write_char_locked('x');
			kmsg_write_char_locked((int)hex[c >> 4]);
			kmsg_write_char_locked((int)hex[c & 15]);

			r += 4;
			continue;
		}

		kmsg_write_char_locked(c);
		r += 1;
	}

	kmsg_write_char_locked('\n');
	r += 1;

	i = kernel->kmsg.state >> 1;
	kernel->kmsg.buffer[i] = '\0';

	kernel->kmsg.counter += 1;

	if (kernel->kmsg.event)
		event_signal(kernel->kmsg.event);

	return r;
}

int kmsg_write(const char *message)
{
	void *lock_local = &kernel->kmsg.lock;
	int r;

	spin_enter(&lock_local);

	r = kmsg_write_locked(message);

	spin_leave(&lock_local);

	return r;
}

int printk(const char *format, ...)
{
	char buffer[128];
	int r;

	va_list va;
	va_start(va, format);
	r = vsnprintf(&buffer[0], sizeof(buffer), format, va);
	va_end(va);

	if (r <= 0)
		return 0;

	if (r > (int)sizeof(buffer) - 4)
		r = (int)sizeof(buffer) - 4;

	buffer[r] = '\0';

	return kmsg_write(&buffer[0]);
}
