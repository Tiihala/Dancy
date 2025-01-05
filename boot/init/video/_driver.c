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
 * init/video/_driver.c
 *      Initialize video hardware
 */

#include <boot/init.h>

uint16_t video_driver_request_width;
uint16_t video_driver_request_height;
uint16_t video_driver_request_bpp;

extern struct b_video_info *video_driver_init_bochs(struct pci_device *);

struct b_video_info *video_driver_init(void)
{
	uint32_t i;

	if (gui_video_info == NULL)
		return NULL;

	if (video_driver_request_width < 640)
		return NULL;
	if (video_driver_request_width > 3840)
		return NULL;

	if (video_driver_request_height < 480)
		return NULL;
	if (video_driver_request_height > 2160)
		return NULL;

	switch (video_driver_request_bpp) {
		case 15: break; case 16: break;
		case 24: break; case 32: break;
		default: return NULL;
	}

	for (i = 0; i < pci_device_count; i++) {
		struct pci_device *pci = &pci_devices[i];
		struct b_video_info *vi;

		if ((pci->class_code & 0xFF0000) != 0x030000)
			continue;

		if (pci->vendor_id == 0x80EE && pci->device_id == 0xBEEF) {
			if ((vi = video_driver_init_bochs(pci)) != NULL)
				return vi;
		}
	}

	return NULL;
}
