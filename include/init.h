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
#include <dancy/boot.h>
#include <dancy/crc.h>
#include <dancy/ctype.h>
#include <dancy/keys.h>
#include <dancy/lib.h>
#include <dancy/limits.h>
#include <dancy/palette.h>
#include <dancy/stdarg.h>
#include <dancy/stdio.h>
#include <dancy/stdlib.h>
#include <dancy/string.h>
#include <dancy/symbol.h>
#include <dancy/types.h>

enum dancy_error {

#define DANCY_X(a, b) a,
#include <dancy/error.h>
#undef DANCY_X

	dancy_error_end
};

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

	unsigned rtc_century_idx;
	unsigned iapc_boot_arch;

	phys_addr_t local_apic_base;
	int dual_8259_setup;

	unsigned num_cpu_core;
	unsigned num_io_apic;

	unsigned max_apic_id;
	unsigned max_io_apic_id;
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
 * Declarations of cpu.asm and cpu.c
 */
int cpu_test_features(void);
void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b);
void cpu_halt(uint32_t counter);
int cpu_ints(int enable);

void cpu_rdtsc(uint32_t *a, uint32_t *d);
void cpu_rdtsc_delay(uint32_t a, uint32_t d);
void cpu_rdtsc_diff(uint32_t *a, uint32_t *d);

uint8_t cpu_in8(uint16_t port);
uint16_t cpu_in16(uint16_t port);
uint32_t cpu_in32(uint16_t port);

void cpu_out8(uint16_t port, uint8_t value);
void cpu_out16(uint16_t port, uint16_t value);
void cpu_out32(uint16_t port, uint32_t value);


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
int gui_init(void);
int gui_create_window(const char *name, int x1, int y1, int x2, int y2);
int gui_delete_window(void);
int gui_draw(unsigned char *png, size_t size, int x1, int y1, int x2, int y2);
int gui_move_window(int x, int y);
void gui_print(const char *format, ...);
void gui_print_alert(const char *message);
void gui_refresh(void);


/*
 * Declarations of idt.asm and idt.c
 */
void idt_init(void);
void idt_handler(unsigned num, unsigned err_code, const void *stack);
void idt_load_null(void);
void idt_restore(void);


/*
 * Declarations of init.c
 */
void init(void);


/*
 * Declarations of ld.asm and ld.c
 */
struct global_symbol {
	uint32_t value;
	char name[28];
};

int ld_init(size_t symbols);
int ld_add(const struct global_symbol *symbol);
int ld_find(const char *name, struct global_symbol **symbol);
void ld_free(void);
int ld_link(const char *name, unsigned char *obj);
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
void memory_print_map(void (*print)(const char *, ...));


/*
 * Declarations of pg.asm and pg.c
 */
extern volatile uint32_t pg_fault_counter;
extern volatile uint32_t pg_tables_size;

int pg_init(void);
int pg_handler(void);
void pg_get_current(phys_addr_t *addr);
void pg_get_fault(phys_addr_t *addr);
void pg_switch(phys_addr_t addr);


/*
 * Declarations of print.c
 */
void b_print(const char *format, ...);


/*
 * Declarations of rtc.c
 */
int rtc_read(struct b_time *bt);


/*
 * Declarations of start.c
 */
void start_init(void *map);


/*
 * Declarations of vga.c
 */
void vga_set_palette(const struct b_video_info *vi);

#endif
