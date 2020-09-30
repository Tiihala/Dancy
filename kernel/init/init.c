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

void *ttf;

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
			vga_set_palette(&vi);
	}

	/*
	 * Load dancy.ttf and create a ttf object.
	 */
	{
		static const char *ttf_name = "data/fonts/dancy.ttf";
		unsigned char *ttf_data;
		size_t ttf_size;

		int r = db_read(ttf_name, &ttf_data, &ttf_size);

		if (r != 0) {
			b_print("%s error: %s\n", ttf_name, db_read_error(r));
			return;
		}

		if (ttf_create(&ttf)) {
			b_print("Error: ttf_create\n");
			return;
		}

		if (ttf_open(ttf, ttf_size, ttf_data)) {
			b_print("Error: ttf_open (%s)\n", ttf_name);
			return;
		}

		free(ttf_data);
	}

	/*
	 * Modify the boot file system.
	 */
	if (!fs_init()) {
		if (!fs_update_config_at(&vi))
			fs_write_logs();
		fs_free();
	} else {
		b_print("Error: fs_init\n");
		b_pause();
	}

	b_log_free();

	/*
	 * Initialize graphical user interface while boot services
	 * are still available. The actual GUI does not require them.
	 */
	if (gui_init()) {
		b_print("*****************************************\n");
		b_print("**** Graphical Mode is not supported ****\n");
		b_print("*****************************************\n\n");
	}

	/*
	 * Temporary code for testing purposes.
	 */
	{
		int x1, y1, x2, y2;
		size_t i;

		x1 = 40;
		y1 = 10;
		x2 = (int)(vi.width - vi.width / 3);
		y2 = (int)(vi.height - 10);

		gui_create_window("Memory Map", x1, y1, x2, y2);
		memory_print_map(gui_print);

		x1 = x1 + (int)(vi.width / 3);
		y1 = y1 + 40;
		x2 = x1 + 320;
		y2 = y1 + 200;

		gui_create_window("Dancy Operating System", x1, y1, x2, y2);
		gui_print("https://github.com/Tiihala/Dancy\n\n");

		for (i = 0; /* void */; i++) {
			unsigned second = UINT_MAX;
			int mov = (int)vi.height / 2 - 60;

			for (;;) {
				rtc_read(&bt);

				gui_print("\r%04u-%02u-%02u %02u:%02u:%02u",
					bt.year, bt.month, bt.day,
					bt.hour, bt.minute, bt.second);

				if (second != bt.second) {
					second = bt.second;
					gui_refresh();
				}

				if ((bt.second % 10) == 9) {
					while ((bt.second % 10) == 9)
						rtc_read(&bt);
					break;
				}
			}

			if ((i % 2) == 0)
				y1 += mov;
			else
				y1 -= mov;

			gui_move_window(x1, y1);
		}
	}
}

DANCY_SYMBOL(init);
