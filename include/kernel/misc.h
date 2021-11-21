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
 * kernel/misc.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_MISC_H
#define KERNEL_MISC_H

#include <dancy/types.h>

/*
 * Declarations of rtc.c
 */
int rtc_init(void);

/*
 * Declarations of serial.c
 */
extern event_t serial_event[4];

int serial_init(void);

int serial_get_baud(int port);
int serial_set_baud(int port, int baud);

size_t serial_receive(int port, void *buf, size_t size);
size_t serial_send(int port, void *buf, size_t size);

#endif
