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
 * libc/keymap/_keymap.c
 *      The keyboard tables
 */

#include <__dancy/keys.h>

extern const struct __dancy_keymap __dancy_keymap_dk;
extern const struct __dancy_keymap __dancy_keymap_uk;
extern const struct __dancy_keymap __dancy_keymap_us;
extern const struct __dancy_keymap __dancy_keymap_et;
extern const struct __dancy_keymap __dancy_keymap_fi;
extern const struct __dancy_keymap __dancy_keymap_de;
extern const struct __dancy_keymap __dancy_keymap_no;
extern const struct __dancy_keymap __dancy_keymap_sv;

const struct __dancy_keymap *const __dancy_keymaps[] = {
	&__dancy_keymap_dk,
	&__dancy_keymap_uk,
	&__dancy_keymap_us,
	&__dancy_keymap_et,
	&__dancy_keymap_fi,
	&__dancy_keymap_de,
	&__dancy_keymap_no,
	&__dancy_keymap_sv,
	NULL
};
