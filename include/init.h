/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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
 * init.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_INIT_H
#define DANCY_INIT_H

#include <dancy/blob.h>
#include <dancy/ctype.h>
#include <dancy/keys.h>
#include <dancy/lib.h>
#include <dancy/limits.h>
#include <dancy/stdarg.h>
#include <dancy/stdio.h>
#include <dancy/stdlib.h>
#include <dancy/string.h>
#include <dancy/symbol.h>
#include <dancy/threads.h>
#include <dancy/time.h>
#include <dancy/types.h>

#include <init/boot.h>
#include <init/palette.h>


/*
 * Declarations of acpi.c
 */
#define INIT_ARCH_LEGACY_DEVICES        (1u << 0)
#define INIT_ARCH_8042                  (1u << 1)
#define INIT_ARCH_VGA_NOT_PRESENT       (1u << 2)
#define INIT_ARCH_MSI_NOT_SUPPORTED     (1u << 3)
#define INIT_ARCH_PCIE_ASPM_CONTROLS    (1u << 4)
#define INIT_ARCH_CMOS_RTC_NOT_PRESENT  (1u << 5)

struct acpi_information {
	phys_addr_t rsdp_addr;
	phys_addr_t rsdt_addr;
	phys_addr_t xsdt_addr;
	phys_addr_t fadt_addr;
	phys_addr_t madt_addr;
	phys_addr_t mcfg_addr;
	phys_addr_t hpet_addr;

	unsigned rtc_century_idx;
	unsigned iapc_boot_arch;

	phys_addr_t local_apic_base;
	int dual_8259_setup;
	int irq0_to_input2_override;

	unsigned num_cpu_core;
	unsigned num_io_apic;

	unsigned max_apic_id;
	unsigned max_io_apic_id;

	phys_addr_t hpet_base;
	unsigned hpet_block_id;
	unsigned hpet_min_tick;
};

struct acpi_information *acpi_get_information(void);

struct acpi_apic {
	uint32_t id;
	int enabled;
};

struct acpi_irq_override {
	uint32_t global_int;
	uint32_t flags;
};

struct acpi_io_apic {
	uint32_t id;
	uint32_t base_int;
	phys_addr_t addr;

	struct acpi_irq_override irq[16];
};

int acpi_get_apic(unsigned idx, struct acpi_apic *apic);
int acpi_get_io_apic(unsigned idx, struct acpi_io_apic *io_apic);


/*
 * Declarations of apic.c
 */
extern int apic_mode;
extern const unsigned apic_spurious_vector;
extern const unsigned ioapic_irq_base;
extern uint32_t apic_bsp_id;

int apic_init(void);
void apic_eoi(void);
uint32_t apic_id(void);
void apic_send(uint32_t icr_low, uint32_t icr_high);
int apic_wait_delivery(void);

void ioapic_init(void);
void ioapic_enable(unsigned isa_irq);


/*
 * Declarations of cpu.asm and cpu.c
 */
int cpu_test_features(void);
void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b);
void cpu_halt(uint32_t counter);
int cpu_ints(int enable);

void cpu_rdtsc(uint32_t *a, uint32_t *d);
void cpu_rdtsc_delay(uint32_t a, uint32_t d);
void cpu_rdtsc_diff(uint32_t *a, uint32_t *d);

void cpu_rdmsr(uint32_t msr, uint32_t *a, uint32_t *d);
void cpu_wrmsr(uint32_t msr, uint32_t a, uint32_t d);

uint8_t cpu_in8(uint16_t port);
uint16_t cpu_in16(uint16_t port);
uint32_t cpu_in32(uint16_t port);

void cpu_out8(uint16_t port, uint8_t value);
void cpu_out16(uint16_t port, uint16_t value);
void cpu_out32(uint16_t port, uint32_t value);

uint8_t cpu_read8(const void *address);
uint16_t cpu_read16(const void *address);
uint32_t cpu_read32(const void *address);

void cpu_write8(void *address, uint8_t value);
void cpu_write16(void *address, uint16_t value);
void cpu_write32(void *address, uint32_t value);


/*
 * Declarations of gdt.asm
 */
void gdt_init(void);


/*
 * Declarations of db.c
 */
int db_init(struct b_mem *map);
void db_free(void);
int db_read(const char *name, unsigned char **buf, size_t *size);
const char *db_read_error(int retval);


/*
 * Declarations of delay.c
 */
extern volatile int delay_ready;
extern volatile int delay_error;
extern volatile uint64_t delay_tsc_hz;

void delay_calibrate(void);
void delay(uint32_t nanoseconds);


/*
 * Declarations of fs.c
 */
int fat_get_size(int id, size_t *block_size, size_t *block_total);
int fat_get_time(char iso_8601_format[19]);
int fat_io_read(int id, size_t lba, size_t *size, void *buf);
int fat_io_write(int id, size_t lba, size_t *size, const void *buf);

int fs_init(void);
void fs_free(void);
void fs_print_error(int r, const char *name);
int fs_update_config_at(struct b_video_info *vi);
void fs_write_logs(void);


/*
 * Declarations of gui.c
 */
