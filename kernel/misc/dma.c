/*
 * Copyright (c) 2022 Antti Tiihala
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
 * misc/dma.c
 *      Direct Memory Access (DMA) Controller
 */

#include <dancy.h>

static mtx_t dma_mtx;
static int dma_reset_state;
static phys_addr_t dma_buffer[8];

int dma_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&dma_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	return 0;
}

static void alloc_dma_buffer(int dma)
{
	dma_buffer[dma] = mm_alloc_pages(mm_legacy, 4);

	if (dma_buffer[dma] > 0xFFFFFF || (dma_buffer[dma] & 0xFFFF) != 0)
		kernel->panic("DMA: unexpected address");

	if (dma_buffer[dma])
		memset((void *)dma_buffer[dma], 0, 0x10000);
}

static phys_addr_t set_dma_from_0_to_3(int dma, size_t size, int mode)
{
	phys_addr_t buffer;

	if (dma_buffer[dma] == 0)
		alloc_dma_buffer(dma);

	if ((buffer = dma_buffer[dma]) != 0) {
		int a0 = (int)(buffer & 0xFF);
		int a1 = (int)((buffer >> 8) & 0xFF);
		int a2 = (int)((buffer >> 16) & 0xFF);
		int s0 = (int)((size - 1) & 0xFF);
		int s1 = (int)(((size - 1) >> 8) & 0xFF);

		/*
		 * Mask the channel.
		 */
		cpu_out8(0x0A, (uint8_t)(dma | 4));

		/*
		 * Set the address register.
		 */
		cpu_out8(0x0C, 0x00);
		cpu_out8((uint16_t)(dma * 2), (uint8_t)a0);
		cpu_out8((uint16_t)(dma * 2), (uint8_t)a1);

		switch (dma) {
			case 0: cpu_out8(0x87, (uint8_t)a2); break;
			case 1: cpu_out8(0x83, (uint8_t)a2); break;
			case 2: cpu_out8(0x81, (uint8_t)a2); break;
			case 3: cpu_out8(0x82, (uint8_t)a2); break;
			default: break;
		}

		/*
		 * Set the count register.
		 */
		cpu_out8(0x0C, 0x00);
		cpu_out8((uint16_t)((dma * 2) + 1), (uint8_t)s0);
		cpu_out8((uint16_t)((dma * 2) + 1), (uint8_t)s1);

		/*
		 * Set the mode register.
		 */
		cpu_out8(0x0B, (uint8_t)mode);

		/*
		 * Unmask the channel.
		 */
		cpu_out8(0x0A, (uint8_t)dma);
	}

	return buffer;
}

static phys_addr_t set_dma_from_4_to_7(int dma, size_t size, int mode)
{
	phys_addr_t buffer;

	/*
	 * This function has not been implemented.
	 */
	buffer = dma_buffer[dma];
	(void)size;
	(void)mode;

	return buffer;
}

static phys_addr_t set_dma(int dma, size_t size, int mode)
{
	phys_addr_t buffer = 0;

	if (!size || size > 0x10000)
		return 0;

	if (mtx_lock(&dma_mtx) != thrd_success)
		return 0;

	if (!dma_reset_state) {
		/*
		 * Reset the controller for channels 0-3.
		 */
		cpu_out8(0x0D, 0x00);

		/*
		 * Reset the controller for channels 4-7.
		 */
		cpu_out8(0xDA, 0x00);

		dma_reset_state = 1;
	}

	if (dma >= 0 && dma <= 3)
		buffer = set_dma_from_0_to_3(dma, size, mode);

	if (dma >= 4 && dma <= 7)
		buffer = set_dma_from_4_to_7(dma, size, mode);

	mtx_unlock(&dma_mtx);

	return buffer;
}

phys_addr_t dma_set_floppy_read(size_t size)
{
	return set_dma(2, size, 0x46);
}

phys_addr_t dma_set_floppy_write(size_t size)
{
	return set_dma(2, size, 0x4A);
}
