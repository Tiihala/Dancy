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
 * kernel/table.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_TABLE_H
#define KERNEL_TABLE_H

#include <misc/types.h>

struct kernel_table {
	size_t table_size;

	addr_t base_addr;
	addr_t heap_addr;

	addr_t stack_array_addr;
	size_t stack_array_size;

	void (*detach_init_module)(volatile uint32_t *ticks);
	void (*panic)(const char *message);
	void (*print)(const char *format, ...);

	addr_t arctic_bin_addr;
	size_t arctic_bin_size;

	addr_t arctic_root_addr;
	size_t arctic_root_size;

	/*
	 * Dynamically linked modules (including the kernel itself).
	 */
	int module_count;

	struct {
		addr_t text_addr;
		size_t text_size;

		addr_t rdata_addr;
		size_t rdata_size;

		addr_t data_addr;
		size_t data_size;

		addr_t bss_addr;
		size_t bss_size;

		char name[16];
	} *module;

	/*
	 * Global symbol table.
	 */
	int symbol_count;

	struct {
		uint32_t value;
		char name[36];
	} *symbol;

	/*
	 * Symmetric Multiprocessing (SMP).
	 *
	 * The smp_ap_state table uses AP ID as an index.
	 */
	int smp_ap_count;
	uint32_t *smp_ap_id;
	volatile uint8_t *smp_ap_state;

	/*
	 * Advanced Configuration and Power Interface (ACPI).
	 */
	int acpi_enabled;

	struct {
		phys_addr_t rsdp_addr;
		phys_addr_t rsdt_addr;
		phys_addr_t xsdt_addr;
		phys_addr_t fadt_addr;
		phys_addr_t madt_addr;
		phys_addr_t mcfg_addr;
		phys_addr_t hpet_addr;

		unsigned int rtc_century_idx;
		unsigned int iapc_boot_arch;
	} *acpi;

	int acpica_available;
	int acpica_enabled;

	/*
	 * Advanced Programmable Interrupt Controller (APIC).
	 */
	int apic_enabled;
	phys_addr_t apic_base_addr;
	addr_t apic_base_vaddr;

	uint32_t apic_bsp_id;

	/*
	 * I/O Advanced Programmable Interrupt Controller (I/O APIC).
	 */
	int io_apic_enabled;
	int io_apic_count;

	struct {
		uint32_t id;
		uint32_t base_int;
		phys_addr_t addr;
	} *io_apic;

	struct {
		uint32_t global_int;
		uint32_t flags;
	} io_apic_override[16];

	/*
	 * Supported processor features.
	 */
	struct {
		int osfxr;
		int nxbit;
		int gpage;
		int rdtscp;
	} cpu_feature;

	/*
	 * Time-Stamp Counter (TSC).
	 */
	uint64_t delay_tsc_hz;

	/*
	 * Kernel messages (KMSG).
	 */
	struct {
		char *buffer;
		size_t size;
		int state;
		int lock;

		unsigned long long counter;
		event_t event;
	} kmsg;

	/*
	 * Framebuffer set by system firmware.
	 */
	uint32_t fb_mode;
	uint32_t fb_width;
	uint32_t fb_height;
	uint32_t fb_stride;
	phys_addr_t fb_addr;

	/*
	 * Standard framebuffer with 4-byte pixels.
	 */
	addr_t fb_standard_addr;
	size_t fb_standard_size;

	/*
	 * Kernel console dimensions.
	 */
	int con_columns;
	int con_rows;

	/*
	 * Kernel console bitmap glyphs.
	 */
	int glyph_count;
	int glyph_width;
	int glyph_height;

	struct {
		int unicode_count;
		uint32_t unicode;
		void *data;
	} *glyph;

	/*
	 * System memory map.
	 */
	size_t memory_map_size;

	struct {
		uint32_t type;
		uint32_t reserved;
		uint64_t base;
		uint64_t efi_attributes;
	} *memory_map;

	/*
	 * Peripheral Component Interconnect (PCI).
	 */
	int pci_device_count;

	struct pci_id {
		int group;
		int bus;
		int device;
		int func;

		phys_addr_t ecam;
		uint32_t vendor_id;
		uint32_t device_id;
		uint32_t class_code;

		int cap_msi;
		int cap_msi_x;
	} *pci_device;

	/*
	 * Task scheduler interface.
	 */
	struct {
		void (*yield)(void);

		int *task_lock;
		void *task_head;
	} scheduler;

	/*
	 * System date and time functions.
	 */
	unsigned long long (*epoch_read)(void);
	unsigned long long (*epoch_read_ms)(void);
	void (*epoch_sync)(void);

	/*
	 * A flag, which is set if the system is going down.
	 */
	int rebooting;

	/*
	 * Events signaled by special key combinations.
	 */
	struct {
		event_t ctrl_alt_del_event;
		uint32_t ctrl_alt_del_data;

		event_t console_switch_event;
		uint32_t console_switch_data;
	} keyboard;
};

#endif
