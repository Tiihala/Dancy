/*
 * Copyright (c) 2020 Antti Tiihala
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
 * dy-mcopy/mcopy.c
 *      Program for adding files into FAT file system images
 */

#include "program.h"

int mcopy(struct options *opt)
{
	const char *file_name = opt->operands[1];
	size_t size = source_file_size;
	unsigned char record[32];
	int r;

	if (file_name[0] != ':' || file_name[1] != ':'|| file_name[2] == '\0')
		return opt->error = "missing path/destination-file", 1;
	file_name += 2;

	if ((r = fat_open(opt->fat, 0, file_name, "wb")) != 0)
		return fat_error(r), 1;

	if ((r = fat_write(opt->fat, 0, &size, source_file)) != 0) {
		(void)fat_close(opt->fat, 0);
		(void)fat_remove(opt->fat, file_name);
		return fat_error(r), 1;
	}

	if (opt->read_only) {
		if ((r = fat_control(opt->fat, 0, 0, record)) != 0) {
			(void)fat_close(opt->fat, 0);
			return fat_error(r), 1;
		}

		record[11] = (unsigned char)(record[11] | 0x01);

		if ((r = fat_control(opt->fat, 0, 1, record)) != 0) {
			(void)fat_close(opt->fat, 0);
			return fat_error(r), 1;
		}
	}

	return (void)fat_close(opt->fat, 0), 0;
}
