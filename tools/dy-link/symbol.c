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
 * dy-link/symbol.c
 *      Linker for Dancy operating system
 */

#include "program.h"

int symbol_check_sizes(struct options *opt)
{
	int s1 = symbol_string_size(opt);
	int s2 = symbol_table_size(opt);

	if (s1 < INT_MAX - s2)
		return s1 + s2;
	return fputs("Error: total symbol table size\n", stderr), INT_MAX;
}

int symbol_copy_table(struct options *opt, unsigned char *out)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned char *sym = buf + LE32(&buf[8]);
		int syms = (int)LE32(&buf[12]);
		int j;

		for (j = 0; j < syms; j++) {
			unsigned long addr = 0ul;
			unsigned long sec = LE16(&sym[12]);
			int skip = 0;
			int add = 18;

			if (sec > 0x0000ul && sec < 0xFFFEul) {
				int s = (int)sec;
				s = section_reloc(opt, i, s, &addr);
				skip = s ? 0 : 1;
				sec = (unsigned long)s;
			}

			if (LE32(&sym[0])) {
				memcpy(&out[0], &sym[0], 8);
			} else {
				W_LE16(&out[4], i);
				W_LE16(&out[6], j);
			}

			/*
			 * Copy the main entries and translate the values
			 * and section numbers.
			 */
			if (!skip) {
				unsigned long s8 = LE32(&sym[8]) + addr;
				unsigned long s12 = sec;
				unsigned long s14 = LE16(&sym[14]);
				unsigned char s16 = sym[16];

				W_LE32(&out[8], s8);
				W_LE16(&out[12], s12);
				W_LE16(&out[14], s14);
				out[16] = s16;
			} else {
				unsigned char s16 = sym[16];
				out[16] = s16;
			}

			/*
			 * Copy the extra entries as they are. Those will be
			 * eventually deleted from the final object.
			 */
			if (sym[17]) {
				size_t n = (size_t)((int)sym[17] * 18);
				memcpy(out + 18, sym + 18, n);
				out[17] = sym[17];
				j += (int)sym[17];
				add += (int)n;
			}
			out += add, sym += add;
			total_size += add;
		}
	}
	return total_size;
}

static int qsort_compare(const void *s1, const void *s2)
{
	const struct { unsigned long i; void *s; void *n; } *sym1 = s1;
	const struct { unsigned long i; void *s; void *n; } *sym2 = s2;
	const char *str1 = sym1->n;
	const char *str2 = sym2->n;
	char buf1[9];
	char buf2[9];

	if (*((unsigned char *)sym1->s)) {
		memcpy(&buf1[0], sym1->s, 8);
		buf1[8] = '\0';
		str1 = &buf1[0];
	}
	if (*((unsigned char *)sym2->s)) {
		memcpy(&buf2[0], sym2->s, 8);
		buf2[8] = '\0';
		str2 = &buf2[0];
	}

	/*
	 * Compare class values and names.
	 */
	{
		unsigned char t1 = *((unsigned char *)sym1->s + 16);
		unsigned char t2 = *((unsigned char *)sym2->s + 16);
		int r;

		if ((unsigned)t1 != (unsigned)t2) {
			if ((unsigned)t1 == 2u)
				return 1;
			if ((unsigned)t2 == 2u)
				return -1;
		}
		/*
		 * Only externals are sorted by name.
		 */
		if ((unsigned)t1 == 2u) {
			if ((r = strcmp(str1, str2)) != 0)
				return r;
		}
	}

	/*
	 * Compare section numbers.
	 */
	{
		unsigned char *p1 = (unsigned char *)sym1->s;
		unsigned char *p2 = (unsigned char *)sym2->s;
		unsigned long t1 = LE16(&p1[12]);
		unsigned long t2 = LE16(&p2[12]);

		if (t1 != t2)
			return (t1 < t2) ? -1 : 1;
	}

	/*
	 * Compare symbol values.
	 */
	{
		unsigned char *p1 = (unsigned char *)sym1->s;
		unsigned char *p2 = (unsigned char *)sym2->s;
		unsigned long t1 = LE32(&p1[8]);
		unsigned long t2 = LE32(&p2[8]);

		if (t1 != t2)
			return (t1 < t2) ? -1 : 1;
	}

	/*
	 * Compare type values.
	 */
	{
		unsigned char *p1 = (unsigned char *)sym1->s;
		unsigned char *p2 = (unsigned char *)sym2->s;
		unsigned long t1 = LE16(&p1[14]);
		unsigned long t2 = LE16(&p2[14]);

		if (t1 != t2)
			return (t1 < t2) ? -1 : 1;
	}

	/*
	 * Make sure that the order is always defined.
	 */
	return (sym1->i < sym2->i) ? -1 : 1;
}