extern mtx_t gui_mtx;
extern int (*gui_mtx_lock)(mtx_t *);
extern int (*gui_mtx_unlock)(mtx_t *);

int gui_init(void);
int gui_create_window(const char *name, int x1, int y1, int x2, int y2);
int gui_delete_window(void);
int gui_draw(unsigned char *png, size_t size, int x1, int y1, int x2, int y2);
int gui_move_window(int x, int y);
void gui_print(const char *format, ...);
void gui_print_alert(const char *message);
void gui_refresh(void);

extern thrd_t gui_thr;
int gui_thread(void *arg);


/*
 * Declarations of hpet.c
 */
extern int hpet_mode;

void hpet_init(void);


/*
 * Declarations of idt.asm and idt.c
 */
extern volatile unsigned idt_irq0;

void idt_init(void);
void idt_handler(unsigned num, const void *stack);
void idt_load_null(void);
void idt_restore(void);


/*
 * Declarations of init.c
 */
void init(void);
void init_ap(uint32_t id);


/*
 * Declarations of kernel.asm and kernel.c
 */
void kernel_init(void);
void kernel_init_ap(uint32_t id);
void kernel_jump(void (*func)(void), void *stack);


/*
 * Declarations of ld.asm and ld.c
 */
struct global_symbol {
	uint32_t value;
	char name[36];
};

extern struct global_symbol *global_symbols;
extern size_t global_symbols_size;

struct ld_object {
	void *base_address;
	size_t reserved_size;
	size_t total_size;

	void *text_section;
	void *rdata_section;
	void *data_section;
	void *bss_section;

	size_t text_size;
	size_t rdata_size;
	size_t data_size;
	size_t bss_size;
};

int ld_init(size_t symbols);
int ld_add(const struct global_symbol *symbol);
int ld_find(const char *name, struct global_symbol **symbol);
void ld_free(void);
int ld_link(const char *name, unsigned char *obj, struct ld_object *ld_obj);
void ld_relocate(void *base, void *reloc, void *symbol);
int ld_validate(const char *name, unsigned char *obj, size_t size);


/*
 * Declarations of log.c
 */
extern char *boot_log;
extern size_t boot_log_size;
int b_log_init(size_t max_size);
void b_log_free(void);
void b_log(const char *format, ...);


/*
 * Declarations of memory.c
 */
extern size_t memory_entries;
extern void *memory_map;

extern mtx_t memory_mtx;
extern int (*memory_mtx_lock)(mtx_t *);
extern int (*memory_mtx_unlock)(mtx_t *);

int memory_init(void *map);
void memory_print_map(void (*print)(const char *, ...));


/*
 * Declarations of panic.c
 */
void panic(const char *message);


/*
 * Declarations of pci.c
 */
struct pci_device {
	phys_addr_t ecam;
	int group;
	int bus;
	int device;
	int func;
	uint32_t vendor_id;
	uint32_t device_id;
	uint32_t class_code;
	int header_type;
};

extern uint32_t pci_device_count;
extern struct pci_device *pci_devices;

int pci_init_early(void);
void pci_init(void);
uint32_t pci_read(const struct pci_device *pci, int off);
void pci_write(struct pci_device *pci, int off, uint32_t val);


/*
 * Declarations of pg.asm and pg.c
 */
extern volatile uint32_t pg_fault_counter;
extern volatile uint32_t pg_tables_size;

int pg_init(void);
int pg_handler(void);

void pg_map_uncached(void *addr);
void pg_get_current(phys_addr_t *addr);
void pg_get_fault(phys_addr_t *addr);
void pg_switch(phys_addr_t addr);


/*
 * Declarations of pit.c
 */
void pit_init(void);


/*
 * Declarations of print.c
 */
extern void (*b_print)(const char *format, ...);


/*
 * Declarations of rtc.c
 */
int rtc_read(struct b_time *bt);


/*
 * Declarations of smp.c and trampoline code.
 */
extern uint32_t smp_ap_count;
extern uint32_t *smp_ap_id;
extern const unsigned char smp_trampoline[512];

void smp_init(void);


/*
 * Declarations of start.c
 */
void start_init(void *map);


/*
 * Declarations of thrd.asm and thrd.c
 *
 * If the following structure is edited, the assembly
 * functions in thrd.asm files must be checked.
 */
struct init_thrd {
	struct init_thrd *next; /* DANCY_32: 0   DANCY_64: 0  */
	void *stack;            /* DANCY_32: 4   DANCY_64: 8  */
	uint32_t detached;      /* DANCY_32: 8   DANCY_64: 16 */
	int res;                /* DANCY_32: 12  DANCY_64: 20 */
	int terminated;         /* DANCY_32: 16  DANCY_64: 24 */
};

struct init_mtx {
	int init;
	int lock;
	int type;
	unsigned count;
	uint32_t id;
};

extern struct init_thrd *init_thrd_current;
extern const size_t init_thrd_size;

void init_thrd_create(void *thr, thrd_start_t func, void *arg);


/*
 * Declarations of usb.c
 */
int usb_init_early(void);
void usb_init(void);


/*
 * Declarations of vga.c
 */
void vga_set_palette(const struct b_video_info *vi);

#endif
