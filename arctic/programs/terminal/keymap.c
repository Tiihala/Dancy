/*
 * Copyright (c) 2024, 2025 Antti Tiihala
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
 * terminal/keymap.c
 *      The terminal program
 */

#include "main.h"

#define T0 "\xE2\x94\x8C"
#define T1 "\xE2\x94\x80"
#define T2 "\xE2\x94\x90"
#define T3 "\xE2\x94\x90"
#define T4 "\xE2\x94\x82"
#define T5 "\xE2\x94\x98"
#define T6 "\xE2\x94\x94"

static int visible_lines = 8;

static void modify_tab_stops(void)
{
	static int state = 0;
	const int ui_width = 46;
	int i;

	printf("\r");

	if (state == 0) {
		struct winsize w = { 0 };

		ioctl(1, TIOCGWINSZ, &w);

		if ((int)w.ws_col < (ui_width + 16)) {
			state = -1;
			return;
		}

		state = ((int)w.ws_col - ui_width) / 2;

		for (i = 0; i < (state / 8); i++)
			printf("\033[%dG\033[g\r", ((i + 1) * 8) + 1);

		printf("\033[%dG\033H\r", state + 1);
		return;
	}

	if (state > 1) {
		printf("\033[%dG\033[g\r", state + 1);

		for (i = 0; i < (state / 8); i++)
			printf("\033[%dG\033H\r", ((i + 1) * 8) + 1);
	}

	state = 0;
}

static void render_list(int offset, int active, int count)
{
	int i;

	for (i = offset; i < offset + visible_lines; i++) {
		printf("\t\033[32m" T4 "\033[0m " T4 " ");

		if (i == active)
			printf("\033[7m");

		if (i < count) {
			const struct __dancy_keymap *k = __dancy_keymaps[i];
			printf("%8s  %-28s", k->name, k->description);
		} else {
			printf("%8s  %-28s", "", "");
		}

		printf("\033[0m " T4 "\033[32m " T4 "\033[0m\n");
	}

	printf("\033[%dA", visible_lines);
}

const struct __dancy_keymap *select_keymap(int fd_keyboard)
{
	int offset = 0, active = 0, count = 0;
	int i;

	for (i = 0; __dancy_keymaps[i] != NULL; i++)
		count += 1;

	if (count <= 1)
		return __dancy_keymaps[0];

	if (visible_lines > count)
		visible_lines = count;

	/*
	 * The Finnish keyboard layout is the default for now.
	 */
	for (i = 0; i < count; i++) {
		if (!strcmp(__dancy_keymaps[i]->name, "fi")) {
			if ((active = i) >= visible_lines)
				offset = active - (visible_lines / 2);
			break;
		}
	}

	printf("\033[?25l\n");
	modify_tab_stops();

	printf("\t\033[32m" T0
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		" \033[93mKEYBOARD MAP SELECTION\033[32m "
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T2 "\033[0m\n");

	printf("\t\033[32m" T4 "\033[0m"
		" Select one of the following keyboard maps. "
		"\033[32m" T4 "\033[0m\n");

	printf("\t\033[32m" T4 "\033[0m"
		"  Use the UP and DOWN arrow keys to scroll  "
		"\033[32m" T4 "\033[0m\n");

	printf("\t\033[32m" T4 "\033[0m"
		" through the list, and press the ENTER key. "
		"\033[32m" T4 "\033[0m\n");

	printf("\t\033[32m" T4 "%44s" T4 "\033[0m\n", "");

	printf("\t\033[32m" T4 "\033[0m " T0
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T2 " \033[32m" T4 "\033[0m\n");

	for (i = 0; i < visible_lines; i++)
		printf("\n");

	printf("\t\033[32m" T4 "\033[0m " T6
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T5 " \033[32m" T4 "\033[0m\n");

	printf("\t\033[32m" T6
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1 T1
		T5 "\033[0m\n");

	printf("\033[%dA", visible_lines + 2);

	for (;;) {
		int key = 0;
		size_t size = sizeof(key);
		struct pollfd fds[1];

		render_list(offset, active, count);

		fds[0].fd = fd_keyboard;
		fds[0].events = POLLIN;
		fds[0].revents = 0;

		poll(&fds[0], 1, 1000);

		if (read(fd_keyboard, &key, size) != (ssize_t)size)
			continue;

		if ((key & __DANCY_KEYTYP_RELEASE) != 0)
			continue;

		key = key & 0xFF;

		if (key == __DANCY_KEY_ENTER || key == __DANCY_KEY_PADENTER)
			break;

		if (key == __DANCY_KEY_DOWNARROW) {
			if (active + 1 < count) {
				active += 1;
				if (active - offset >= visible_lines)
					offset += 1;
			}
			continue;
		}

		if (key == __DANCY_KEY_UPARROW) {
			if (active - 1 >= 0) {
				active -= 1;
				if (active < offset)
					offset -= 1;
			}
			continue;
		}
	}

	modify_tab_stops();
	printf("\033[7A\033[%dM\033[?25h", visible_lines + 9);
	fflush(stdout);

	return __dancy_keymaps[active];
}
