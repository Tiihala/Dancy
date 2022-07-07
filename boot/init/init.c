/*
 * Copyright (c) 2018, 2019, 2020, 2021 Antti Tiihala
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

#include <boot/init.h>

void *ttf;
void *ttf_array[3];

unsigned char *arctic_bin_data;
size_t arctic_bin_size;

static const char *ttf_names[3] = {
	"share/fonts/dcysan.ttf",
	"share/fonts/dcysanb.ttf",
	"share/fonts/dcysanm.ttf"
};

void init(void)
{
	const uint32_t log_mem = 0x00080000;
	struct b_video_info vi;
	struct b_time bt;
	int i;

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
		unsigned idx = (unsigned)vi.mode;
		const char *mode_name = (idx < 8) ? mode_names[idx] : "";

		b_log("Video Mode\n");
		b_log("\tMode: %ux%ux%s\n", vi.width, vi.height, mode_name);
		b_log("\n");

		/*
		 * The boot loader guarantees that VGA registers can be
		 * accessed if mode is B_MODE_VGA or B_MODE_PALETTE.
		 */
		if (vi.mode == B_MODE_VGA || vi.mode == B_MODE_PALETTE)
			vga_set_palette_early(&vi);
	}

	/*
	 * Initialize PCI (early).
	 */
	if (pci_init_early())
		return;

	/*
	 * Initialize USB (early).
	 */
	if (usb_init_early())
		return;

	/*
	 * Load the file system image for native binaries.
	 */
	{
#if DANCY_32
		const char *bin_name = "arctic/bin32.img";
#else
		const char *bin_name = "arctic/bin64.img";
#endif
		int r = db_read(bin_name, &arctic_bin_data, &arctic_bin_size);

		if (r != 0) {
			b_print("%s error: %s\n", bin_name, db_read_error(r));
			return;
		}
	}

	/*
	 * Load fonts and create ttf objects.
	 */
	for (i = 0; i < (int)(sizeof(ttf_array) / sizeof(*ttf_array)); i++) {
		const char *ttf_name = ttf_names[i];
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

		ttf_array[i] = ttf;
	}

	/*
	 * Use the first ttf object.
	 */
	ttf = ttf_array[0];

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
		const char *message =
			"*************************************\n"
			"** Graphical mode is not supported **\n"
			"*************************************\n\n"
			"The operating system does not use any\n"
			"graphics without a video framebuffer.\n\n";

		const char *vertical_bar = "\xE2\x94\x82";
		const char *empty_space = " ";

		if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
			vertical_bar = "|";

		b_output_control(B_CLEAR_CONSOLE, 0);
		b_print(message);

		b_print("%33s" "%s" "\r" "%3s" "%s",
			empty_space, vertical_bar,
			empty_space, vertical_bar);

		for (i = 0; i < 29; i++) {
			b_output_string_hl(empty_space, 1);
			b_pause();
		}

		b_output_control(B_CLEAR_CONSOLE, 0);
	}

	/*
	 * Draw the background picture.
	 */
	if (vi.width != 0) {
		const char *png_name = "share/pictures/init.png";
		unsigned char *png_data;
		size_t png_size;

		int x1 = 0, y1 = 0;
		int x2 = (int)(vi.width - 1), y2 = (int)(vi.height - 1);

		if (!db_read(png_name, &png_data, &png_size)) {
			gui_draw(png_data, png_size, x1, y1, x2, y2);
			free(png_data);
		}
	}

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
	 * The boot service termination may have modified the memory map.
	 */
	if (memory_init(memory_map)) {
		gui_refresh();
		cpu_halt(0);
	}

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
	 * Initialize control registers.
	 */
	cpu_init_control_registers();

	/*
	 * Refresh the GUI. Otherwise panic() may fail later because the
	 * framebuffer would not be identity mapped. Normally page faults
	 * will map the pages but panic() does not allow exceptions.
	 */
	gui_refresh();

	/*
	 * Finalize the color palette if using the 8-bit mode.
	 */
	if (vi.width != 0 && vi.mode == B_MODE_PALETTE)
		vga_set_palette();

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
	 * Allow IRQ 0, and IRQ 2 if using I/O APIC. For the latter, the
	 * order is very relevant, i.e. IRQ 2 must be enabled first. After
	 * that, IRQ 0 usually overrides its entry on the I/O APIC's table.
	 */
	if (apic_mode)
		ioapic_enable(2), ioapic_enable(0);
	else
		cpu_out8(0x21, 0xFA);

	/*
	 * Enable interrupts.
	 */
	cpu_ints(1);

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
		cpu_halt(1);
	}

	/*
	 * Enable Symmetric Multiprocessing (SMP).
	 */
	smp_init();

	/*
	 * Initialize PCI.
	 */
	pci_init();

	/*
	 * Initialize USB.
	 */
	usb_init();

	/*
	 * Print messages before initializing the kernel. This will create
	 * a GUI window if not already created.
	 */
	{
		static const char *months[13] = {
			"???", "Jan", "Feb", "Mar", "Apr",
			"May", "Jun", "Jul", "Aug", "Sep",
			"Oct", "Nov", "Dec"
		};
		int m = (bt.month <= 12) ? (int)bt.month : 0;
		unsigned int mhz = (unsigned)(delay_tsc_hz / 1000000);
		unsigned int usb_controllers = 0;
		int dancy_bit = (int)(sizeof(void *)) * 8;

		/*
		 * If starting the message with '\b', the blit function
		 * is not called and output is not immediately drawn.
		 */
		b_print("\bStarted on %u %s %u %02u:%02u\n",
			(unsigned)bt.day, months[m], (unsigned)bt.year,
			(unsigned)bt.hour, (unsigned)bt.minute);

		ttf = ttf_array[1];
		b_print("\b\nSystem Summary\n");
		ttf = ttf_array[0];

		if (!smp_ap_count)
			b_print("\b  Uniprocessor configuration\n");
		else
			b_print("\b  Symmetric multiprocessing (%u units)\n",
				(unsigned int)(smp_ap_count + 1));

		b_print("\b  Time-Stamp Counter (%u Mhz)\n", mhz);

		if (hpet_mode)
			b_print("\b  High Precision Event Timer (HPET)\n");

		if (pci_device_count > 1) {
			const char *ecam = (pci_devices[0].ecam != 0) ?
				"ECAM" : "Mechanism #1";

			b_print("\b  Found %u PCI devices (%s)\n",
				(unsigned)pci_device_count, ecam);
		}

		usb_controllers += usb_uhci_count;
		usb_controllers += usb_ohci_count;
		usb_controllers += usb_ehci_count;
		usb_controllers += usb_xhci_count;

		if (usb_controllers) {
			int printed_flag = 0;

			b_print("\b  Initialized ");

			if (usb_uhci_count) {
				b_print("\b%u UHCI", usb_uhci_count);
				printed_flag = 1;
			}

			if (usb_ohci_count) {
				b_print("\b%s%u OHCI",
					((printed_flag != 0) ? ", " : ""),
					usb_ohci_count);
				printed_flag = 1;
			}

			if (usb_ehci_count) {
				b_print("\b%s%u EHCI",
					((printed_flag != 0) ? ", " : ""),
					usb_ehci_count);
				printed_flag = 1;
			}

			if (usb_xhci_count) {
				b_print("\b%s%u xHCI",
					((printed_flag != 0) ? ", " : ""),
					usb_xhci_count);
			}

			b_print("\b device%s\n",
				(usb_controllers > 1) ? "s" : "");
		}

		ttf = ttf_array[1];
		b_print("\b\nSoftware Environment\n");
		ttf = ttf_array[0];

		b_print("  Dancy Operating System %i.%i (%i-bit)\n"
			"  Written by Antti Tiihala\n\n\n",
			DANCY_MAJOR, DANCY_MINOR, dancy_bit);
	}

	/*
	 * Initialize the kernel.
	 */
	kernel_init();

	cpu_halt(0);
}

void init_ap(uint32_t id)
{
	/*
	 * Initialize Local Advanced Programmable Interrupt Controller.
	 */
	if (apic_init())
		panic("APIC initialization failed (application processor)");

	/*
	 * Initialize control registers.
	 */
	cpu_init_control_registers();

	/*
	 * Enable interrupts.
	 */
	cpu_ints(1);

	/*
	 * Enable the APIC periodic timer and have a few halt instructions
	 * for testing purposes. Interrupts must wake up the halted CPU.
	 */
	apic_start_timer(), cpu_halt(4);

	/*
	 * Initialize the kernel (application processors).
	 */
	kernel_init_ap(id);

	cpu_halt(0);
}

DANCY_SYMBOL(init);
