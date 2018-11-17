/*
 * Copyright (c) 2017, 2018 Antti Tiihala
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
 * dancy.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_H
#define DANCY_H

#include <dancy/limits.h>
#include <dancy/types.h>

#define DANCY_BLOB(a,b,c,d,e,f,g,h) \
(unsigned)(a) >> 8 & 0xFFu, (unsigned)(a) & 0xFFu, \
(unsigned)(b) >> 8 & 0xFFu, (unsigned)(b) & 0xFFu, \
(unsigned)(c) >> 8 & 0xFFu, (unsigned)(c) & 0xFFu, \
(unsigned)(d) >> 8 & 0xFFu, (unsigned)(d) & 0xFFu, \
(unsigned)(e) >> 8 & 0xFFu, (unsigned)(e) & 0xFFu, \
(unsigned)(f) >> 8 & 0xFFu, (unsigned)(f) & 0xFFu, \
(unsigned)(g) >> 8 & 0xFFu, (unsigned)(g) & 0xFFu, \
(unsigned)(h) >> 8 & 0xFFu, (unsigned)(h) & 0xFFu

unsigned long b_a0(const char *, unsigned int);
unsigned long b_a1(const char *, unsigned int);
unsigned long b_a2(unsigned int, unsigned int);
unsigned long b_a3(void);
unsigned long b_a4(void);
unsigned long b_a5(unsigned char);
unsigned long b_a6(void);
unsigned long b_a7(unsigned char);
unsigned long b_a8(unsigned int);
unsigned long b_a9(void *, unsigned int);
unsigned long b_aa(void *, unsigned int);
unsigned long b_ab(unsigned int, unsigned int);
unsigned long b_ac(void *, unsigned int);
unsigned long b_ad(unsigned int, unsigned int);
unsigned long b_ae(void);
unsigned long b_af(void);

#define b_output_string         (b_a0)
#define b_output_string_hl      (b_a1)
#define b_output_control        (b_a2)
#define b_get_keycode           (b_a3)
#define b_get_byte_com1         (b_a4)
#define b_put_byte_com1         (b_a5)
#define b_get_byte_com2         (b_a6)
#define b_put_byte_com2         (b_a7)
#define b_get_parameter         (b_a8)
#define b_get_structure         (b_a9)
#define b_set_read_buffer       (b_aa)
#define b_read_blocks           (b_ab)
#define b_set_write_buffer      (b_ac)
#define b_write_blocks          (b_ad)
#define b_pause                 (b_ae)
#define b_exit                  (b_af)

enum b_parameter {
	B_BYTES_PER_BLOCK,
	B_TOTAL_BLOCKS,
	B_HIDDEN_BLOCKS,
	B_DRIVE_NUMBER,
	B_MEDIA_CHANGED,
	B_A20_STATE,
	B_DROPPED_BYTES_COM1,
	B_DROPPED_BYTES_COM2,
	B_DRIVE_PARAMETERS_80H,
	B_DRIVE_PARAMETERS_81H,
	B_DRIVE_PARAMETERS_82H,
	B_DRIVE_PARAMETERS_83H,
	B_ACPI_POINTER
};

enum b_structure {
	B_VIDEO_INFO,
	B_VIDEO_EDID
};

struct b_video_info {
	uint32_t mode;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	phys_addr_t framebuffer;

#if defined(DANCY_64_BIT)
	uint8_t reserved[8];
#else
	uint8_t reserved[12];
#endif
};

enum dancy_key {
	DANCY_KEY_NULL,
	DANCY_KEY_ERROR1,
	DANCY_KEY_ERROR2,
	DANCY_KEY_UNDEFINED,
	DANCY_KEY_A,
	DANCY_KEY_B,
	DANCY_KEY_C,
	DANCY_KEY_D,
	DANCY_KEY_E,
	DANCY_KEY_F,
	DANCY_KEY_G,
	DANCY_KEY_H,
	DANCY_KEY_I,
	DANCY_KEY_J,
	DANCY_KEY_K,
	DANCY_KEY_L,
	DANCY_KEY_M,
	DANCY_KEY_N,
	DANCY_KEY_O,
	DANCY_KEY_P,
	DANCY_KEY_Q,
	DANCY_KEY_R,
	DANCY_KEY_S,
	DANCY_KEY_T,
	DANCY_KEY_U,
	DANCY_KEY_V,
	DANCY_KEY_W,
	DANCY_KEY_X,
	DANCY_KEY_Y,
	DANCY_KEY_Z,
	DANCY_KEY_1,
	DANCY_KEY_2,
	DANCY_KEY_3,
	DANCY_KEY_4,
	DANCY_KEY_5,
	DANCY_KEY_6,
	DANCY_KEY_7,
	DANCY_KEY_8,
	DANCY_KEY_9,
	DANCY_KEY_0,
	DANCY_KEY_ENTER,
	DANCY_KEY_ESCAPE,
	DANCY_KEY_BACKSPACE,
	DANCY_KEY_TAB,
	DANCY_KEY_SPACE,
	DANCY_KEY_UNDERSCORE,
	DANCY_KEY_EQUALS,
	DANCY_KEY_LEFTBRACKET,
	DANCY_KEY_RIGHTBRACKET,
	DANCY_KEY_BACKSLASH,
	DANCY_KEY_HASH,
	DANCY_KEY_SEMICOLON,
	DANCY_KEY_APOSTROPHE,
	DANCY_KEY_GRAVE,
	DANCY_KEY_COMMA,
	DANCY_KEY_PERIOD,
	DANCY_KEY_SLASH,
	DANCY_KEY_CAPSLOCK,
	DANCY_KEY_F1,
	DANCY_KEY_F2,
	DANCY_KEY_F3,
	DANCY_KEY_F4,
	DANCY_KEY_F5,
	DANCY_KEY_F6,
	DANCY_KEY_F7,
	DANCY_KEY_F8,
	DANCY_KEY_F9,
	DANCY_KEY_F10,
	DANCY_KEY_F11,
	DANCY_KEY_F12,
	DANCY_KEY_PRINTSCREEN,
	DANCY_KEY_SCROLLLOCK,
	DANCY_KEY_PAUSE,
	DANCY_KEY_INSERT,
	DANCY_KEY_HOME,
	DANCY_KEY_PAGEUP,
	DANCY_KEY_DELETE,
	DANCY_KEY_END,
	DANCY_KEY_PAGEDOWN,
	DANCY_KEY_RIGHTARROW,
	DANCY_KEY_LEFTARROW,
	DANCY_KEY_DOWNARROW,
	DANCY_KEY_UPARROW,
	DANCY_KEY_NUMLOCK,
	DANCY_KEY_PADSLASH,
	DANCY_KEY_PADASTERISK,
	DANCY_KEY_PADMINUS,
	DANCY_KEY_PADPLUS,
	DANCY_KEY_PADENTER,
	DANCY_KEY_PAD1,
	DANCY_KEY_PAD2,
	DANCY_KEY_PAD3,
	DANCY_KEY_PAD4,
	DANCY_KEY_PAD5,
	DANCY_KEY_PAD6,
	DANCY_KEY_PAD7,
	DANCY_KEY_PAD8,
	DANCY_KEY_PAD9,
	DANCY_KEY_PAD0,
	DANCY_KEY_PADDELETE
};

#define DANCY_KEYMOD_LCTRL  (0x00010000ul)
#define DANCY_KEYMOD_LSHIFT (0x00020000ul)
#define DANCY_KEYMOD_LALT   (0x00040000ul)
#define DANCY_KEYMOD_LGUI   (0x00080000ul)
#define DANCY_KEYMOD_RCTRL  (0x00100000ul)
#define DANCY_KEYMOD_RSHIFT (0x00200000ul)
#define DANCY_KEYMOD_RALT   (0x00400000ul)
#define DANCY_KEYMOD_RGUI   (0x00800000ul)

#define DANCY_KEYMOD_CTRL   (DANCY_KEYMOD_LCTRL  | DANCY_KEYMOD_RCTRL)
#define DANCY_KEYMOD_SHIFT  (DANCY_KEYMOD_LSHIFT | DANCY_KEYMOD_RSHIFT)
#define DANCY_KEYMOD_ALT    (DANCY_KEYMOD_LALT   | DANCY_KEYMOD_RALT)

#endif
