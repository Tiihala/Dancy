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

struct iterate {
	struct options *opt;
	const char *name;
	unsigned obj;
	unsigned sec;
	unsigned group;
};

static void iterate_init(struct iterate *it, void *opt, const void *name)
{
	it->opt = opt;
	it->name = name;
	it->obj = 0u;
	it->sec = 0u;
	it->group = 0u;
}

static unsigned char *iterate_next(struct iterate *it)
{
	unsigned i;

	for (i = it->obj; i < (unsigned)it->opt->nr_mfiles; i++) {
		unsigned char *dat = it->opt->mfiles[i].data;
		unsigned secs = LE16(&dat[2]);
		unsigned j;

		for (j = it->sec; j < secs; j++) {
			unsigned char *sec = dat + 20u + j * 40u;
			if (!strncmp((const char *)sec, it->name, 8u)) {
				it->obj = i;
				it->sec = ++j;
				return sec;
			}
		}
		it->sec = 0u;
	}
	return NULL;
}

int section_data_size(struct options *opt, const char *name)
{
	int total_size = 0;
	unsigned char *buf;
	struct iterate it;

	iterate_init(&it, opt, name);
	while ((buf = iterate_next(&it)) != NULL) {
		unsigned long add = LE32(&buf[16]);
		unsigned long bit = LE32(&buf[36]);
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
	return total_size;
}

int section_reloc_size(struct options *opt, const char *name)
{
	int total_size = 0;
	unsigned char *buf;
	struct iterate it;

	iterate_init(&it, opt, name);
	while ((buf = iterate_next(&it)) != NULL) {
		unsigned long nr_rel = LE16(&buf[32]);
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
	return total_size;
}

int section_group(struct options *opt)
{
	return 0;
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
		total_size = INT_MAX;
		break;
	}
	if (total_size < INT_MAX && (unsigned long)total_size < 0x7FFFFFFFul)
		return total_size;
	return fputs("Error: total size of sections\n", stderr), INT_MAX;
}

int section_copy_d(struct options *opt, const char *name, unsigned char *out)
{
	int total_size = 0;
	unsigned char *buf;
	struct iterate it;

	iterate_init(&it, opt, name);
	while ((buf = iterate_next(&it)) != NULL) {
		unsigned char *dat = opt->mfiles[it.obj].data;
		unsigned long add = LE32(&buf[16]);
		unsigned long off = LE32(&buf[20]);
		unsigned long bit = LE32(&buf[36]);
		int align = (int)((bit & 0x00F00000ul) >> 20);

		if (align)
			align = 1 << (align - 1);
		else
			align = 16;
		/*
		 * Handle section alignments.
		 */
		{
			unsigned t1 = (unsigned)total_size;
			unsigned t2 = (unsigned)(align - 1);
			total_size = (int)((t1 + t2) & ~t2);
		}
		/*
		 * Set the offset so we can refer to the
		 * correct data in the final object file.
		 */
		W_LE32(&buf[12], total_size);
		/*
		 * Copy the section data.
		 */
		if (out && add) {
			void *t1 = (void *)(out + total_size);
			void *t2 = (void *)(dat + off);
			size_t t3 = (size_t)add;
			memcpy(t1, (const void *)t2, t3);
		}
		total_size += (int)add;
	}
	return total_size;
}

int section_copy_r(struct options *opt, const char *name, unsigned char *out)
{
	int total_size = 0;
	unsigned char *buf;
	struct iterate it;

	iterate_init(&it, opt, name);
	while ((buf = iterate_next(&it)) != NULL) {
		unsigned char *dat = opt->mfiles[it.obj].data;
		unsigned long t_off = 0ul;
		unsigned long s_off = LE32(&buf[12]);
		unsigned long r_off = LE32(&buf[24]);
		unsigned long r_num = LE16(&buf[32]);
		unsigned char *src = dat + r_off;
		unsigned char *dst = out + total_size;
		unsigned objs = it.obj;

		/*
		 * Figure out the global table offset.
		 */
		while (objs--) {
			unsigned char *d = opt->mfiles[objs].data;
			t_off += LE32(&d[12]);
		}
		/*
		 * Translate the section relocations.
		 */
		total_size += (int)r_num * 10;
		while (r_num--) {
			unsigned long r1 = LE32(&src[0]);
			unsigned long r2 = LE32(&src[4]);
			unsigned long r3 = LE16(&src[8]);

			r1 += s_off;
			r2 += t_off;
			W_LE32(&dst[0], r1);
			W_LE32(&dst[4], r2);
			W_LE16(&dst[8], r3);
			src += 10, dst += 10;
		}
	}
	return total_size;
}

int section_reloc(struct options *opt, int obj, int sec, unsigned long *addr)
{
	unsigned char *dat = opt->mfiles[obj].data + 20 + ((sec - 1) * 40);

	*addr = LE32(&dat[12]);

	if (!strcmp((char *)dat, ".text"))
		return 1;
	if (!strcmp((char *)dat, ".rdata"))
		return 2;
	if (!strcmp((char *)dat, ".data"))
		return 3;
	if (!strcmp((char *)dat, ".bss"))
		return 4;
	return 0;
}
