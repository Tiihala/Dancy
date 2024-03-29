/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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

static int write_obj(struct options *opt, unsigned char *out, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!opt->arg_o)
		return free(out), 0;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return free(out), 1;
		}
		is_stdout = 0;
	}

	if ((errno = 0, fwrite(out, 1, size, fp)) != size) {
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

static int match(unsigned char *objs[], unsigned char *syms[])
{
	unsigned char *o1 = objs[0];
	unsigned char *o2 = objs[1];
	unsigned char *s1 = syms[0];
	unsigned char *s2 = syms[1];
	unsigned char b1 = s1[8];
	unsigned char b2 = s2[8];
	int r;

	if (!*s1)
		s1 = o1 + LE32(&o1[8]) + (LE32(&o1[12]) * 18u) + LE32(&s1[4]);
	else
		s1[8] = 0u;
	if (!*s2)
		s2 = o2 + LE32(&o2[8]) + (LE32(&o2[12]) * 18u) + LE32(&s2[4]);
	else
		s2[8] = 0u;

	r = strcmp((const char *)s1, (const char *)s2);
	syms[0][8] = b1, syms[1][8] = b2;
	return r ? 0 : 1;
}

static int duplicate(unsigned char *obj, unsigned char *sym, int type)
{
	static void *type2 = NULL;
	static int type2_num = 0;
	struct { unsigned char *o; unsigned char *s; } *record = NULL;

	if (obj && type == 2) {
		const int chunk = 32;
		unsigned char *objs[2];
		unsigned char *syms[2];
		int i;

		for (i = 0; i < type2_num; i++) {
			record = type2;
			objs[0] = (record + i)->o, objs[1] = obj;
			syms[0] = (record + i)->s, syms[1] = sym;
			if (match(objs, syms))
				break;
			record = NULL;
		}
		if (record) {
			unsigned char *sec;
			sec = obj + 20 + ((LE16(&sym[12]) - 1) * 40);
			memcpy(&sec[0], "____SKIP", 8);
			return 0;
		}
		if ((type2_num % chunk) == 0) {
			size_t s1 = sizeof(record[0]);
			if (!type2) {
				size_t s2 = (size_t)chunk * s1;
				type2 = malloc(s2);
			} else {
				size_t s2 = (size_t)(type2_num + chunk) * s1;
				type2 = realloc(type2, s2);
			}
		}
		if (!type2) {
			fputs("Error: not enough memory\n", stderr);
			return 1;
		}
		record = type2;
		record[type2_num].o = obj;
		record[type2_num].s = sym;
		return type2_num++, 0;
	}
	if (obj)
		return 0;

	free(type2), type2 = NULL;
	type2_num = 0;

	return 0;
}

static int mangled(const char *name)
{
	char c;

	if (*name == '\0' || isdigit(*name))
		return 1;

	while ((c = *name++) != '\0') {
		if (c != '_' && !isalnum(c))
			return 1;
	}
	return 0;
}

static int get_pre_size(struct options *opt)
{
	/*
	 * The file header and section entries, i.e. text,
	 * rdata, data, and bss. Use the maximum value,
	 * assuming that all sections are available.
	 */
	int size = 20 + 4 * 40;

	if (!strcmp(opt->arg_f, "at"))
		size += AT_HEADER_SIZE;
	else if (!strcmp(opt->arg_f, "init"))
		size += IN_HEADER_SIZE;
	else if (!strcmp(opt->arg_f, "uefi"))
		size += EF_HEADER_SIZE;
	return size;
}

static void add_with_align(int *off, int add, int init)
{
	unsigned u = (init) ? 63 : 15;
	int t = *off;

	t = t + add;
	t = (int)(((unsigned)t + u) & ~u);
	*off = t;
}

static int set_max_size(struct options *opt, int *size)
{
	int add;

	/*
	 * The file header and section entries.
	 */
	*size = get_pre_size(opt);

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
	if (*size < INT_MAX - (add = 4096 + 1024)) {
		*size += add;
		*size = (int)((unsigned)(*size) & ~(4095u));
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
	if ((unsigned long)(*size) < 0x7FFFFFFF)
		return 0;
	return fputs("Error: overflow (file size)\n", stderr), 1;
}

static unsigned long crc32c(const void *obj, size_t len)
{
	const unsigned char *ptr = (const unsigned char *)obj;
	unsigned long crc = 0xFFFFFFFFul;
	int i;

	while (len--) {
		crc ^= (unsigned long)*ptr++;
		for (i = 0; i < CHAR_BIT; i++) {
			if (crc & 1ul)
				crc >>= 1, crc ^= 0x82F63B78ul;
			else
				crc >>= 1;
		}
	}
	return ~crc & 0xFFFFFFFFul;
}

static void set_at_header(unsigned char *buf, int size)
{
	static const unsigned char file_header[16] = {
		0x8D, 0x41, 0x54, 0x0D, 0x0A, 0x73, 0x74, 0x64,
		0x0C, 0x43, 0x0C, 0x45, 0x0C, 0x0A, 0x71, 0xF8
	};
	unsigned long crc;

	memset(&buf[0], 0, (size_t)AT_HEADER_SIZE);
	memcpy(&buf[0], &file_header[0], sizeof(file_header));
	W_LE32(&buf[16], size);

	crc = crc32c(buf, (size_t)size);
	W_LE32(&buf[24], crc);
}

int link_main(struct options *opt)
{
	int init = (strcmp(opt->arg_f, "init")) ? 0 : 1;
	unsigned char *out;
	int size;
	int off;

	/*
	 * The uefi format is like "init" when it comes to file alignment.
	 */
	if (!strcmp(opt->arg_f, "uefi"))
		init = 1;

	/*
	 * Handle "duplicate" sections and remove "mangled" names.
	 */
	{
		static const char *err = "Error: inconsistent symbol table\n";
		int i;
		int j;

		for (i = 0; i < opt->nr_ofiles; i++) {
			unsigned char *dat = opt->ofiles[i].data;
			unsigned char *sym = dat + LE32(&dat[8]);
			unsigned char *str = sym + (LE32(&dat[12]) * 18);
			int syms = (int)LE32(&dat[12]);
			int state = 0;

			for (j = 0; j < syms; j++) {
				unsigned sec = (unsigned)LE16(&sym[12]);
				unsigned type = (unsigned)sym[16];

				/*
				 * Symbols have a "type", typically 0x20 or
				 * 0x00 (function or unknown). The current
				 * implementation removes this information
				 * and uses the data field internally.
				 */
				sym[14] = 0, sym[15] = 0;

				/*
				 * The current implementation changes weak
				 * externals to "normal" externals.
				 */
				if (type == 105)
					sym[16] = (unsigned char)(type = 2);

				if (!state && type == 3u && !LE32(&sym[8])) {
					const char *n = (const char *)&sym[0];
					if (!strcmp(n, ".text"))
						state = 1;
					if (!strcmp(n, ".rdata"))
						state = 1;
					if (!strcmp(n, ".data"))
						state = 1;
					if (!sec)
						state = 0;
					if (sym[17] && !sym[18 + 14])
						state = 0;
					if (state) {
						unsigned char *t1 = dat + 20;
						t1 += ((int)sec - 1) * 40;
						if (memcmp(t1, n, 8))
							state = 0;
						if (!(t1[37] & 0x10u))
							state = 0;
					}
					state = state ? (int)sym[17] : 0;
				} else if (state) {
					unsigned char *t1 = sym - state * 18;
					unsigned char *t2 = t1 - 18;
					int t3 = (int)(*(t1 + 14));

					if (LE16(&sym[12]) != LE16(&t2[12])) {
						fputs(err, stderr);
						return 1;
					}
					if (duplicate(dat, sym, t3))
						return 1;
					state = 0;
				}

				/*
				 * The special type 0xD0 means that the symbol
				 * is external but the name will be discarded.
				 */
				if (sec && type == 2) {
					unsigned char *name = sym;
					unsigned char b;

					if (!*sym)
						name = str + LE32(&sym[4]);
					b = sym[8], sym[8] = 0;
					if (mangled((const char *)name))
						sym[16] = 0xD0;
					sym[8] = b;
				}

				if ((unsigned)sym[17]) {
					j += (int)sym[17];
					sym += (int)sym[17] * 18;
				}
				sym += 18;
			}
		}
		(void)duplicate(NULL, NULL, 0);
	}

	/*
	 * Allocate the buffer.
	 */
	if (set_max_size(opt, &size))
		return 0;
	out = calloc((size_t)size, sizeof(unsigned char));
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;
	add_with_align((off = 0, &off), get_pre_size(opt), init);

	/*
	 * Set the header values.
	 */
	{
		unsigned long magic = LE16(opt->ofiles[0].data);
		int i;

		for (i = 1; !magic && i < opt->nr_ofiles; i++)
			magic = LE16(opt->ofiles[i].data);

		W_LE16(&out[0], magic);
		W_LE16(&out[2], 4);
	}

	/*
	 * Section: .text
	 */
	{
		const char *name = ".text";
		unsigned char *section = out + 20 + 0 * 40;
		unsigned long flags = 0x60000020ul | opt->alignbits_t;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes, 0);
		nbytes = section_copy_r(opt, name, out + off);
		if ((unsigned)nbytes / 10u > 0xFFFE) {
			fputs("Error: too many .text relocations\n", stderr);
			return free(out), 1;
		}
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes, init);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .rdata
	 */
	{
		const char *name = ".rdata";
		unsigned char *section = out + 20 + 1 * 40;
		unsigned long flags = 0x40000040ul | opt->alignbits_r;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes, 0);
		nbytes = section_copy_r(opt, name, out + off);
		if ((unsigned)nbytes / 10u > 0xFFFE) {
			fputs("Error: too many .rdata relocations\n", stderr);
			return free(out), 1;
		}
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes, init);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .data
	 */
	{
		const char *name = ".data";
		unsigned char *section = out + 20 + 2 * 40;
		unsigned long flags = 0xC0000040ul | opt->alignbits_d;
		int nbytes;

		strcpy((char *)(section + 0), name);
		nbytes = section_copy_d(opt, name, out + off);
		W_LE32(section + 16, nbytes);
		W_LE32(section + 20, nbytes ? off : 0);
		add_with_align(&off, nbytes, 0);
		nbytes = section_copy_r(opt, name, out + off);
		if ((unsigned)nbytes / 10u > 0xFFFE) {
			fputs("Error: too many .data relocations\n", stderr);
			return free(out), 1;
		}
		W_LE32(section + 24, nbytes ? off : 0);
		W_LE16(section + 32, nbytes / 10);
		add_with_align(&off, nbytes, init);
		W_LE32(section + 36, flags);
	}

	/*
	 * Section: .bss
	 */
	{
		const char *name = ".bss";
		unsigned char *section = out + 20 + 3 * 40;
		unsigned long flags = 0xC0000080ul | opt->alignbits_b;
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
		off += (int)LE32(out + 12) * 18;
	}

	/*
	 * Check the memory layout.
	 */
	{
		unsigned char *section = out + 20;
		unsigned long total_size = 0;
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
		if (!strcmp(opt->arg_f, "init")) {
			/*
			 * section .text  align 64
			 * section .data  align 64
			 * section .rdata align 64
			 * section .bss   align 64
			 */
			if (opt->alignbits_t > 0x00700000) {
				fputs("Warning: init .text align\n", stderr);
			}
			if (opt->alignbits_r > 0x00700000) {
				fputs("Error: init .rdata align\n", stderr);
				return free(out), 1;
			}
			if (opt->alignbits_d > 0x00700000) {
				fputs("Error: init .data align\n", stderr);
				return free(out), 1;
			}
			if (opt->alignbits_b > 0x00700000) {
				fputs("Error: init .bss align\n", stderr);
				return free(out), 1;
			}
			if (!(total_size < 0x0000E000))
				err = 1;
		}
		if (!strcmp(opt->arg_f, "uefi")) {
			/*
			 * section .text  align 64
			 * section .data  align 64
			 * section .rdata align 64
			 */
			if (opt->alignbits_t > 0x00700000) {
				fputs("Warning: uefi .text align\n", stderr);
			}
			if (opt->alignbits_r > 0x00700000) {
				fputs("Error: uefi .rdata align\n", stderr);
				return free(out), 1;
			}
			if (opt->alignbits_d > 0x00700000) {
				fputs("Error: uefi .data align\n", stderr);
				return free(out), 1;
			}
		}
		if (err || !(total_size < 0x7FFFFFFF)) {
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
				unsigned char *t3 = opt->ofiles[t1].data;
				unsigned char *t4 = t3 + LE32(&t3[8]);
				unsigned char *t5 = t4 + LE32(&t3[12]) * 18;
				unsigned char *t6 = t4 + t2 * 18;
				unsigned char *t7 = t5 + LE32(&t6[4]);
				int t8 = (int)strlen((char *)t7) + 1;

				memcpy(&str[strs], t7, (size_t)t8);
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
	 * Adjust the size.
	 */
	if (off >= size - 128)
		fputs("Error: buffer overflow\n", stderr), exit(1);
	size = off;

	/*
	 * Handle custom headers.
	 */
	if (!strcmp(opt->arg_f, "at")) {
		size_t s = (size_t)AT_HEADER_SIZE;
		memmove(&out[s], &out[0], (20 + 4 * 40));
		set_at_header(&out[0], size);
	} else if (!strcmp(opt->arg_f, "init")) {
		size_t s = (size_t)IN_HEADER_SIZE;
		memmove(&out[s], &out[0], (20 + 4 * 40));
		memset(&out[0], 0, s);
		if ((unsigned)size > 0xF000) {
			fputs("Error: init file overflow\n", stderr);
			return free(out), 1;
		}
	} else if (!strcmp(opt->arg_f, "uefi")) {
		size_t s = (size_t)EF_HEADER_SIZE;
		memmove(&out[s], &out[0], (20 + 4 * 40));
		memset(&out[0], 0, s);
	}

	/*
	 * Handle the library mode.
	 */
	if (opt->lib_mode) {
		opt->lib_ofiles[opt->nr_lib_ofiles].name = "object";
		opt->lib_ofiles[opt->nr_lib_ofiles].data = out;
		opt->lib_ofiles[opt->nr_lib_ofiles].size = size;
		opt->lib_ofiles[opt->nr_lib_ofiles].type = 1;
		opt->nr_lib_ofiles += 1;
		return 0;
	}

	/*
	 * Write the output.
	 */
	return write_obj(opt, out, (size_t)size);
}
