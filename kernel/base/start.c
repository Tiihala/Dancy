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

static const char *optional_names[] = { "_acpios_init" };
#endif

#ifdef DANCY_64
struct kernel_table __dancy_kernel_table;
struct kernel_table *kernel = &__dancy_kernel_table;

static const char *optional_names[] = { "acpios_init" };
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
	int names = (int)(sizeof(optional_names) / sizeof(*optional_names));
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
	checked_init(idt_init, "IDT (BSP)");

	checked_init(pg_init, "Paging (BSP)");
	checked_init(mm_init, "Physical memory manager");
	checked_init(fb_init, "Framebuffer");

	checked_init(con_init, "Console");
	checked_init(irq_init, "IRQ");

	kernel->panic = panic;
	kernel->print = con_print;
	kernel->detach_init_module(&timer_ticks);

	checked_init(task_init, "Task");
	checked_init(runlevel_init, "Runlevel");

	/*
	 * Call all optional init functions, e.g. ACPICA.
	 */
	for (i = 0; i < names; i++) {
		const char *name = optional_names[i];
		int (*func)(void);

		for (j = 0; j < kernel->symbol_count; j++) {
			if (!strcmp(kernel->symbol[j].name, name)) {
				addr_t a = (addr_t)kernel->symbol[j].value;

				func = (int (*)(void))a;
				checked_init(func, name);
				break;
			}
		}
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

	checked_init(pg_init_ap, "Paging (AP)");
	checked_init(task_init_ap, "Task (AP)");

	cpu_halt(0);
}
