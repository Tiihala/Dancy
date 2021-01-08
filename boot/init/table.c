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
 * init/table.c
 *      Kernel table initialization
 */

#include <init.h>

static size_t heap_size = 0;
static size_t heap_used = 0;

static void *table_alloc_pages(size_t count)
{
	size_t size = count * 0x1000;
	addr_t addr;

	heap_used = (heap_used + 0x0FFFu) & 0xFFFFF000u;

	if (count > 0x8000 || (heap_used + size) > heap_size)
		panic("table_alloc_pages: out of memory");

	addr = kernel->heap_addr + (addr_t)heap_used;
	heap_used += size;

	return (void *)addr;
}

static void *table_malloc(size_t size)
{
	size_t aligned_size = (size + 0x0Fu) & 0xFFFFFFF0u;
	addr_t addr;

	heap_used = (heap_used + 0x0Fu) & 0xFFFFFFF0u;

	if (size > 0x10000 || (heap_used + aligned_size) > heap_size)
		panic("table_malloc: out of memory");

	addr = kernel->heap_addr + (addr_t)heap_used;
	heap_used += aligned_size;

	return (void *)addr;
}

void table_init(void)
{
	addr_t addr;
	size_t size, i;

	/*
	 * Initialize the local memory manager.
	 */
	heap_size = (size_t)(kernel->stack_array_addr - kernel->heap_addr);

	if (heap_size == 0 || (heap_size & 0x0FFFu) != 0)
		panic("table_init: unexpected behavior");

	/*
	 * Write the function pointers.
	 */
	kernel->panic = panic;
	kernel->print = gui_print;

	/*
	 * Write the framebuffer variables.
	 */
	if (gui_video_info != NULL) {
		const struct b_video_info *vi = gui_video_info;

		kernel->fb_mode   = vi->mode;
		kernel->fb_width  = vi->width;
		kernel->fb_height = vi->height;
		kernel->fb_stride = vi->stride;
		kernel->fb_addr   = vi->framebuffer;

		/*
		 * The standard framebuffer has 4-byte pixels.
		 */
		size = (size_t)(vi->width * vi->height * sizeof(uint32_t));
		size = (size + 0x0FFFu) & 0xFFFFF000u;
		addr = (addr_t)table_alloc_pages(size / 0x1000);

		kernel->fb_standard_addr = addr;
		kernel->fb_standard_size = size;
	}

	/*
	 * Write the SMP variables. Allocate the zero-length memory area
	 * even if there were no application processors.
	 */
	{
		kernel->smp_ap_count = smp_ap_count;

		size = (size_t)(smp_ap_count * sizeof(uint32_t));
		kernel->smp_ap_id = table_malloc(size);

		for (i = 0; i < smp_ap_count; i++)
			kernel->smp_ap_id[i] = smp_ap_id[i];
	}

	/*
	 * Update the heap_addr variable.
	 */
	heap_used = (heap_used + 0x0FFFu) & 0xFFFFF000u;
	kernel->heap_addr += (addr_t)heap_used;

	if (kernel->heap_addr > kernel->stack_array_addr)
		panic("table_init: unexpected behavior");
}
