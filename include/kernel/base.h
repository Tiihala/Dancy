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
 * kernel/base.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_BASE_H
#define KERNEL_BASE_H

#include <common/types.h>
#include <kernel/table.h>
#include <kernel/task.h>

/*
 * Declarations of apic.c
 */
void apic_eoi(void);
uint32_t apic_id(void);
void apic_send(uint32_t icr_low, uint32_t icr_high);
int apic_wait_delivery(void);

void ioapic_disable(int irq);
void ioapic_enable(int irq);
uint64_t ioapic_redtbl(int irq);

/*
 * Declarations of console.c
 */
int con_init(void);
int con_switch(int i);
void con_clear(void);
void con_panic(const char *message);
void con_print(const char *format, ...);
void con_write(const void *data, size_t size);

/*
 * Declarations of delay.c
 */
void delay(uint32_t nanoseconds);

/*
 * Declarations of event.c
 */
enum event_type {
	event_type_manual_reset = 0x01,
	event_type_set_signaled = 0x02
};

event_t event_create(int type);
void event_delete(event_t event);

void event_reset(event_t event);
void event_signal(event_t event);

int event_wait(event_t event, uint16_t milliseconds);
int event_wait_array(int count, event_t *events, uint16_t milliseconds);

void event_yield(void);

/*
 * Declarations of fb.c
 */
int fb_init(void);
void fb_panic(void);

void fb_enter(void);
void fb_leave(void);

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
	gdt_task_state  = 0x30,
	gdt_block_data  = 0x40
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
uint32_t gdt_read_segment(int sel, size_t offset);

/*
 * Declarations of heap.c
 */
int heap_init(void);
void heap_usage(size_t *allocated, size_t *unallocated);
void *heap_alloc_static_page(void);

/*
 * Declarations of idt.asm and idt.c
 */
extern uint8_t idt_asm_array[256][16];
extern const uint8_t idt_asm_gp_handler[];
extern const uint8_t idt_asm_handler[];
extern const uint8_t idt_asm_panic[];

extern uint32_t idt_nmi_panic[];
extern uint32_t idt_nmi_unknown;

extern void (*idt_msi_handler)(int irq);

int idt_init(void);
int idt_init_ap(void);

void idt_handler(int num, void *stack);
void idt_install_asm(int num, const uint8_t asm_handler[]);
void idt_load(const void *idt_ptr);

/*
 * Declarations of idt_user.c
 */
int idt_user_exception(int num, void *stack);

/*
 * Declarations of irq.c
 */
extern void (*irq_handler_apic)(int irq);
extern void (*irq_handler_pic)(int irq);

int irq_init(void);

void irq_disable(int irq);
void irq_enable(int irq);

void *irq_install(int irq, void *arg, void (*func)(int irq, void *arg));
void irq_uninstall(void *irq);

/*
 * Declarations of kmsg.c
 */
int kmsg_write(const char *message);
int printk(const char *format, ...);

/*
 * Declarations of mm.c
 */
enum mm_type {
	mm_normal = 0x00,
	mm_addr36 = 0x01,
	mm_addr32 = 0x02,
	mm_addr28 = 0x03,
	mm_addr24 = 0x04,
	mm_addr20 = 0x05,
	mm_kernel = 0x10
};

int mm_init(void);
size_t mm_available_pages(int type);

phys_addr_t mm_alloc_page(void);
phys_addr_t mm_alloc_pages(int type, int order);

void mm_free_page(phys_addr_t addr);
void mm_free_pages(phys_addr_t addr, int order);

/*
 * Declarations of mtx.c
 */
typedef void *mtx_t;

enum dancy_mtx_constants {
	mtx_plain,
	mtx_timed,
	mtx_recursive
};

enum dancy_thrd_constants {
	thrd_success,
	thrd_busy,
	thrd_error,
	thrd_nomem,
	thrd_timedout
};

void mtx_destroy(mtx_t *mtx);
int mtx_init(mtx_t *mtx, int type);
int mtx_lock(mtx_t *mtx);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);

/*
 * Declarations of panic.c
 */
extern int panic_lock;
void panic(const char *message);

/*
 * Declarations of pg.c
 */
enum pg_type {
	pg_normal   = 0x00,
	pg_uncached = 0x01,
	pg_extended = 0x02
};

extern cpu_native_t pg_kernel;

int pg_init(void);
int pg_init_ap(void);

int pg_create(void);
void pg_delete(void);
void pg_enter_kernel(void);
void pg_leave_kernel(void);

int pg_alt_create(void);
void pg_alt_accept(void);
void pg_alt_delete(void);

void *pg_get_entry(cpu_native_t cr3, const void *pte);
void *pg_map_kernel(phys_addr_t addr, size_t size, int type);
void *pg_map_user(addr_t vaddr, size_t size);
int pg_unmap_user(addr_t vaddr, size_t size);

uint64_t pg_read_memory(phys_addr_t addr, size_t size);
void pg_write_memory(phys_addr_t addr, uint64_t val, size_t size);

int pg_check_user_read(const void *vaddr, size_t size);
int pg_check_user_write(void *vaddr, size_t size);
int pg_check_user_string(const void *vaddr, int *count);
int pg_check_user_vector(const void *vaddr, int *count);

/*
 * Declarations of ret_user.c
 */
void ret_user_handler(struct task *current, void *stack);

/*
 * Declarations of runlevel.c
 */
int runlevel_init(void);
int runlevel_current(void);
int runlevel_send_request(int id);

/*
 * Declarations of spin.c
 */
void spin_lock_yield(int *lock);

/*
 * Declarations of start.c
 */
extern struct kernel_table *kernel;

void kernel_start(void);
void kernel_start_ap(void);

/*
 * Declarations of timer.asm
 */
extern uint8_t timer_apic_base[];
extern const uint8_t timer_asm_handler_apic[];
extern const uint8_t timer_asm_handler_apic_ap[];
extern const uint8_t timer_asm_handler_pic[];

extern volatile uint32_t timer_ticks;
extern volatile uint32_t timer_fault_count;

uint64_t timer_read(void);

#endif
