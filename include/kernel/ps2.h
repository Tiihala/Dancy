/*
 * Copyright (c) 2021 Antti Tiihala
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
 * kernel/ps2.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_PS2_H
#define KERNEL_PS2_H

#include <dancy/types.h>

/*
 * Declarations of 8042.c
 */
extern event_t ps2_event_port1;
extern event_t ps2_event_port2;

int ps2_init(void);

int ps2_receive_port1(void);
int ps2_receive_port2(void);

int ps2_send_port1(uint8_t val);
int ps2_send_port2(uint8_t val);

/*
 * Declarations of keyboard.c
 */
int ps2_kbd_init(void);
void ps2_kbd_handler(void);

/*
 * Declarations of mouse.c
 */
int ps2_mse_init(void);
void ps2_mse_handler(void);

#endif
