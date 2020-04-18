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
#include <dancy/limits.h>
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

	unsigned rtc_century_idx;
	unsigned iapc_boot_arch;
};

struct acpi_information *acpi_get_information(void);


/*
 * Declarations of bitarray.c
 */
struct bitarray {
	unsigned char *data;
	size_t size;
	unsigned state[2];
	size_t written;
	int (*callback)(struct bitarray *b);
};

void bitarray_init(struct bitarray *b, unsigned char *data, size_t size);
void bitarray_callback(struct bitarray *b, int (*func)(struct bitarray *b));
void bitarray_clear(struct bitarray *b);
long bitarray_aligned_fetch(struct bitarray *b, unsigned bits, void **data);
long bitarray_fetch(struct bitarray *b, unsigned bits);
int bitarray_shove(struct bitarray *b, unsigned bits, unsigned val);
int bitarray_written(struct bitarray *b, size_t *written);


/*
 * Declarations of cpu.c
 */
int cpu_test_features(void);
void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b);
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
 * Declarations of db.c
 */
int db_init(struct b_mem *map);
void db_free(void);
int db_read(const char *name, unsigned char **buf, size_t *size);
const char *db_read_error(int retval);


/*
 * Declarations of gui.c
 */
int gui_init(void);
int gui_create_window(const char *name, int x1, int y1, int x2, int y2);
int gui_delete_window(void);
void gui_print(const char *format, ...);
void gui_refresh(void);


/*
 * Declarations of huffman.c
 */
struct huffman {
	unsigned lengths[16];
	unsigned *symbols;
	unsigned completed;
};

int huffman_init(struct huffman *h, unsigned *symbols, int n);
int huffman_fetch(struct huffman *h, struct bitarray *b);
int huffman_table(struct huffman *h, unsigned *table, int n);


/*
 * Declarations of inflate.c
 */
int inflate_uncompress(struct bitarray *b, unsigned char *out, size_t *size);


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
void b_log_close(void);
void b_log(const char *format, ...);


/*
 * Declarations of memory.c
 */
int memory_init(void *map);
void memory_print_map(void (*print)(const char *, ...));


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
 * Declarations of ttf.c
 */
int ttf_create(void **instance);
int ttf_delete(void *ttf);

int ttf_open(void *ttf, size_t size, const void *ttf_file);
int ttf_set_bitmap(void *ttf, size_t size, void *bitmap);
int ttf_render(void *ttf, unsigned int code_point, unsigned int *width);


/*
 * Declarations of vga.c
 */
void vga_set_palette(void);

#endif
