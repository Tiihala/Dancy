/*
 * Copyright (c) 2021 Antti Tiihala
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
 * init/kernel.c
 *      Kernel initialization
 */

#include <init.h>

#ifdef DANCY_32
static const char *kernel_at = "o32/kernel.at";
static const char *sym_start = "_kernel_start";
static const char *sym_start_ap = "_kernel_start_ap";
#endif

#ifdef DANCY_64
static const char *kernel_at = "o64/kernel.at";
static const char *sym_start = "kernel_start";
static const char *sym_start_ap = "kernel_start_ap";
#endif

static const size_t stack_size = 0x2000;

static void (*kernel_start_func)(void);
static void (*kernel_start_ap_func)(void);

void kernel_init(void)
{

}

void kernel_init_ap(uint32_t id)
{

}
