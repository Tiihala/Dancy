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
	DANCY_PALETTE_0,
	DANCY_PALETTE_1,
	DANCY_PALETTE_2,
	DANCY_PALETTE_3,
	DANCY_PALETTE_4,
	DANCY_PALETTE_5,
	DANCY_PALETTE_6,
	DANCY_PALETTE_7,
	DANCY_PALETTE_8,
	DANCY_PALETTE_9,
	DANCY_PALETTE_A,
	DANCY_PALETTE_B,
	DANCY_PALETTE_C,
	DANCY_PALETTE_D,
	DANCY_PALETTE_E,
	DANCY_PALETTE_F
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
		int red   = (int)((color >> 2)  & 63u);
		int green = (int)((color >> 10) & 63u);
		int blue  = (int)((color >> 18) & 63u);

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
