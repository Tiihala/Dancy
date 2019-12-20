/*
 * Copyright (c) 2019 Antti Tiihala
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
 * init/vga.c
 *      Initialize VGA hardware
 */

#include <init.h>

#define VGA_COLORS 16

static const uint32_t vga_colors[VGA_COLORS] = {
	0x00000000, /* 0x00 */
	0x00AA0000, /* 0x01 */
	0x0000AA00, /* 0x02 */
	0x00AAAA00, /* 0x03 */
	0x000000AA, /* 0x04 */
	0x00AA00AA, /* 0x05 */
	0x000055AA, /* 0x06 */
	0x00AAAAAA, /* 0x07 */
	0x00555555, /* 0x08 */
	0x00FF5555, /* 0x09 */
	0x0055FF55, /* 0x0A */
	0x00FFFF55, /* 0x0B */
	0x005555FF, /* 0x0C */
	0x00FF55FF, /* 0x0D */
	0x0055FFFF, /* 0x0E */
	0x00FFFFFF  /* 0x0F */
};

void vga_set_palette(void)
{

	uint32_t delay_low, delay_high;
	uint8_t pic1, pic2;
	int i;

	/*
	 * The caller must make sure that writing VGA registers is safe
	 * and PICs are available like on "normal" BIOS computers.
	 */
	if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
		return;

	/*
	 * Disable IRQs.
	 */
	pic1 = cpu_in8(0x21);
	pic2 = cpu_in8(0xA1);
	cpu_out8(0xA1, 0xFF);
	cpu_out8(0x21, 0xFF);

	/*
	 * Calibrate IO delay.
	 */
	cpu_rdtsc(&delay_low, &delay_high);
	cpu_out8(0xA1, 0xFF), cpu_out8(0x21, 0xFF);
	cpu_rdtsc_diff(&delay_low, &delay_high);

	/*
	 * Use the correct mask.
	 */
	cpu_out8(0x03C6, 0xFF);
	cpu_rdtsc_delay(delay_low, delay_high);

	/*
	 * Generate the palette.
	 */
	for (i = 0; i < 256; i++) {
		uint32_t color = vga_colors[i % VGA_COLORS];
		int red   = (int)((color >> 2)  & 63u) - (i / VGA_COLORS * 3);
		int green = (int)((color >> 10) & 63u) - (i / VGA_COLORS * 3);
		int blue  = (int)((color >> 18) & 63u) - (i / VGA_COLORS * 3);

		/*
		 * Select the color index.
		 */
		cpu_out8(0x03C8, (uint8_t)i);
		cpu_rdtsc_delay(delay_low, delay_high);

		/*
		 * Write the color values.
		 */
		cpu_out8(0x03C9, (uint8_t)(red >= 0 ? red : 0));
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, (uint8_t)(green >= 0 ? green : 0));
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, (uint8_t)(blue >= 0 ? blue : 0));
		cpu_rdtsc_delay(delay_low, delay_high);
	}

	/*
	 * Enable IRQs.
	 */
	cpu_out8(0x21, pic1);
	cpu_out8(0xA1, pic2);
}
