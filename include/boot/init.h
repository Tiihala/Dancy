/*
 * Copyright (c) 2019, 2020, 2021 Antti Tiihala
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
 * boot/init.h
 *      Header of Dancy Operating System
 */

#ifndef BOOT_INIT_H
#define BOOT_INIT_H

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

#include <boot/loader.h>
#include <kernel/table.h>

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
extern phys_addr_t apic_base_addr;
extern const unsigned apic_spurious_vector;
extern const unsigned ioapic_irq_base;
extern uint32_t apic_bsp_id;

int apic_init(void);
void apic_eoi(void);
uint32_t apic_id(void);
void apic_send(uint32_t icr_low, uint32_t icr_high);
void apic_start_timer(void);
int apic_wait_delivery(void);

void ioapic_init(void);
void ioapic_enable(unsigned isa_irq);


/*
 * Declarations of cpu.c
 */
extern int cpu_osfxr_support;
extern int cpu_nxbit_support;
extern int cpu_gpage_support;
extern int cpu_rdtscp_support;

int cpu_test_features(void);
void cpu_init_control_registers(void);


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
extern struct b_video_info *gui_video_info;

int gui_init(void);
int gui_create_window(const char *name, int x1, int y1, int x2, int y2);
int gui_delete_window(void);
int gui_detach(void);
int gui_draw(unsigned char *png, size_t size, int x1, int y1, int x2, int y2);

void gui_print(const char *format, ...);
void gui_print_alert(const char *message);
void gui_refresh(void);


/*
 * Declarations of hpet.c
 */
extern int hpet_mode;

void hpet_init(void);


/*
 * Declarations of idt.asm and idt.c
 */
extern volatile uint32_t idt_irq0;

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
extern struct kernel_table *kernel;

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

int memory_init(void *map);
void memory_disable_manager(void);
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
 * Declarations of pg.c
 */
extern volatile uint32_t pg_fault_counter;
extern volatile uint32_t pg_tables_size;

int pg_init(void);
int pg_handler(void);
void pg_map_uncached(void *addr);
void pg_restore(void);


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
 * Declarations of table.c
 */
void table_init(void);


/*
 * Declarations of usb.c
 */
extern unsigned int usb_uhci_count;
extern unsigned int usb_ohci_count;
extern unsigned int usb_ehci_count;
extern unsigned int usb_xhci_count;

int usb_init_early(void);
void usb_init(void);


/*
 * Declarations of vga.c
 */
void vga_set_palette_early(const struct b_video_info *vi);
void vga_set_palette(void);

#endif
