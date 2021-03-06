/*
 * Copyright (c) 2019, 2020, 2021 Antti Tiihala
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

#include <boot/init.h>

static uint32_t delay_low;
static uint32_t delay_high;

void vga_set_palette_early(const struct b_video_info *vi)
{
	uint8_t color = 0;
	uint8_t pic1, pic2;
	int i;

	/*
	 * The caller must make sure that writing VGA registers is safe
	 * and PICs are available like on "normal" BIOS computers.
	 */
	if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
		return;

	/*
	 * Clear the screen (B_MODE_PALETTE).
	 */
	if (vi->mode == B_MODE_PALETTE) {
		unsigned char *dst = (unsigned char *)vi->framebuffer;
		unsigned x, y;

		for (y = 0; y < vi->height; y++) {
			for (x = 0; x < vi->width; x++)
				dst[x] = 0;
			dst += vi->stride;
		}
	}

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
	 * Clear the screen (B_MODE_VGA).
	 */
	if (vi->mode == B_MODE_VGA) {
		unsigned char *dst = (unsigned char *)vi->framebuffer;
		const size_t size = 38400;

		cpu_out8(0x03C4, 0x02);
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C5, 0x0F);
		cpu_rdtsc_delay(delay_low, delay_high);

		/*
		 * Enable IRQs when clearing the screen
		 * and then disable them again.
		 */
		cpu_out8(0x21, pic1), cpu_out8(0xA1, pic2);
		memset(dst, 0, size);
		cpu_out8(0xA1, 0xFF), cpu_out8(0x21, 0xFF);
	}

	/*
	 * Use the correct mask.
	 */
	cpu_out8(0x03C6, 0xFF);
	cpu_rdtsc_delay(delay_low, delay_high);

	/*
	 * Generate the palette.
	 */
	for (i = 0; i < 256; i++) {
		uint8_t red   = (uint8_t)(color >> 2);
		uint8_t green = (uint8_t)(color >> 2);
		uint8_t blue  = (uint8_t)(color >> 2);

		/*
		 * Select the color index.
		 */
		cpu_out8(0x03C8, (uint8_t)i);
		cpu_rdtsc_delay(delay_low, delay_high);

		/*
		 * Write the color values.
		 */
		cpu_out8(0x03C9, red);
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, green);
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, blue);
		cpu_rdtsc_delay(delay_low, delay_high);

		if (i < 15)
			color = (uint8_t)(color + 0x10u);
		else
			color = (uint8_t)(i + 1);
	}

	/*
	 * Set the palette indices.
	 */
	for (i = -1; i < 16; i++) {
		uint8_t dat;

		if (i >= 0) {
			dat = cpu_in8(0x03C0);
			cpu_rdtsc_delay(delay_low, delay_high);

			cpu_out8(0x03C0, (uint8_t)i);
			cpu_rdtsc_delay(delay_low, delay_high);

			cpu_out8(0x03C0, (uint8_t)i);
			cpu_rdtsc_delay(delay_low, delay_high);

			cpu_out8(0x03C0, dat);
			cpu_rdtsc_delay(delay_low, delay_high);
		}
		cpu_in8(0x03DA);
		cpu_rdtsc_delay(delay_low, delay_high);
	}

	/*
	 * Enable IRQs.
	 */
	cpu_out8(0x21, pic1);
	cpu_out8(0xA1, pic2);
}

void vga_set_palette(void)
{
	int i, r;

	/*
	 * The caller must make sure that writing VGA registers is safe.
	 */
	r = cpu_ints(0);

	/*
	 * Generate the palette.
	 */
	for (i = 0; i < 256; i++) {
		uint32_t color = (uint32_t)i;
		uint8_t red, green, blue;

		switch (i % 4) {
		case 0:
			color = (color << 16) | (color << 8) | (color << 0);
			break;
		case 1:
			color = (color << 0);
			break;
		case 2:
			color = (color << 8);
			break;
		default:
			color = (color << 16);
			break;
		}

		red   = (uint8_t)((color >> 2)  & 63u);
		green = (uint8_t)((color >> 10) & 63u);
		blue  = (uint8_t)((color >> 18) & 63u);

		/*
		 * Select the color index.
		 */
		cpu_out8(0x03C8, (uint8_t)i);
		cpu_rdtsc_delay(delay_low, delay_high);

		/*
		 * Write the color values.
		 */
		cpu_out8(0x03C9, red);
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, green);
		cpu_rdtsc_delay(delay_low, delay_high);

		cpu_out8(0x03C9, blue);
		cpu_rdtsc_delay(delay_low, delay_high);
	}

	cpu_ints(r);
}
