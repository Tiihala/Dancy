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
 * Declarations of gdt.asm and gdt.c
 */
enum gdt_sel {
	gdt_null_1      = 0x00,
	gdt_kernel_code = 0x08,
	gdt_kernel_data = 0x10,
	gdt_null_2      = 0x18,
	gdt_user_data   = 0x20,
	gdt_user_code   = 0x28,
	gdt_task_state  = 0x30
};

enum gdt_sys_sel {
	gdt_syscall_sel = 0x08,
	gdt_sysret_sel  = 0x18
};

int gdt_init(void);
int gdt_init_ap(void);

void gdt_load(const void *gdt_ptr);
void gdt_load_cs(int sel);
void gdt_load_es(int sel);
void gdt_load_ss(int sel);
void gdt_load_ds(int sel);
void gdt_load_fs(int sel);
void gdt_load_gs(int sel);

void *gdt_get_tss(void);
void gdt_load_tss(int sel);

/*
 * Declarations of heap.c
 */
int heap_init(void);

/*
 * Declarations of idt.asm and idt.c
 */
extern uint8_t idt_asm_array[256][16];
extern const uint8_t idt_asm_gp_handler[];
extern const uint8_t idt_asm_handler[];

int idt_init(void);
int idt_init_ap(void);

void idt_handler(int num, void *stack);
void idt_install_asm(int num, const uint8_t asm_handler[]);
void idt_load(const void *idt_ptr);

/*
 * Declarations of start.c
 */
extern struct kernel_table *kernel;

void kernel_start(void);
void kernel_start_ap(void);

/*
 * Declarations of timer.asm and timer.c
 */
extern uint8_t timer_apic_base[];
extern const uint8_t timer_asm_handler_apic[];
extern const uint8_t timer_asm_handler_pic[];

extern volatile uint32_t timer_ticks;
extern volatile uint64_t timer_ticks_64;
extern volatile uint32_t timer_ticks_wait;

void timer_handler(void);

#endif
