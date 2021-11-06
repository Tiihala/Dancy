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

#include <boot/uefi.h>

char *uefi_log;
size_t uefi_log_size;

static size_t log_max_size;
static size_t log_column;

static char boot_log_buffer[0x20000];

void u_log_dump(void)
{
	const char *ptr = uefi_log;
	uint32_t row, rows;
	size_t length = 0;

	b_output_control(0, B_CLEAR_CONSOLE);
	rows = (video_active != 0) ? video_rows : (uint32_t)gOutputRows;

	while (*ptr != '\0') {
		length += 1;

		if (*ptr++ != '\n')
			continue;

		row = (uint32_t)(b_output_control(0, B_GET_CURSOR) >> 8);

		if (row >= rows - 1) {
			EFI_INPUT_KEY key;

			b_output_string_hl("\r--More--", 0);

			while (u_read_key(&key))
				u_stall(20);

			u_print("\r%8s\n", " ");
			b_output_control(((rows - 2) << 8), B_SET_CURSOR);
		}
		u_print("%.*s", (int)length, (ptr - length));
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
