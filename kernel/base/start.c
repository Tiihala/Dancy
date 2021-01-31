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
#endif

#ifdef DANCY_64
struct kernel_table __dancy_kernel_table;
struct kernel_table *kernel = &__dancy_kernel_table;
#endif

static int ap_count;
static int ap_lock;

static void checked_init(int (*func)(void), const char *desc)
{
	int r = func();

	if (r) {
		char msg[128];

		snprintf(&msg[0], 128, "%s: init error code %d", desc, r);
		kernel->panic(msg);
	}
}

void kernel_start(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return;

	/*
	 * The kernel table structure is set by the init module. The size
	 * of the structure is like a magic number, and it gives additional
	 * certainty that both init and kernel use the same structure.
	 */
	if (cpu_read32((uint32_t *)&kernel->table_size) != sizeof(*kernel))
		return;

	kernel->print("Welcome to Dancy Operating System\n");

	/*
	 * The BSP calls this kernel_start function after all application
	 * processors have called the kernel_start_ap function. Check that
	 * everything really worked as intended.
	 */
	if ((int)cpu_read32((uint32_t *)&ap_count) != kernel->smp_ap_count)
		kernel->panic("SMP: kernel synchronization failure");

	checked_init(heap_init, "Heap memory manager");
	checked_init(gdt_init, "GDT (BSP)");

	/*
	 * Initialize the interrupt descriptor table.
	 */
	{
		const int irq0_apic = 64;
		const int irq0_pic = 32;
		uint32_t a, *p;

		int r = cpu_ints(0);

		checked_init(idt_init, "IDT (BSP)");

		if (kernel->io_apic_enabled) {
			a = (uint32_t)kernel->apic_base_addr;
			p = (uint32_t *)((addr_t)&timer_apic_base[0]);
			if (cpu_read32(p))
				kernel->panic("timer_apic_base: error 1");
			cpu_write32(p, a);
#ifdef DANCY_64
			a = (uint32_t)(kernel->apic_base_addr >> 32);
			p = (uint32_t *)((addr_t)&timer_apic_base[4]);
			if (cpu_read32(p))
				kernel->panic("timer_apic_base: error 2");
			cpu_write32(p, a);
#endif
			idt_install_asm(irq0_apic, timer_asm_handler_apic);
		} else {
			idt_install_asm(irq0_pic, timer_asm_handler_pic);
		}

		cpu_ints(r);
	}

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

	cpu_halt(0);
}
