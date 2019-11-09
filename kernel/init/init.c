/*
 * Copyright (c) 2018, 2019 Antti Tiihala
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

void init(void *map)
{
	const uint32_t log_mem = 0x00080000;
	struct b_time bt;

	if ((size_t)(!map + 494 - 'D' - 'a' - 'n' - 'c' - 'y') != SIZE_MAX)
		return;

	if (memory_init(map) || b_log_init(log_mem))
		return;

	memory_print_map(b_log);

	if (db_init(map))
		return;

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

	/*
	 * Temporary code for testing purposes.
	 */
	{
		static const char *key_name[] = {
			"DANCY_KEY_NULL",
			"DANCY_KEY_ERROR1",
			"DANCY_KEY_ERROR2",
			"DANCY_KEY_UNDEFINED",
			"DANCY_KEY_A",
			"DANCY_KEY_B",
			"DANCY_KEY_C",
			"DANCY_KEY_D",
			"DANCY_KEY_E",
			"DANCY_KEY_F",
			"DANCY_KEY_G",
			"DANCY_KEY_H",
			"DANCY_KEY_I",
			"DANCY_KEY_J",
			"DANCY_KEY_K",
			"DANCY_KEY_L",
			"DANCY_KEY_M",
			"DANCY_KEY_N",
			"DANCY_KEY_O",
			"DANCY_KEY_P",
			"DANCY_KEY_Q",
			"DANCY_KEY_R",
			"DANCY_KEY_S",
			"DANCY_KEY_T",
			"DANCY_KEY_U",
			"DANCY_KEY_V",
			"DANCY_KEY_W",
			"DANCY_KEY_X",
			"DANCY_KEY_Y",
			"DANCY_KEY_Z",
			"DANCY_KEY_1",
			"DANCY_KEY_2",
			"DANCY_KEY_3",
			"DANCY_KEY_4",
			"DANCY_KEY_5",
			"DANCY_KEY_6",
			"DANCY_KEY_7",
			"DANCY_KEY_8",
			"DANCY_KEY_9",
			"DANCY_KEY_0",
			"DANCY_KEY_ENTER",
			"DANCY_KEY_ESCAPE",
			"DANCY_KEY_BACKSPACE",
			"DANCY_KEY_TAB",
			"DANCY_KEY_SPACE",
			"DANCY_KEY_UNDERSCORE",
			"DANCY_KEY_EQUALS",
			"DANCY_KEY_LEFTBRACKET",
			"DANCY_KEY_RIGHTBRACKET",
			"DANCY_KEY_BACKSLASH",
			"DANCY_KEY_HASH",
			"DANCY_KEY_SEMICOLON",
			"DANCY_KEY_APOSTROPHE",
			"DANCY_KEY_GRAVE",
			"DANCY_KEY_COMMA",
			"DANCY_KEY_PERIOD",
			"DANCY_KEY_SLASH",
			"DANCY_KEY_CAPSLOCK",
			"DANCY_KEY_F1",
			"DANCY_KEY_F2",
			"DANCY_KEY_F3",
			"DANCY_KEY_F4",
			"DANCY_KEY_F5",
			"DANCY_KEY_F6",
			"DANCY_KEY_F7",
			"DANCY_KEY_F8",
			"DANCY_KEY_F9",
			"DANCY_KEY_F10",
			"DANCY_KEY_F11",
			"DANCY_KEY_F12",
			"DANCY_KEY_PRINTSCREEN",
			"DANCY_KEY_SCROLLLOCK",
			"DANCY_KEY_PAUSE",
			"DANCY_KEY_INSERT",
			"DANCY_KEY_HOME",
			"DANCY_KEY_PAGEUP",
			"DANCY_KEY_DELETE",
			"DANCY_KEY_END",
			"DANCY_KEY_PAGEDOWN",
			"DANCY_KEY_RIGHTARROW",
			"DANCY_KEY_LEFTARROW",
			"DANCY_KEY_DOWNARROW",
			"DANCY_KEY_UPARROW",
			"DANCY_KEY_NUMLOCK",
			"DANCY_KEY_PADSLASH",
			"DANCY_KEY_PADASTERISK",
			"DANCY_KEY_PADMINUS",
			"DANCY_KEY_PADPLUS",
			"DANCY_KEY_PADENTER",
			"DANCY_KEY_PAD1",
			"DANCY_KEY_PAD2",
			"DANCY_KEY_PAD3",
			"DANCY_KEY_PAD4",
			"DANCY_KEY_PAD5",
			"DANCY_KEY_PAD6",
			"DANCY_KEY_PAD7",
			"DANCY_KEY_PAD8",
			"DANCY_KEY_PAD9",
			"DANCY_KEY_PAD0",
			"DANCY_KEY_PADDELETE"
		};
		size_t i;

		for (i = 0; i < boot_log_size + 1; i++) {
			if (!b_put_byte_com1((unsigned char)boot_log[i]))
				break;
		}
		for (i = 0; i < boot_log_size + 1; i++) {
			if (!b_put_byte_com2((unsigned char)boot_log[i]))
				break;
		}

		b_output_string(boot_log, (unsigned)boot_log_size);

		for (;;) {
			unsigned long r = b_get_keycode();

			if (r == 0)
				continue;
			b_print("%04lX ", r >> 16);
			b_print("%s\n", key_name[r & 0xFFul]);
		}
	}

	while (!b_pause()) { }
}
