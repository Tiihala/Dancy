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
 * base/start.c
 *      Kernel start functions
 */

#include <dancy.h>

#ifdef DANCY_32
struct kernel_table _dancy_kernel_table;
struct kernel_table *kernel = &_dancy_kernel_table;
#define SYMBOL_PREFIX "_"
#endif

#ifdef DANCY_64
struct kernel_table __dancy_kernel_table;
struct kernel_table *kernel = &__dancy_kernel_table;
#define SYMBOL_PREFIX ""
#endif

static struct {
	int optional; addr_t addr; const char *name; const char *desc;
} dynamic_init[] = {
	/*
	 * Dynamically linked initialization functions.
	 */
	{ 0, 0, SYMBOL_PREFIX "rtc_init", "Real-Time Clock" },
	{ 0, 0, SYMBOL_PREFIX "epoch_init", "Epoch Functions" },
	{ 0, 0, SYMBOL_PREFIX "sched_init", "Scheduler" },
	{ 0, 0, SYMBOL_PREFIX "vfs_init", "Virtual File System" },
	{ 0, 0, SYMBOL_PREFIX "bin_init", "Bin Mount" },
	{ 0, 0, SYMBOL_PREFIX "devfs_init", "Devfs Mount" },
	{ 0, 0, SYMBOL_PREFIX "zero_init", "Devfs Zero" },
	{ 0, 0, SYMBOL_PREFIX "console_init", "Devfs Console" },
	{ 0, 0, SYMBOL_PREFIX "dma_init", "DMA" },
	{ 0, 0, SYMBOL_PREFIX "floppy_init", "Floppy" },
	{ 0, 0, SYMBOL_PREFIX "serial_init", "Serial Ports" },
	{ 0, 0, SYMBOL_PREFIX "pci_init", "PCI" },
	{ 0, 0, SYMBOL_PREFIX "ps2_init", "PS/2 Controller" },
	{ 0, 0, SYMBOL_PREFIX "hdd_part_init", "Disk Partitions" },
	{ 0, 0, SYMBOL_PREFIX "hdd_fat_init", "FAT Mount" },
	{ 0, 0, SYMBOL_PREFIX "file_init", "File" },
	{ 0, 0, SYMBOL_PREFIX "syscall_init", "System Calls" },
	{ 1, 0, SYMBOL_PREFIX "acpios_init", "ACPICA" },
	{ 1, 0, SYMBOL_PREFIX "debug_init", "Debug" }
};

static int ap_count;
static int ap_lock;

static void checked_init(int (*func)(void), const char *desc)
{
	int r = func();

	if (r) {
		char msg[128];

		snprintf(&msg[0], 128, "%s: %s (%d)", desc, strerror(r), r);
		kernel->panic(msg);
	}
}

static uint32_t empty_yield_count;

static void empty_yield(void)
{
	cpu_add32(&empty_yield_count, 1);
}

void kernel_start(void)
{
	static int run_once;
	int funcs = (int)(sizeof(dynamic_init) / sizeof(*dynamic_init));
	int i, j;

	if (!spin_trylock(&run_once))
		return;

	/*
	 * The kernel table structure is set by the init module. The size
	 * of the structure is like a magic number, and it gives additional
	 * certainty that both init and kernel use the same structure.
	 */
	if (cpu_read32((uint32_t *)&kernel->table_size) != sizeof(*kernel))
		return;

	/*
	 * The BSP calls this kernel_start function after all application
	 * processors have called the kernel_start_ap function. Check that
	 * everything really worked as intended.
	 */
	if ((int)cpu_read32((uint32_t *)&ap_count) != kernel->smp_ap_count)
		kernel->panic("SMP: kernel synchronization failure");

	kernel->scheduler.yield = empty_yield;

	checked_init(heap_init, "Heap memory manager");
	checked_init(gdt_init, "GDT (BSP)");
	checked_init(idt_init, "IDT (BSP)");

	checked_init(pg_init, "Paging (BSP)");
	checked_init(fb_init, "Framebuffer");

	checked_init(con_init, "Console");
	checked_init(irq_init, "IRQ");

	kernel->panic = panic;
	kernel->print = con_print;
	kernel->detach_init_module(&timer_ticks);

	checked_init(mm_init, "Physical memory manager");
	checked_init(task_init, "Task");
	checked_init(runlevel_init, "Runlevel");

	/*
	 * Find all dynamically linked initialization functions.
	 */
	for (i = 0; i < funcs; i++) {
		const char *name = dynamic_init[i].name;

		for (j = 0; j < kernel->symbol_count; j++) {
			if (!strcmp(kernel->symbol[j].name, name)) {
				addr_t addr = (addr_t)kernel->symbol[j].value;
				dynamic_init[i].addr = addr;
				break;
			}
		}
	}

	/*
	 * Call the initialization functions in a defined order.
	 */
	for (i = 0; i < funcs; i++) {
		addr_t addr = dynamic_init[i].addr;
		int (*func)(void);

		if (addr) {
			func = (int (*)(void))addr;
			checked_init(func, dynamic_init[i].desc);

		} else if (!dynamic_init[i].optional) {
			char msg[128];

			snprintf(&msg[0], 128, "%s (%s): symbol not found",
				dynamic_init[i].name, dynamic_init[i].desc);
			kernel->panic(msg);
		}
	}

	while (panic_lock == 0)
		task_wait_descendant(NULL, NULL);

	cpu_halt(0);
}

void kernel_start_ap(void)
{
	/*
	 * The kernel->smp_ap_count member defines how many times this
	 * kernel_start_ap function can be called.
	 */
	{
		spin_lock(&ap_lock);

		if (ap_count >= kernel->smp_ap_count) {
			spin_unlock(&ap_lock);
			return;
		}

		ap_count += 1;

		spin_unlock(&ap_lock);
	}

	checked_init(gdt_init_ap, "GDT (AP)");
	checked_init(idt_init_ap, "IDT (AP)");

	checked_init(pg_init_ap, "Paging (AP)");
	checked_init(task_init_ap, "Task (AP)");

	task_exit(0);
}
