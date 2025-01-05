/*
 * Copyright (c) 2025 Antti Tiihala
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
 * init/video/bochs.c
 *      Bochs virtual video hardware
 */

#include <boot/init.h>

#define INDEX_ID      0x00
#define INDEX_XRES    0x01
#define INDEX_YRES    0x02
#define INDEX_BPP     0x03
#define INDEX_ENABLE  0x04
#define INDEX_BANK    0x05
#define INDEX_VIRT_W  0x06
#define INDEX_VIRT_H  0x07
#define INDEX_X_OFF   0x08
#define INDEX_Y_OFF   0x09

static uint32_t read_register(int i)
{
	cpu_out16(0x01CE, (uint16_t)i);
	return cpu_in16(0x01CF);
}

static void write_register(int i, uint32_t v)
{
	cpu_out16(0x01CE, (uint16_t)i);
	cpu_out16(0x01CF, (uint16_t)v);
}

static void *read_vi(struct pci_device *pci, struct b_video_info *vi)
{
	uint32_t width = read_register(INDEX_XRES);
	uint32_t height = read_register(INDEX_YRES);
	uint32_t bpp = read_register(INDEX_BPP);
	uint32_t stride = width;
	uint32_t fb = pci_read(pci, 0x10);

	if ((fb & 7) != 0 || (fb &= 0xFFFFFFF0u) == 0)
		return NULL;

	switch (bpp) {
		case 8:
			vi->mode = B_MODE_PALETTE;
			break;
		case 15:
			vi->mode = B_MODE_15_BIT;
			stride *= 2;
			break;
		case 16:
			vi->mode = B_MODE_16_BIT;
			stride *= 2;
			break;
		case 24:
			vi->mode = B_MODE_BLUE_GREEN_RED;
			stride *= 3;
			break;
		case 32:
			vi->mode = B_MODE_BLUE_GREEN_RED_RESERVED;
			stride *= 4;
			break;
		default:
			return NULL;
	}

	vi->width = width;
	vi->height = height;
	vi->stride = stride;
	vi->framebuffer = (phys_addr_t)fb;

	return vi;
}

struct b_video_info *video_driver_init_bochs(struct pci_device *pci)
{
	static struct b_video_info vi;
	uint32_t id = read_register(INDEX_ID);

	if (id < 0xB0C4 || id > 0xB0CF)
		return NULL;

	if (read_vi(pci, &vi) == NULL)
		return NULL;

	if (gui_video_info->width != vi.width)
		return NULL;

	if (gui_video_info->height != vi.height)
		return NULL;

	write_register(INDEX_ENABLE, 0x00);
	write_register(INDEX_XRES, video_driver_request_width);
	write_register(INDEX_YRES, video_driver_request_height);
	write_register(INDEX_BPP, video_driver_request_bpp);
	write_register(INDEX_ENABLE, 0x41);

	read_vi(pci, &vi);

	return &vi;
}
