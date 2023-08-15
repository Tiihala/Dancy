/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/keymap/_write.c
 *      The __dancy_keymap_write function
 */

#include <__dancy/keys.h>
#include <errno.h>
#include <unistd.h>

static void call_write(int fd, unsigned char *utf8_buffer, size_t utf8_size)
{
	if (utf8_size && utf8_buffer[0]) {
		int saved_errno = errno;
		ssize_t r = write(fd, utf8_buffer, utf8_size);

		if (r == (ssize_t)utf8_size)
			errno = saved_errno;
		else if (r >= 0)
			errno = EIO;
	}
}

static size_t utf8_encode(unsigned int unicode, unsigned char *out)
{
	if (unicode <= 0x7F) {
		out[0] = (unsigned char)unicode;
		return 1;
	}

	if (unicode <= 0x07FF) {
		out[0] = (unsigned char)(0xC0 + (unicode >> 6));
		out[1] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 2;
	}

	if ((unicode >= 0xD800 && unicode <= 0xDFFF))
		return 0;

	if (unicode <= 0xFFFF) {
		out[0] = (unsigned char)(0xE0 + (unicode >> 12));
		out[1] = (unsigned char)(0x80 + ((unicode >> 6) & 0x3F));
		out[2] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 3;
	}

	if (unicode <= 0x10FFFF) {
		out[0] = (unsigned char)(0xF0 + (unicode >> 18));
		out[1] = (unsigned char)(0x80 + ((unicode >> 12) & 0x3F));
		out[2] = (unsigned char)(0x80 + ((unicode >> 6) & 0x3F));
		out[3] = (unsigned char)(0x80 + (unicode & 0x3F));
		return 4;
	}

	return 0;
}

/*
 * This function shall not change the errno value if there are no errors.
 */
void __dancy_keymap_write(int fd, int key,
	const struct __dancy_keymap *map, int *state)
{
	int *dead_key_state = state;

	unsigned char utf8_buffer[4];
	unsigned int unicode;
	size_t utf8_size;

	int mod_lshift = (key & __DANCY_KEYMOD_LSHIFT);
	int mod_rshift = (key & __DANCY_KEYMOD_RSHIFT);

	int mod_lctrl = (key & __DANCY_KEYMOD_LCTRL);
	int mod_rctrl = (key & __DANCY_KEYMOD_RCTRL);

	int mod_alt = (key & __DANCY_KEYMOD_LALT);
	int mod_altgr = (key & __DANCY_KEYMOD_RALT);

	int mod_capslock = (key & __DANCY_KEYMOD_CAPSLOCK);
	int mod_numlock = (key & __DANCY_KEYMOD_NUMLOCK);

	int i, map_i = 0;
	int value;

	if ((key & __DANCY_KEYTYP_RELEASE) != 0)
		return;

	map_i |= ((mod_lshift || mod_rshift) ? __DANCY_KEYMAP_SHIFT : 0);
	map_i |= ((mod_lctrl || mod_rctrl) ? __DANCY_KEYMAP_CTRL : 0);
	map_i |= ((mod_altgr) ? __DANCY_KEYMAP_ALTGR : 0);
	map_i |= ((mod_alt) ? __DANCY_KEYMAP_ALT : 0);

	if ((value = map->map[map_i][key & 127]) == 0)
		return;

	if ((value & __DANCY_KEYBIT_CAPS) != 0) {
		if (mod_capslock) {
			map_i ^= __DANCY_KEYMAP_SHIFT;
			if ((value = map->map[map_i][key & 127]) == 0)
				return;
		}
	}

	if ((value & __DANCY_KEYBIT_NPAD) != 0) {
		if ((map_i & __DANCY_KEYMAP_SHIFT) != 0)
			return;

		if (mod_numlock) {
			int numlock_map_i = __DANCY_KEYMAP_SHIFT;
			if ((value = map->map[numlock_map_i][key & 127]) == 0)
				return;
		}
	}

	if ((value & __DANCY_KEYBIT_DEAD) != 0) {
		if (*dead_key_state == 0) {
			*dead_key_state = (value & 0x1FFFFF);
			return;
		}

		unicode = (unsigned int)(*dead_key_state & 0x1FFFFF);
		utf8_size = utf8_encode(unicode, &utf8_buffer[0]);
		call_write(fd, &utf8_buffer[0], utf8_size);

		*dead_key_state = 0;
	}

	if (*dead_key_state) {
		int dead_key = (*dead_key_state & 0x1FFFFF);
		int diacritic_found = 0;

		*dead_key_state = 0;
		unicode = (unsigned int)(value & 0x1FFFFF);

		if (unicode == 0x20) {
			value = dead_key;
			diacritic_found = 1;
		} else {
			for (i = 0; i < 64; i++) {
				int d0 = map->diacritic[i][0];
				int d1 = map->diacritic[i][1];

				if (d0 == 0 || d1 == 0)
					break;

				if (d0 == dead_key && d1 == (int)unicode) {
					value = map->diacritic[i][2];
					diacritic_found = 1;
					break;
				}
			}
		}

		if (!diacritic_found) {
			unicode = (unsigned int)dead_key;
			utf8_size = utf8_encode(unicode, &utf8_buffer[0]);
			call_write(fd, &utf8_buffer[0], utf8_size);
		}
	}

	unicode = (unsigned int)(value & 0x1FFFFF);
	utf8_size = utf8_encode(unicode, &utf8_buffer[0]);
	call_write(fd, &utf8_buffer[0], utf8_size);
}
