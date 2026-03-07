/*
 * Copyright (c) 2026 Antti Tiihala
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
 * ld-dancy/elf.c
 *      The executable loader
 */

#include "main.h"

static int elf_error(struct options *opt, const char *msg)
{
	fprintf(stderr, "ld-dancy: %s: %s\n", opt->operands[0], msg);
	return EXIT_FAILURE;
}

int elf_execute(struct options *opt)
{
	unsigned char *program = opt->program;
	size_t program_size = opt->program_size;

#if defined(__DANCY_32)
	Elf32_Ehdr *ehdr;
	const unsigned char ei_class = 1;
	const char *ei_class_error = "Only 32-bit supported (EI_CLASS)";
	const char *file_format = "elf32-i386";
#elif defined(__DANCY_64)
	Elf64_Ehdr *ehdr;
	const unsigned char ei_class = 2;
	const char *ei_class_error = "Only 64-bit supported (EI_CLASS)";
	const char *file_format = "elf64-x86-64";
#else
#error "__DANCY_32 or __DANCY_64 must be defined"
#endif

	if (program_size < sizeof(*ehdr))
		return elf_error(opt, "Exec header not supported");

	ehdr = (void *)program;

	if (ehdr->e_ident[4] != ei_class)
		return elf_error(opt, ei_class_error);

	if (ehdr->e_ident[5] != 1)
		return elf_error(opt, "Data encoding not supported");

	if (ehdr->e_ident[6] != 1 || ehdr->e_version != 1)
		return elf_error(opt, "File version not supported");

	if (ehdr->e_ident[7] != 0 || ehdr->e_ident[8] != 0)
		return elf_error(opt, "ABI ident/version not supported");

	if (ehdr->e_type != 2)
		return elf_error(opt, "Only executables supported (ET_EXEC)");

	if (opt->debug) {
		printf("\n%s: file format %s\n\n",
			opt->operands[0], file_format);
		printf("flags 0x%08X\n", (unsigned int)ehdr->e_flags);
		printf("start address 0x%p\n\n", (void *)ehdr->e_entry);
	}

	return 0;
}
