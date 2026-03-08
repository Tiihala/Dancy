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
 * ld-dancy/main.h
 *      The executable loader
 */

#ifndef MAIN_CMDNAME
#define MAIN_CMDNAME "ld-dancy"

#include <__dancy/syscall.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct options {
	char **operands;
	const char *error;
	int debug;

	unsigned char *program;
	size_t program_size;
};

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;

#define EI_NIDENT 16

typedef struct {
	unsigned char e_ident[EI_NIDENT];

	Elf32_Half  e_type;
	Elf32_Half  e_machine;
	Elf32_Word  e_version;
	Elf32_Addr  e_entry;
	Elf32_Off   e_phoff;
	Elf32_Off   e_shoff;
	Elf32_Word  e_flags;
	Elf32_Half  e_ehsize;
	Elf32_Half  e_phentsize;
	Elf32_Half  e_phnum;
	Elf32_Half  e_shentsize;
	Elf32_Half  e_shnum;
	Elf32_Half  e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	unsigned char e_ident[EI_NIDENT];

	Elf64_Half  e_type;
	Elf64_Half  e_machine;
	Elf64_Word  e_version;
	Elf64_Addr  e_entry;
	Elf64_Off   e_phoff;
	Elf64_Off   e_shoff;
	Elf64_Word  e_flags;
	Elf64_Half  e_ehsize;
	Elf64_Half  e_phentsize;
	Elf64_Half  e_phnum;
	Elf64_Half  e_shentsize;
	Elf64_Half  e_shnum;
	Elf64_Half  e_shstrndx;
} Elf64_Ehdr;

typedef struct {
	Elf32_Word  p_type;
	Elf32_Off   p_offset;
	Elf32_Addr  p_vaddr;
	Elf32_Addr  p_paddr;
	Elf32_Word  p_filesz;
	Elf32_Word  p_memsz;
	Elf32_Word  p_flags;
	Elf32_Word  p_align;
} Elf32_Phdr;

typedef struct {
	Elf64_Word  p_type;
	Elf64_Word  p_flags;
	Elf64_Off   p_offset;
	Elf64_Addr  p_vaddr;
	Elf64_Addr  p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

int elf_execute(struct options *opt);
int operate(struct options *opt);

#else
#error "MAIN_CMDNAME"
#endif
