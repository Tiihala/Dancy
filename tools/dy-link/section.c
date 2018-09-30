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

#include "program.h"

static int section_data_size(struct options *opt, const char *name)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long secs = LE16(&buf[2]);

		while (secs--) {
			if (!strcmp((const char *)&buf[20], name)) {
				unsigned long add = LE32(&buf[20 + 16]);
				unsigned long bit = LE32(&buf[20 + 36]);
				int align = (int)((bit & 0x00F00000ul) >> 20);

				if (align)
					align = 1 << (align - 1);
				else
					align = 16;
				/*
				 * Handle section alignments correctly. The
				 * total size has the padding bytes included.
				 */
				if (total_size < INT_MAX - (align - 1)) {
					unsigned t1 = (unsigned)total_size;
					unsigned t2 = (unsigned)(align - 1);
					total_size = (int)((t1 + t2) & ~t2);
				} else {
					return INT_MAX;
				}
				if (add < (unsigned long)INT_MAX) {
					if (total_size < INT_MAX - (int)add)
						total_size += (int)add;
					else
						return INT_MAX;
				} else {
					return INT_MAX;
				}
			}
			buf += 40;
		}
	}
	return total_size;
}

static int section_reloc_size(struct options *opt, const char *name)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long secs = LE16(&buf[2]);

		while (secs--) {
			if (!strcmp((const char *)&buf[20], name)) {
				unsigned long nr_rel = LE16(&buf[20 + 32]);
				unsigned long add = nr_rel * 10ul;

				if (add < (unsigned long)INT_MAX) {
					if (total_size < INT_MAX - (int)add)
						total_size += (int)add;
					else
						return INT_MAX;
				} else {
					return INT_MAX;
				}
			}
			buf += 40;
		}
	}
	return total_size;
}

int section_check_sizes(struct options *opt)
{
	const char *sections[] = {
		".text",
		".rdata",
		".data",
		".bss"
	};
	int total_size = 0;
	int i;

	for (i = 0; i < (int)(sizeof(sections) / sizeof(sections[0])); i++) {
		int add1 = section_data_size(opt, sections[i]);
		int add2 = section_reloc_size(opt, sections[i]);

		if (add1 < INT_MAX - add2) {
			int add = add1 + add2;
			if (total_size < INT_MAX - add) {
				total_size += add;
				continue;
			}
		}
		return fputs("Error: total size\n", stderr), INT_MAX;
	}
	return total_size;
}

int section_sizeof_text(struct options *opt)
{
	return section_data_size(opt, ".text");
}

int section_sizeof_text_reloc(struct options *opt)
{
	return section_reloc_size(opt, ".text");
}

int section_sizeof_rdata(struct options *opt)
{
	return section_data_size(opt, ".rdata");
}

int section_sizeof_rdata_reloc(struct options *opt)
{
	return section_reloc_size(opt, ".rdata");
}

int section_sizeof_data(struct options *opt)
{
	return section_data_size(opt, ".data");
}

int section_sizeof_data_reloc(struct options *opt)
{
	return section_reloc_size(opt, ".data");
}
