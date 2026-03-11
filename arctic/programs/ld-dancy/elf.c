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

extern char **environ;

static int elf_map_pages(void *addr, size_t size)
{
	int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED;

	void *r = mmap(addr, size, prot, flags | __DANCY_MAP_ARCTIC, -1, 0);

	return (r != addr) ? EXIT_FAILURE : 0;
}

static void elf_start(void *addr, char *const argv[], char *const envp[])
{
	long long r;

	r = __dancy_syscall4(__dancy_syscall_arctic, addr, argv, envp, 0);

	if (r < 0)
		errno = -((int)r);
	else
		errno = ENOSYS;
}

static int elf_error(struct options *opt, const char *msg)
{
	fprintf(stderr, "ld-dancy: %s: %s\n", opt->operands[0], msg);
	return EXIT_FAILURE;
}

static int elf_ph_load(struct options *opt, void *entry)
{
	int r = 0, prot = 0;

#if defined(__DANCY_32)
	Elf32_Phdr *p = entry;
#elif defined(__DANCY_64)
	Elf64_Phdr *p = entry;
#else
#error "__DANCY_32 or __DANCY_64 must be defined"
#endif

	if (p->p_type != 1 || p->p_memsz == 0)
		return 0;

	r |= (p->p_offset > opt->program_size);
	r |= (p->p_vaddr < 0x20000000);
	r |= (p->p_vaddr > 0x40000000);

	r |= (p->p_memsz > 0x20000000);
	r |= (p->p_filesz > p->p_memsz);
	r |= (p->p_offset + p->p_filesz > opt->program_size);

	if (r != 0)
		return elf_error(opt, "Program header not supported");

	if (opt->debug) {
		printf("  LOAD off    0x%08X"
			" vaddr 0x%08X paddr 0x%08X align %u\n",
			(unsigned int)p->p_offset,
			(unsigned int)p->p_vaddr, (unsigned int)p->p_paddr,
			(unsigned int)p->p_align);

		printf("       filesz 0x%08X memsz 0x%08X flags %c%c%c\n",
			(unsigned int)p->p_filesz, (unsigned int)p->p_memsz,
			(p->p_flags & 4) ? 'r' : '-',
			(p->p_flags & 2) ? 'w' : '-',
			(p->p_flags & 1) ? 'x' : '-');
	}

	if (elf_map_pages((void *)p->p_vaddr, (size_t)p->p_memsz)) {
		fprintf(stderr, "ld-dancy: %s: mmap: %s\n",
			opt->operands[0], strerror(errno));
		return EXIT_FAILURE;
	}

	if (p->p_filesz != 0) {
		void *d = (void *)p->p_vaddr;
		const void *s = &opt->program[p->p_offset];
		memcpy(d, s, (size_t)p->p_filesz);
	}

	if ((p->p_flags & 4) != 0)
		prot |= PROT_READ;

	if ((p->p_flags & 2) != 0)
		prot |= PROT_WRITE;

	if ((p->p_flags & 1) != 0)
		prot |= PROT_EXEC;

	(void)mprotect((void *)p->p_vaddr, (size_t)p->p_memsz, prot);

	return 0;
}

int elf_execute(struct options *opt)
{
	unsigned char *program = opt->program;
	size_t i, program_size = opt->program_size;
	int r = 0;

#if defined(__DANCY_32)
	Elf32_Ehdr *ehdr;
	Elf32_Phdr *phdr;
	const unsigned char ei_class = 1;
	const char *ei_class_error = "Only 32-bit supported (EI_CLASS)";
	const char *file_format = "elf32-i386";
#elif defined(__DANCY_64)
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	const unsigned char ei_class = 2;
	const char *ei_class_error = "Only 64-bit supported (EI_CLASS)";
	const char *file_format = "elf64-x86-64";
#else
#error "__DANCY_32 or __DANCY_64 must be defined"
#endif

	if (program_size < sizeof(*ehdr))
		return elf_error(opt, "Exec header not supported");

	if (program_size > 0x20000000)
		return elf_error(opt, "Program size not supported");

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

	if (ehdr->e_entry < 0x20000000 || ehdr->e_entry > 0x40000000)
		return elf_error(opt, "Start address not supported");

	if (ehdr->e_phentsize < sizeof(*phdr) || ehdr->e_phentsize > 0x1000)
		return elf_error(opt, "Program entries not supported");

	if (ehdr->e_phoff > 0x20000000 || ehdr->e_phnum > 0x1000)
		return elf_error(opt, "Program header offset not supported");

	if (opt->debug) {
		printf("\n%s: file format %s\n\n",
			opt->operands[0], file_format);
		printf("start address 0x%08X\n\n",
			(unsigned int)ehdr->e_entry);
		printf("Program Header:\n");
	}

	for (i = 0; i < ehdr->e_phnum; i++) {
		size_t offset = (size_t)ehdr->e_phoff;
		offset += (i * (size_t)ehdr->e_phentsize);

		if (offset + (size_t)ehdr->e_phentsize > program_size)
			return elf_error(opt, "Program header overflow");
	}

	for (i = 0; i < ehdr->e_phnum; i++) {
		size_t offset = (size_t)ehdr->e_phoff;
		offset += (i * (size_t)ehdr->e_phentsize);

		if ((r = elf_ph_load(opt, &program[offset])) != 0)
			break;
	}

	if (opt->debug)
		printf("\n");

	if (r != 0)
		return EXIT_FAILURE;

	elf_start((void *)ehdr->e_entry, &opt->operands[0], environ);

	fprintf(stderr, "ld-dancy: %s: start: %s\n",
		opt->operands[0], strerror(errno));

	return EXIT_FAILURE;
}
