/*
 * Copyright (c) 2021, 2022 Antti Tiihala
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

#include <common/types.h>
#include <kernel/vfs.h>

/*
 * Declarations of bin.c
 */
int bin_init(void);

/*
 * Declarations of coff.c
 */
extern const unsigned int coff_native_signature;
int coff_load_executable(struct vfs_node *node, addr_t *start_addr);

/*
 * Declarations of console.c
 */
int console_init(void);

/*
 * Declarations of dma.c
 */
int dma_init(void);
phys_addr_t dma_set_floppy_read(size_t size);
phys_addr_t dma_set_floppy_write(size_t size);

/*
 * Declarations of epoch.c
 */
unsigned long long epoch_read(void);
unsigned long long epoch_read_ms(void);
void epoch_sync(void);

/*
 * Declarations of fb_user.c
 */
int fb_user_init(void);

/*
 * Declarations of floppy.c
 */
int floppy_init(void);
int floppy_read(int dsel, uint64_t offset, size_t *size, void *buffer);
int floppy_write(int dsel, uint64_t offset, size_t *size, const void *buffer);
int floppy_test(int dsel);

/*
 * Declarations of hdd_mnt.c
 */
int hdd_mnt_init(void);

/*
 * Declarations of hdd_part.c
 */
int hdd_part_init(void);

/*
 * Declarations of keyboard.c
 */
int dancy_kbd_init(void);
void dancy_kbd_clear(void);
int dancy_kbd_write(size_t *size, void *buffer);

/*
 * Declarations of kmsg.c
 */
int kmsg_init(void);

/*
 * Declarations of mouse.c
 */
int dancy_mse_init(void);
void dancy_mse_clear(void);
int dancy_mse_write(size_t *size, void *buffer);

/*
 * Declarations of rtc.c
 */
struct rtc_time {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t reserved[9];
};

int rtc_init(void);
int rtc_read(struct rtc_time *rt);

/*
 * Declarations of serial.c
 */
extern event_t serial_event[4];

int serial_init(void);

int serial_get_baud(int port);
int serial_set_baud(int port, int baud);

size_t serial_receive(int port, void *buf, size_t size);
size_t serial_send(int port, const void *buf, size_t size);

/*
 * Declarations of zero.c
 */
int zero_init(void);

#endif
