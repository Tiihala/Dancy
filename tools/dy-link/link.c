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
 * dy-link/link.c
 *      Linker for Dancy operating system
 */

#include "program.h"

static int end(struct options *opt, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!out || !size)
		return 1;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return free(out), 1;
		}
		is_stdout = 0;
	}
	if ((errno = 0, fwrite(out, 1u, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return free(out), 1;
	}
	free(out);
	errno = 0;

	if (is_stdout)
		return fflush(fp) ? perror("Error"), 1 : 0;
	return fclose(fp) ? perror("Error"), 1 : 0;
}

static int get_pre_size(void)
{
	/*
	 * The file header and section entries, i.e. text,
	 * rdata, data, and bss. Use the maximum value,
	 * assuming that all sections are available.
	 */
	return 20 + 4 * 40;
}

static void add_with_align(int *off, int add)
{
	const unsigned u15 = 15u;
	int t = *off;

	t = t + add;
	t = (int)(((unsigned)t + u15) & ~u15);
	*off = t;
}

static int set_max_size(struct options *opt, int *size)
{
	int add;

	/*
	 * The file header and section entries.
	 */
	*size = get_pre_size();

	/*
	 * Section .text data and relocation.
	 */
	if (*size < INT_MAX - (add = section_sizeof_text(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.text)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_sizeof_text_reloc(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.text reloc)\n", stderr), 1;

	/*
	 * Section .rdata data and relocation.
	 */
	if (*size < INT_MAX - (add = section_sizeof_rdata(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.rdata)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_sizeof_rdata_reloc(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.rdata reloc)\n", stderr), 1;

	/*
	 * Section .data data and relocation.
	 */
	if (*size < INT_MAX - (add = section_sizeof_data(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.data)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_sizeof_data_reloc(opt)))
		*size += add;
	else
		return fputs("Error: overflow (.data reloc)\n", stderr), 1;

	/*
	 * The symbol table.
	 */
	if (*size < INT_MAX - (add = symbol_sizeof_table(opt)))
		*size += add;
	else
		return fputs("Error: overflow (symbol table)\n", stderr), 1;

	/*
	 * The string table.
	 */
	if (*size < INT_MAX - (add = symbol_sizeof_string(opt)))
		*size += add;
	else
		return fputs("Error: overflow (string table)\n", stderr), 1;

	/*
	 * The extra space for alignments and symbols.
	 */
	if (*size < INT_MAX - (add = 512))
		*size += add;
	else
		return fputs("Error: overflow (extra space)\n", stderr), 1;
	return 0;
}

int link_main(struct options *opt)
{
	unsigned char *out;
	int size;
	int off;

	if (!opt->arg_o)
		return 0;
	if (set_max_size(opt, &size))
		return 0;
	out = calloc((size_t)size, sizeof(unsigned char));
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;
	add_with_align((off = 0, &off), get_pre_size());

	/*
	 * Handle "duplicate" sections.
	 */
	{
		int i;
		int j;

		/*
		 * Replace special value 0xFF with 0x06 (label).
		 */
		for (i = 0; i < opt->nr_mfiles; i++) {
			unsigned char *dat = opt->mfiles[i].data;
			unsigned char *sym = dat + LE32(&dat[8]);
			int syms = (int)LE32(&dat[12]);

			for (j = 0; j < syms; j++) {
				if ((unsigned)sym[16] == 0xFFu)
					sym[16] = 6u;
				if ((unsigned)sym[17]) {
					j += (int)sym[17];
					sym += (int)sym[17] * 18;
				}
				sym += 18;
			}
		}
	}

	/*
	 * Set the header values.
	 */
	{
		unsigned long magic = LE16(opt->mfiles[0].data);
		W_LE16(&out[0], magic);
		W_LE16(&out[2], 4);
	}

	/*
	 * Section: .text
	 */
	{
		const char *name = ".text";
		unsigned char *section = out + 20 + 0 * 40;
		unsigned long flags = 0x60D00020ul;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes);
		nbytes = section_copy_r(opt, name, out + off);
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .rdata
	 */
	{
		const char *name = ".rdata";
		unsigned char *section = out + 20 + 1 * 40;
		unsigned long flags = 0x40D00040ul;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes);
		nbytes = section_copy_r(opt, name, out + off);
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .data
	 */
	{
		const char *name = ".data";
		unsigned char *section = out + 20 + 2 * 40;
		unsigned long flags = 0xC0D00040ul;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes);
		nbytes = section_copy_r(opt, name, out + off);
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .bss
	 */
	{
		const char *name = ".bss";
		unsigned char *section = out + 20 + 3 * 40;
		unsigned long flags = 0xC0D00080ul;

		strcpy((char *)(section + 0), name);
		(void)section_copy_d(opt, name, NULL);
		W_LE32(section + 16, section_sizeof_bss(opt));
		W_LE32(section + 36, flags);
	}

	/*
	 * Symbol table.
	 */
	{
		int t1 = symbol_copy_table(opt, out + off);
		int t2 = t1 / 18;

		W_LE32(out + 8, off);
		W_LE32(out + 12, t2);
		if (symbol_process(opt, out)) {
			free(out);
			return 1;
		}
		off += LE32(out + 12) * 18ul;
	}

	/*
	 * String table.
	 */
	{
		unsigned char *sym = out + LE32(&out[8]);
		unsigned char *str = out + off;
		int syms = (int)LE32(&out[12]);
		int strs = 4;
		int i;

		for (i = 0; i < syms; i++) {
			if (!LE32(&sym[0])) {
				int t1 = (int)LE16(&sym[4]);
				int t2 = (int)LE16(&sym[6]);
				unsigned char *t3 = opt->mfiles[t1].data;
				unsigned char *t4 = t3 + LE32(&t3[8]);
				unsigned char *t5 = t4 + LE32(&t3[12]) * 18ul;
				unsigned char *t6 = t4 + t2 * 18;
				unsigned char *t7 = t5 + LE32(&t6[4]);
				int t8 = (int)strlen((char *)t7) + 1;

				memcpy(&str[strs], t7, t8);
				W_LE32(&sym[4], strs);
				strs += t8;
			}
			if (sym[17]) {
				i += (int)sym[17];
				sym += (int)sym[17] * 18;
			}
			sym += 18;
		}
		W_LE32(str, strs);
		off += strs;
	}

	/*
	 * Adjust the size and write the output.
	 */
	if (off >= size - 32)
		fputs("Error: buffer overflow\n", stderr), exit(1);
	size = off;
	return end(opt, out, (size_t)size);
}
