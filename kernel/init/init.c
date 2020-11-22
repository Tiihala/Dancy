/*
 * Copyright (c) 2018, 2019, 2020 Antti Tiihala
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
 * init/init.c
 *      Initialization of Dancy Operating System
 */

#include <init.h>

void *ttf;

void init(void)
{
	const uint32_t log_mem = 0x00080000;
	struct b_video_info vi;
	struct b_time bt;

	if (b_log_init(log_mem))
		return;

	b_log("Init Module (Dancy Operating System)\n\n");

	memory_print_map(b_log);

	if (cpu_test_features())
		return;

	/*
	 * Read ACPI information. For older computers, the ACPI
	 * information is not necessarily available.
	 */
	if (!acpi_get_information()) {
		if (boot_loader_type != BOOT_LOADER_TYPE_BIOS) {
			b_print("Error: acpi_get_information\n");
			return;
		}
	}

	if (rtc_read(&bt)) {
		/*
		 * This should not be likely. However, the operating system
		 * does not stop because it must handle wrong time and date
		 * values anyway. If nothing is returned, it is just treated
		 * like other obviously wrong values. Print a warning.
		 */
		b_print("Warning: reading Real Time Clock (RTC) failed\n");
		b_pause();
	}

	if (b_get_structure(&vi, B_VIDEO_INFO) != 0 && vi.width != 0) {
		static const char *mode_names[8] = {
			"4 VGA",
			"8 PALETTE",
			"15",
			"16",
			"24 RGB",
			"24 BGR",
			"32 RGBX",
			"32 BGRX"
		};
		unsigned i = (unsigned)vi.mode;
		const char *mode_name = (i < 8) ? mode_names[i] : "";

		b_log("Video Mode\n");
		b_log("\tMode: %ux%ux%s\n", vi.width, vi.height, mode_name);
		b_log("\n");

		/*
		 * The boot loader guarantees that VGA registers can be
		 * accessed if mode is B_MODE_VGA or B_MODE_PALETTE.
		 */
		if (vi.mode == B_MODE_VGA || vi.mode == B_MODE_PALETTE)
			vga_set_palette(&vi);
	}

	/*
	 * Load dancy.ttf and create a ttf object.
	 */
	{
		static const char *ttf_name = "system/fonts/dancy.ttf";
		unsigned char *ttf_data;
		size_t ttf_size;

		int r = db_read(ttf_name, &ttf_data, &ttf_size);

		if (r != 0) {
			b_print("%s error: %s\n", ttf_name, db_read_error(r));
			return;
		}

		if (ttf_create(&ttf)) {
			b_print("Error: ttf_create\n");
			return;
		}

		if (ttf_open(ttf, ttf_size, ttf_data)) {
			b_print("Error: ttf_open (%s)\n", ttf_name);
			return;
		}

		free(ttf_data);
	}

	/*
	 * Modify the boot file system.
	 */
	if (!fs_init()) {
		if (!fs_update_config_at(&vi))
			fs_write_logs();
		fs_free();
	} else {
		b_print("Error: fs_init\n");
		b_pause();
	}

	b_log_free();

	/*
	 * Initialize graphical user interface while boot services
	 * are still available. The actual GUI does not require them.
	 */
	if (gui_init()) {
		b_print("*****************************************\n");
		b_print("**** Graphical Mode is not supported ****\n");
		b_print("*****************************************\n\n");
	}

	/*
	 * Create the first GUI window.
	 */
	gui_create_window("Dancy Operating System", 20, 20, 620, 460);

	/*
	 * Use gui_print instead of the loader provided service.
	 */
	b_print = gui_print;

	/*
	 * Terminate all boot loader services. If using the UEFI boot
	 * loader, this will internally call
	 *
	 *     EFI_BOOT_SERVICES.ExitBootServices()
	 *
	 * After b_exit, the PICs (if available) have been programmed
	 * so that IRQs start from vector 32 and all of them have been
	 * disabled. The boot processor is at ring 0. Other processors
	 * have not been configured by the boot loader.
	 */
	b_exit();

	/*
	 * Load the Global Descriptor Table.
	 */
	gdt_init();

	/*
	 * Initialize the interrupt management.
	 */
	idt_init();

	/*
	 * Enable new paging tables.
	 */
	if (pg_init()) {
		idt_load_null();
		return;
	}

	/*
	 * Refresh the GUI. Otherwise panic() may fail later because the
	 * framebuffer would not be identity mapped. Normally page faults
	 * will map the pages but panic() does not allow exceptions.
	 */
	gui_refresh();

	/*
	 * Initialize Local Advanced Programmable Interrupt Controller.
	 */
	if (apic_init()) {
		const struct acpi_information *acpi = acpi_get_information();

		/*
		 * On SMP computers, the APIC initialization must work.
		 */
		if (acpi != NULL && acpi->num_cpu_core > 1)
			panic("APIC initialization failed");
	}

	/*
	 * Initialize I/O Advanced Programmable Interrupt Controller(s).
	 */
	ioapic_init();

	/*
	 * Initialize High Precision Event Timer (HPET).
	 */
	hpet_init();

	/*
	 * Initialize Programmable Interval Timer ("8254" PIT) if HPET is
	 * not available. The computer should have the PIT in that case.
	 */
	if (!hpet_mode)
		pit_init();

	/*
	 * Enable interrupts.
	 */
	cpu_ints(1);

	/*
	 * Allow IRQ 0 and IRQ 8.
	 */
	if (apic_mode) {
		ioapic_enable(0);
		ioapic_enable(8);
	} else {
		cpu_out8(0xA1, 0xFE);
		cpu_out8(0x21, 0xFA);
	}

	/*
	 * Wait for the delay function calibration.
	 */
	while (delay_ready == 0) {
		if (delay_error != 0) {
			const char *e = "Error: delay calibration";
			char msg[64];

			snprintf(&msg[0], 64, "%s (%d)", e, delay_error);
			panic(&msg[0]);
		}
	}

	/*
	 * Enable Symmetric Multiprocessing (SMP).
	 */
	smp_init();

	/*
	 * Temporary code for testing purposes.
	 */
	{
		int x1, y1, x2, y2;
		unsigned prev = 0, next;

		x1 = 40;
		y1 = 80;
		x2 = x1 + 320;
		y2 = y1 + 200;

		gui_create_window("Dancy Operating System", x1, y1, x2, y2);
		gui_print("https://github.com/Tiihala/Dancy\n\n");
		gui_refresh();

		if (hpet_mode)
			gui_print("Using High Precision Event Timer\n");

		gui_print("Using %s\n\n",
			(apic_mode == 0) ? "PIC 8259" : "I/O APIC");

		gui_print("TSC: %4u.%03u Mhz\n\n",
			(unsigned)(delay_tsc_hz / 1000000),
			(unsigned)((delay_tsc_hz % 1000000) / 1000));

		for (next = 0; next <= 10; next++) {
			unsigned delay_calls = 10;

			while (delay_calls--)
				delay(100000000);

			gui_print("\rDelay: %u/10", next);
			gui_refresh();
		}

		gui_print("\rNumber of APs started: %u\n\n", smp_ap_count);

		for (;;) {
			while (prev == (next = idt_irq0 / 1000))
				cpu_halt(1);

			gui_print("\rUptime (seconds): %u", next);
			gui_refresh();

			prev = next;
		}
	}
}

volatile uint32_t init_ap_error = 0;

void init_ap(uint32_t id)
{
	/*
	 * Load the Global Descriptor Table.
	 */
	gdt_init();

	/*
	 * Load the Interrupt Descriptor Table.
	 */
	idt_restore();

	/*
	 * Initialize Local Advanced Programmable Interrupt Controller.
	 */
	if (apic_init()) {
		cpu_write32((uint32_t *)&init_ap_error, 1);
		cpu_halt(0);
	}

	/*
	 * Enable interrupts.
	 */
	cpu_ints(1);

	/*
	 * Nothing to do at the moment.
	 */
	(void)id;
	cpu_halt(0);
}

DANCY_SYMBOL(init);
