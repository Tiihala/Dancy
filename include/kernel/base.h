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
 * kernel/base.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_BASE_H
#define KERNEL_BASE_H

#include <dancy/types.h>
#include <kernel/table.h>

/*
 * Declarations of apic.c
 */
void apic_eoi(void);
uint32_t apic_id(void);
void apic_send(uint32_t icr_low, uint32_t icr_high);
int apic_wait_delivery(void);

/*
 * Declarations of delay.c
 */
void delay(uint32_t nanoseconds);

/*
 * Declarations of start.c
 */
extern struct kernel_table *kernel;

void kernel_start(void);
void kernel_start_ap(void);

#endif
