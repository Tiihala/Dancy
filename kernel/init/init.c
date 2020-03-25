/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * init/init.c
 *      Initialization of Dancy Operating System
 */

#include <init.h>

void init(void)
{
	const uint32_t log_mem = 0x00080000;
	struct b_video_info vi;
	struct b_time bt;

	if (b_log_init(log_mem))
		return;

	memory_print_map(b_log);

	if (cpu_test_features())
		return;

	if (rtc_read(&bt)) {
		/*
		 * This should not be likely. However, the operating system
		 * does not stop because it must handle wrong time and date
		 * values anyway. If nothing is returned, it is just treated
		 * like other obviously wrong values. Print a warning.
		 */
		b_print("Warning: reading Real Time Clock (RTC) failed\n");
		b_pause();
	}

	if (b_get_structure(&vi, B_VIDEO_INFO) != 0 && vi.width != 0) {
		static const char *mode_names[8] = {
			"4 VGA",
			"8 PALETTE",
			"15",
			"16",
			"24 RGB",
			"24 BGR",
			"32 RGBX",
			"32 BGRX"
		};
		unsigned i = (unsigned)vi.mode;
		const char *mode_name = (i < 8) ? mode_names[i] : "";

		b_log("Video Mode\n");
		b_log("\tMode: %ux%ux%s\n", vi.width, vi.height, mode_name);
		b_log("\n");

		/*
		 * The boot loader guarantees that VGA registers can be
		 * accessed if mode is B_MODE_VGA or B_MODE_PALETTE.
		 */
		if (vi.mode == B_MODE_VGA || vi.mode == B_MODE_PALETTE)
			vga_set_palette();
	}

	/*
	 * Temporary code for testing purposes.
	 */
	{
		size_t i;

		for (i = 0; i < boot_log_size; i++) {
			if (!b_put_byte_com1((unsigned char)boot_log[i]))
				break;
		}

		for (i = 0; i < boot_log_size; i++) {
			if (!b_put_byte_com2((unsigned char)boot_log[i]))
				break;
		}

		b_output_string(boot_log, (unsigned)boot_log_size);
	}
}

DANCY_SYMBOL(init);