static int delete_record(unsigned char *obj, int sym)
{
	int symtab = (int)LE32(&obj[8]);
	int syms = (int)LE32(&obj[12]);
	int i;

	if (sym >= syms)
		return 1;
	/*
	 * Check that relocation entries do not point
	 * to the symbol that will be deleted.
	 */
	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			if (sym == (int)LE32(&r_off[4]))
				return 2;
			r_off += 10;
		}
	}

	/*
	 * Delete the symbol from the table.
	 */
	{
		unsigned char *dst = obj + symtab + (sym * 18) + 0;
		unsigned char *src = obj + symtab + (sym * 18) + 18;
		size_t size = (size_t)((syms - sym - 1) * 18);

		if (size)
			memmove(dst, src, size);
		dst = obj + symtab + (syms * 18) - 18;
		memset(dst, 0, 18u);
	}

	/*
	 * Fix relocation entries.
	 */
	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			int s = (int)LE32(&r_off[4]);
			if (sym < s--)
				W_LE32(&r_off[4], s);
			r_off += 10;
		}
	}

	syms -= 1;
	W_LE32(&obj[12], syms);
	return 0;
}

static void reloc_update1(unsigned char *obj, const void *arr, int n)
{
	const struct { unsigned long i; void *s; void *n; } *table = arr;
	int i;
	int j;

	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			unsigned long s = LE32(&r_off[4]);
			for (j = 0; j < n; j++) {
				if (s == table[j].i) {
					W_LE32(&r_off[4], j);
					break;
				}
			}
			r_off += 10;
		}
	}
}

static void reloc_update2(unsigned char *obj, int old_s, int new_s)
{
	int i;

	for (i = 0; i < 3; i++) {
		unsigned char *sec = obj + 20 + (i * 40);
		unsigned char *r_off = obj + LE32(&sec[24]);
		unsigned long r_num = LE16(&sec[32]);

		while (r_num--) {
			int s = (int)LE32(&r_off[4]);
			if (s == old_s)
				W_LE32(&r_off[4], new_s);
			r_off += 10;
		}
	}
}

static char *get_long_name(struct options *opt, unsigned char *sym)
{
	static char empty[] = "";

	if (!LE32(&sym[0])) {
		int t1 = (int)LE16(&sym[4]);
		int t2 = (int)LE16(&sym[6]);
		unsigned char *t3 = opt->mfiles[t1].data;
		unsigned char *t4 = t3 + LE32(&t3[8]);
		unsigned char *t5 = t4 + LE32(&t3[12]) * 18ul;
		unsigned char *t6 = t4 + t2 * 18;

		return (char *)(t5 + LE32(&t6[4]));
	}
	return &empty[0];
}

static int match(struct options *opt, unsigned char *s1, unsigned char *s2)
{
	const char *str1;
	const char *str2;
	char buf1[9];
	char buf2[9];

	if (!memcmp(s1, s2, 8u))
		return 1;

	if (*s1) {
		memcpy(&buf1[0], s1, 8u);
		buf1[8] = '\0';
		str1 = &buf1[0];
	} else {
		str1 = get_long_name(opt, s1);
	}
	if (*s2) {
		memcpy(&buf2[0], s2, 8u);
		buf2[8] = '\0';
		str2 = &buf2[0];
	} else {
		str2 = get_long_name(opt, s2);
	}
	return (!strcmp(str1, str2)) ? 1 : 0;
}

