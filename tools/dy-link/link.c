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

static int duplicate(const char *name, int obj, int sym, int type)
{
	if (type == INT_MAX) {
		fputs("Error: inconsistent symbol table", stderr);
		return 1;
	}
	if (name)
		printf("duplicate: %i %i %i %s\n", type, sym, obj, name);
	return 0;
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
	 * Section .text and relocation.
	 */
	if (*size < INT_MAX - (add = section_data_size(opt, ".text")))
		*size += add;
	else
		return fputs("Error: overflow (.text)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_reloc_size(opt, ".text")))
		*size += add;
	else
		return fputs("Error: overflow (.text reloc)\n", stderr), 1;

	/*
	 * Section .rdata and relocation.
	 */
	if (*size < INT_MAX - (add = section_data_size(opt, ".rdata")))
		*size += add;
	else
		return fputs("Error: overflow (.rdata)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_reloc_size(opt, ".rdata")))
		*size += add;
	else
		return fputs("Error: overflow (.rdata reloc)\n", stderr), 1;

	/*
	 * Section .data and relocation.
	 */
	if (*size < INT_MAX - (add = section_data_size(opt, ".data")))
		*size += add;
	else
		return fputs("Error: overflow (.data)\n", stderr), 1;

	if (*size < INT_MAX - (add = section_reloc_size(opt, ".data")))
		*size += add;
	else
		return fputs("Error: overflow (.data reloc)\n", stderr), 1;

	/*
	 * The symbol table.
	 */
	if (*size < INT_MAX - (add = symbol_table_size(opt)))
		*size += add;
	else
		return fputs("Error: overflow (symbol table)\n", stderr), 1;

	/*
	 * The string table.
	 */
	if (*size < INT_MAX - (add = symbol_string_size(opt)))
		*size += add;
	else
		return fputs("Error: overflow (string table)\n", stderr), 1;

	/*
	 * The extra space for alignments and symbols.
	 */
	if (*size < INT_MAX - (add = 512)) {
		*size += add;
		*size = (int)((unsigned)(*size) & ~(31u));
	} else {
		return fputs("Error: overflow (extra space)\n", stderr), 1;
	}

	/*
	 * Check the total file size. If sizeof(int) were larger
	 * than 4 bytes, this error could happen. The object file
	 * format does not support very large objects so this is
	 * the simplest way to make sure that all values fit into
	 * the variables.
	 */
	if ((unsigned long)(*size) < 0x7FFFFFFFul)
		return 0;
	return fputs("Error: overflow (file size)\n", stderr), 1;
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

		for (i = 0; i < opt->nr_mfiles; i++) {
			unsigned char *dat = opt->mfiles[i].data;
			unsigned char *sym = dat + LE32(&dat[8]);
			const char *str = (char *)sym + LE32(&dat[12]) * 18ul;
			int syms = (int)LE32(&dat[12]);
			int state = 0;

			for (j = 0; j < syms; j++) {
				unsigned type = (unsigned)sym[16];

				if (type == 0xFFu)
					sym[16] = 6u;

				if (!state && type == 3u && !LE32(&sym[8])) {
					const char *n = (const char *)&sym[0];
					int sec = (int)LE16(&sym[12]);
					if (!strcmp(n, ".rdata"))
						state = 1;
					if (!strcmp(n, ".text"))
						state = 1;
					if (!sec)
						state = 0;
					if (sym[17] && !sym[18 + 14])
						state = 0;
					if (state) {
						unsigned char *t1 = dat + 20;
						t1 += (sec - 1) * 40;
						if (memcmp(t1, n, 8u))
							state = 0;
						if (!(t1[37] & 0x10u))
							state = 0;
					}
					state = state ? (int)sym[17] : 0;
				} else if (state) {
					unsigned char *t1 = sym - state * 18;
					unsigned char *t2 = t1 - 18;
					int t3 = (int)(*(t1 + 14));
					const char *n;
					char buf[9];

					if (LE32(&sym[0])) {
						memcpy(&buf[0], sym, 8u);
						buf[8] = '\0';
						n = &buf[0];
					} else {
						n = str + LE32(&sym[4]);
					}
					if (LE16(&sym[12]) != LE16(&t2[12]))
						t3 = INT_MAX;
					if (duplicate(n, i, j, t3))
						return free(out), 1;
					state = 0;
				}

				if ((unsigned)sym[17]) {
					j += (int)sym[17];
					sym += (int)sym[17] * 18;
				}
				sym += 18;
			}
		}
		if (duplicate(NULL, 0, 0, 0))
			return free(out), 1;
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
		if ((unsigned)nbytes / 10u > 0xFFFEu) {
			fputs("Error: too many .text relocations\n", stderr);
			return free(out), 1;
		}
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
		if ((unsigned)nbytes / 10u > 0xFFFEu) {
			fputs("Error: too many .rdata relocations\n", stderr);
			return free(out), 1;
		}
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
		if ((unsigned)nbytes / 10u > 0xFFFEu) {
			fputs("Error: too many .data relocations\n", stderr);
			return free(out), 1;
		}
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
		int nbytes = section_data_size(opt, ".bss");

		strcpy((char *)(section + 0), name);
		(void)section_copy_d(opt, name, NULL);
		W_LE32(section + 16, nbytes);
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
	 * Check the memory layout.
	 */
	{
		unsigned char *section = out + 20;
		unsigned long total_size = 0ul;
		int err = 0;
		int i;

		for (i = 0; i < 4; i++, section += 40) {
			unsigned long add = LE32(&section[16]);
			if (add < ULONG_MAX - 4095ul) {
				add += 4095ul;
				add &= ~(4095ul);
				if (total_size < ULONG_MAX - add) {
					total_size += add;
					continue;
				}
			}
			err = 1;
			break;
		}
		if (err || !(total_size < 0x7FFFFFFFul)) {
			const char *e = "Error: memory layout, %08lX bytes\n";
			fprintf(stderr, e, total_size);
			return free(out), 1;
		}
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

	if (opt->verbose) {
		printf("Output buffer, %i bytes\n", size);
		printf("Output file, %i bytes\n", off);
	}

	/*
	 * Adjust the size and write the output.
	 */
	if (off >= size - 128)
		fputs("Error: buffer overflow\n", stderr), exit(1);
	size = off;
	return end(opt, out, (size_t)size);
}
