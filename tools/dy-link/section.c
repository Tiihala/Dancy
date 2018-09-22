/*
 * Copyright (c) 2018 Antti Tiihala
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
 * dy-link/section.c
 *      Linker for Dancy operating system
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"

static int get_size(struct options *opt, const char *name)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long secs = LE16(&buf[2]);
		int size = 0;

		while (secs--) {
			if (!strcmp((char *)&buf[20], name)) {
				unsigned long add = LE32(&buf[20 + 16]);
				/*
				 * Round up to the nearest multiple of 16. It
				 * has been checked (in validate.c) that this
				 * calculation can not overflow.
				 */
				size += (int)((add + 15ul) & 0xFFFFFFF0ul);
			}
			buf += 40;
		}
		if (total_size < INT_MAX - size)
			total_size += size;
		else
			return INT_MAX;
	}
	return total_size;
}

int section_check_sizes(struct options *opt)
{
	int total_size = 0;
	char *sections[] = {
		".text",
		".rdata",
		".data",
		".bss"
	};
	int i;

	for (i = 0; i < (int)(sizeof(sections) / sizeof(sections[0])); i++) {
		int add = get_size(opt, sections[i]);
		if (total_size < INT_MAX - add)
			total_size += add;
		else
			return fputs("Error: total size\n", stderr), INT_MAX;
	}
	return total_size;
}

int section_get_text_size(struct options *opt)
{
	return get_size(opt, ".text");
}

int section_get_rdata_size(struct options *opt)
{
	return get_size(opt, ".rdata");
}

int section_get_data_size(struct options *opt)
{
	return get_size(opt, ".data");
}

int section_get_bss_size(struct options *opt)
{
	return get_size(opt, ".bss");
}