static void reloc_error(unsigned sec)
{
	if (sec == 0xFFFEu)
		fputs("Error: relocations to debug symbol\n", stderr);
	else if (sec == 0xFFFFu)
		fputs("Error: relocations to absolute symbol\n", stderr);
	else
		fputs("Error: relocations to unknown symbol\n", stderr);
}

int symbol_process(struct options *opt, unsigned char *obj)
{
	int symtab = (int)LE32(&obj[8]);
	int i;

	/*
	 * Delete extra records.
	 */
	for (i = 0; i < (int)LE32(&obj[12]); /* void */) {
		unsigned char *sym = obj + symtab + (i * 18);
		unsigned extra = (unsigned)sym[17];

		if (extra) {
			while (extra--) {
				if (delete_record(obj, i + 1))
					return reloc_error(0), 1;
			}
			sym[17] = 0u;
			continue;
		}
		i += 1;
	}

	/*
	 * Delete unnecessary symbols. All but external symbols are
	 * deleted if there are no relocation entries pointing to
	 * them. Do not allow "section 0" if it is not an external
	 * symbol. Also, absolute and debug symbols cannot be used
	 * in relocation entries. This policy can be changed if it
	 * turns out to be a problem in the future.
	 */
	for (i = 0; i < (int)LE32(&obj[12]); /* void */) {
		unsigned char *sym = obj + symtab + (i * 18);
		unsigned sec = (unsigned)LE16(&sym[12]);

		if (sec == 0 && (unsigned)sym[16] != 2u) {
			if ((unsigned)sym[16] != 0xFFu) {
				if (delete_record(obj, i))
					return reloc_error(0), 1;
				continue;
			}
		}
		if (sec > 0x0004u) {
			if (delete_record(obj, i))
				return reloc_error(sec), 1;
			continue;
		}
		if (sec == 0 || (unsigned)sym[16] != 2u) {
			if (!delete_record(obj, i))
				continue;
		}
		i += 1;
	}

	/*
	 * Handle common symbols.
	 */
	if (LE32(&obj[12])) {
		int syms = (int)LE32(&obj[12]);
		int j;

		for (i = 0; i < syms; i++) {
			unsigned char *s1 = obj + symtab + (i * 18);
			unsigned char *s2 = NULL;
			unsigned long add = LE32(&s1[8]);
			unsigned long bss_size = LE32(&obj[156]);
			int err = 0;

			if (!add || LE16(&s1[12]))
				continue;
			/*
			 * Find the largest storage allocation.
			 */
			for (j = 0; j < syms; j++) {
				s2 = obj + symtab + (j * 18);
				if (!LE32(&s2[8]) || LE16(&s2[12]))
					continue;
				if (!match(opt, s1, s2))
					continue;
				if (add < LE32(&s2[8]))
					add = LE32(&s2[8]);
			}

			/*
			 * Adjust the alignment of the symbol.
			 */
			if (add == 2ul || add == 4ul || add == 8ul) {
				if (bss_size < UINT_MAX - (add - 1ul)) {
					bss_size += (add - 1ul);
					bss_size &= ~(add - 1ul);
				} else {
					err = 1;
				}
			} else if (add > 1ul && add < 8ul) {
				if (bss_size < UINT_MAX - 7ul) {
					bss_size += 7ul;
					bss_size &= ~(7ul);
				} else {
					err = 1;
				}
			} else if (add != 1ul) {
				if (bss_size < UINT_MAX - 15ul) {
					bss_size += 15ul;
					bss_size &= ~(15ul);
				} else {
					err = 1;
				}
			}

			W_LE32(&s1[8], bss_size);
			W_LE16(&s1[12], 4ul);

			for (j = 0; j < syms; j++) {
				s2 = obj + symtab + (j * 18);
				if (!LE32(&s2[8]) || LE16(&s2[12]))
					continue;
				if (!match(opt, s1, s2))
					continue;
				W_LE32(&s2[8], bss_size);
				W_LE16(&s2[12], 4ul);
			}

			if (bss_size < UINT_MAX - add)
				bss_size += add;
			else
				err = 1;

			if (err || !(bss_size < 0x7FFFFFFFul)) {
				fputs("Error: .bss overflow\n", stderr);
				return 1;
			}
			W_LE32(&obj[156], bss_size);
		}
	}

	/*
	 * Link symbols.
	 */
	if (LE32(&obj[12])) {
		int syms = (int)LE32(&obj[12]);
		int j;

		for (i = 0; i < syms; i++) {
			unsigned char *s1 = obj + symtab + (i * 18);
			unsigned char *s2 = NULL;
			unsigned type = (unsigned)s1[16];

			if (LE32(&s1[8]) || LE16(&s1[12]))
				continue;
			if (type != 2u && type != 0xFFu)
				continue;

			for (j = 0; j < syms; j++) {
				s2 = obj + symtab + (j * 18);
				if ((unsigned)s2[16] == type && LE16(&s2[12]))
					if (match(opt, s1, s2))
						break;
				s2 = NULL;
			}
			if (s2 != NULL && s1 != s2)
				memcpy(s1, s2, 18u);
			if (!LE16(&s1[12]))
				s1[16] = 2u;
		}
	}

	/*
	 * Insert section symbols.
	 */
	{
		int syms = (int)LE32(&obj[12]);
		unsigned char *sym = obj + symtab + (syms * 18);

		/*
		 * Use temporary names.
		 */
		memset(&sym[0], 0u, (4u * 18u));
		sym[0] = 0x54u, sym[12] = 1u, sym[16] = 3u, sym += 18;
		sym[0] = 0x54u, sym[12] = 2u, sym[16] = 3u, sym += 18;
		sym[0] = 0x54u, sym[12] = 3u, sym[16] = 3u, sym += 18;
		sym[0] = 0x54u, sym[12] = 4u, sym[16] = 3u, sym += 18;

		syms += 4;
		W_LE32(&obj[12], syms);
	}

	/*
	 * Sort all symbols.
	 */
	{
		struct { unsigned long i; void *s; void *n; } *sym;
		size_t syms = (size_t)LE32(&obj[12]);
		void *arr = calloc(syms, sizeof(*sym));
		void *buf = calloc(syms, 18u);

		if (!arr || !buf) {
			fputs("Error: not enough memory\n", stderr);
			free(buf), free(arr);
			return 1;
		}
		for (i = 0, sym = arr; i < (int)syms; i++) {
			unsigned char *s = obj + symtab + (i * 18);
			sym->i = (unsigned long)i;
			sym->s = s;
			sym->n = get_long_name(opt, s);
			sym += 1;
		}
		qsort(arr, syms, sizeof(*sym), qsort_compare);
		for (i = 0, sym = arr; i < (int)syms; i++) {
			memcpy((unsigned char *)buf + (i * 18), sym->s, 18u);
			sym += 1;
		}
		memcpy(obj + symtab, buf, (syms * 18u));
		reloc_update1(obj, arr, (int)syms);
		free(buf), free(arr);
	}

	/*
	 * Merge duplicate symbols.
	 */
	{
		for (i = 0; i < ((int)LE32(&obj[12]) - 1); /* void */) {
			unsigned char *s1 = obj + symtab + ((i + 0) * 18);
			unsigned char *s2 = obj + symtab + ((i + 1) * 18);

			if (memcmp(&s1[8], &s2[8], 6u)) {
				i += 1;
				continue;
			}
			if ((unsigned)s1[16] == 2u && !match(opt, s1, s2)) {
				i += 1;
				continue;
			}
			reloc_update2(obj, i + 1, i);
			if (delete_record(obj, i + 1))
				return reloc_error(0), 1;
		}
	}

	/*
	 * Handle non-external symbols and detect multiply defined externals.
	 */
	{
		int allow_ext = strcmp(opt->arg_f, "obj") ? 0 : 1;
		int syms = (int)LE32(&obj[12]);
		int i_t = 1, i_r = 1, i_d = 1, i_b = 1;
		int unresolved = 0;

		for (i = 0; i < syms; i++) {
			unsigned char *s = obj + symtab + (i * 18);
			unsigned s_num = (unsigned)LE16(&s[12]);

			if ((unsigned)s[16] == 2u)
				break;
			if (s_num < 1 || s_num > 4) {
				fprintf(stderr, "Error: %u\n", s_num);
				return 1;
			}
			memset(&s[0], 0u, 8u);
			if (!LE32(&s[8])) {
				if (s_num == 1u)
					strcpy((char *)&s[0], ".text");
				else if (s_num == 2u)
					strcpy((char *)&s[0], ".rdata");
				else if (s_num == 3u)
					strcpy((char *)&s[0], ".data");
				else if (s_num == 4u)
					strcpy((char *)&s[0], ".bss");
			} else {
				if (s_num == 1u)
					sprintf((char *)&s[0], "_t%i", i_t++);
				else if (s_num == 2u)
					sprintf((char *)&s[0], "_r%i", i_r++);
				else if (s_num == 3u)
					sprintf((char *)&s[0], "_d%i", i_d++);
				else if (s_num == 4u)
					sprintf((char *)&s[0], "_b%i", i_b++);
			}
			s[14] = 0u, s[15] = 0u, s[16] = 3u;
		}
		while (i < syms - 1) {
			unsigned char *s1 = obj + symtab + ((i + 0) * 18);
			unsigned char *s2 = obj + symtab + ((i + 1) * 18);
			unsigned long t1 = LE32(&s1[0]);
			unsigned long t2 = LE32(&s2[0]);

			if (!allow_ext && !LE16(&s1[12])) {
				const char *name = (const char *)s1;
				fputs("Error: unresolved symbol \"", stderr);
				if (t1) {
					fprintf(stderr, "%.8s", name);
				} else {
					name = get_long_name(opt, s1);
					fprintf(stderr, "%s", name);
				}
				fputs("\"\n", stderr);
				unresolved = 1;
			}
			if (t1 && t2 && (t1 != t2)) {
				i += 1;
				continue;
			}
			if (match(opt, s1, s2)) {
				const char *name = (const char *)s1;
				fputs("Error: symbol \"", stderr);
				if (t1) {
					fprintf(stderr, "%.8s", name);
				} else {
					name = get_long_name(opt, s1);
					fprintf(stderr, "%s", name);
				}
				fputs("\" multiply defined\n", stderr);
				return 1;
			}
			i += 1;
		}
		if (unresolved)
			return 1;
	}
	return 0;
}

int symbol_string_size(struct options *opt)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long off = LE32(&buf[8]);
		unsigned long syms = LE32(&buf[12]);
		unsigned long add;

		if (!off || !syms)
			continue;
		buf += off;
		buf += syms * 18ul;

		add = LE32(&buf[0]);
		if (add >= 4ul && add < (unsigned long)INT_MAX) {
			if (total_size < INT_MAX - (int)add) {
				total_size += (int)(add - 4ul);
				continue;
			}
		}
		return INT_MAX;
	}
	return (total_size) ? total_size + 4 : 0;
}

int symbol_table_size(struct options *opt)
{
	int total_size = 0;
	int i;

	for (i = 0; i < opt->nr_mfiles; i++) {
		unsigned char *buf = opt->mfiles[i].data;
		unsigned long add = LE32(&buf[12]) * 18ul;

		if (total_size < INT_MAX - (int)add)
			total_size += (int)add;
		else
			return INT_MAX;
	}
	return (total_size / 18 <= 0x7FF0) ? total_size : INT_MAX;
}
