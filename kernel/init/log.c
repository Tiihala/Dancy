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
 * init/log.c
 *      Boot log manager
 */

#include <dancy.h>

static size_t log_size;
static size_t log_max_size;
static size_t log_column;
static void *boot_log;

int log_init(size_t max_size)
{
	size_t aligned_size = (max_size + 0xFF) & 0x1FFFFF00;

	if (!aligned_size || aligned_size < max_size || boot_log != NULL)
		return 1;
	if ((boot_log = aligned_alloc(256, aligned_size)) == NULL)
		return 1;

	memset(boot_log, 0, aligned_size);
	log_max_size = aligned_size - 1;
	return 0;
}

void log_close(void)
{
	void *new_ptr;

	log_max_size = log_size;
	log_column = 0;

	new_ptr = realloc(boot_log, log_size);
	if (new_ptr != NULL)
		boot_log = new_ptr;
}

const char *log_get_data(void)
{
	return (const char *)boot_log;
}

size_t log_get_size(void)
{
	return log_size;
}

int log(const char *format, ...)
{
	char *out = (char *)boot_log;
	char buf[2048];
	int ret, i, j;

	va_list va;
	va_start(va, format);
	ret = vsnprintf(buf, 2048, format, va);
	va_end(va);

	for (i = 0; i < ret; i++) {
		if (buf[i] == '\t') {
			for (j = 0; (j == 0 || log_column & 3); j++ ) {
				if (log_size < log_max_size)
					out[log_size++] = ' ';
				log_column += 1;
			}
		} else if (buf[i] == '\n') {
			if (log_size < log_max_size)
				out[log_size++] = '\r';
			if (log_size < log_max_size)
				out[log_size++] = '\n';
			log_column = 0;
		} else {
			if (log_size < log_max_size)
				out[log_size++] = buf[i];
			log_column += 1;
		}
	}

	if (log_size >= log_max_size)
		return log_close(), 1;
	return (ret > 0) ? 0 : 1;
}
