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
 * uefi/log.c
 *      Boot log manager (UEFI)
 */

#include <uefi.h>

char *uefi_log;
size_t uefi_log_size;

static size_t log_max_size;
static size_t log_column;

static char boot_log_buffer[0x20000];

void u_log_dump(void)
{
	const char *ptr = uefi_log;
	uint64_t column, row;
	size_t length = 0;

	u_clear_screen();

	while (*ptr != '\0') {
		length += 1;

		if (*ptr++ != '\n')
			continue;

		u_get_cursor(&column, &row);

		if (row >= gOutputRows - 1) {
			EFI_INPUT_KEY key;

			u_set_colors(0x70);
			u_print("\r--More--");
			u_set_colors(0x07);

			while (u_read_key(&key))
				u_stall(20);

			u_print("\r%8s\n", " ");
			u_set_cursor(0, gOutputRows - 2);
		}
		u_print("%.*s", length, (ptr - length));
		length = 0;
	}
}

void u_log_init(void)
{
	if (uefi_log)
		memset(&boot_log_buffer[0], 0, sizeof(boot_log_buffer));

	uefi_log = &boot_log_buffer[0];
	uefi_log_size = 0;

	log_max_size = sizeof(boot_log_buffer) - 1;
	log_column = 0;
}

void u_log(const char *format, ...)
{
	char *out = uefi_log;
	char buf[4096];
	int ret, i, j;

	va_list va;
	va_start(va, format);
	ret = vsnprintf(buf, 4096, format, va);
	va_end(va);

	for (i = 0; i < ret; i++) {
		if (buf[i] == '\t') {
			for (j = 0; (j == 0 || log_column & 3); j++ ) {
				if (uefi_log_size < log_max_size)
					out[uefi_log_size++] = ' ';
				log_column += 1;
			}
		} else if (buf[i] == '\n') {
			if (uefi_log_size < log_max_size)
				out[uefi_log_size++] = '\r';
			if (uefi_log_size < log_max_size)
				out[uefi_log_size++] = '\n';
			log_column = 0;
		} else {
			if (uefi_log_size < log_max_size)
				out[uefi_log_size++] = buf[i];
			log_column += 1;
		}
	}
}
