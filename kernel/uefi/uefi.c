/*
 * Copyright (c) 2019 Antti Tiihala
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
 * uefi/uefi.c
 *      Boot Loader (UEFI)
 */

#include <uefi.h>

static const char *welcome =
	"UEFI Boot Loader (Dancy Operating System)\n"
	"Copyright (c) 2019 Antti Tiihala\n\n";

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gSystemTable;
void *gBaseAddress;
void *gOriginalRsp;

uint64_t gOutputColumns = 80;
uint64_t gOutputRows = 25;

static void boot_error(void)
{
	static const char *m1 = "\n*** Press F1 to view the boot log ***\n\n";
	static const char *m2 = "\rPress other keys or wait %2d seconds...";
	int show_log = 0;
	int seconds = 20;
	EFI_INPUT_KEY key;
	int i;

	u_print(m1);
	u_stall(2000);

	/*
	 * Do not allow key strokes immediately. The intention is to
	 * display the message and then accept _new_ key strokes. The
	 * key strokes in the buffer will be discarded.
	 */
	for (i = 0; i < 128; i++) {
		if (u_read_key(&key))
			break;
	}

	for (/* void */; seconds >= 0; seconds--) {
		u_print(m2, seconds);
		u_stall(500);
		if (!u_read_key(&key)) {
			if (key.ScanCode == 0x0B)
				show_log = 1;
			break;
		}
		u_stall(500);
	}
	u_print("\n");

	if (show_log) {
		u_log_dump();
		u_stall(2000);
	}

	/*
	 * Free allocated memory (see memory.c). It is safe to run this
	 * function even if the memory_init itself has failed.
	 */
	memory_free();
}

void uefi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, ...)
{
	va_list va;
	va_start(va, SystemTable);
	gImageHandle = ImageHandle;
	gSystemTable = SystemTable;
	gBaseAddress = va_arg(va, void *);
	gOriginalRsp = va_arg(va, void *);
	va_end(va);

	if (((addr_t)gBaseAddress & 4095) + 495 - 'D' - 'a' - 'n' - 'c' - 'y')
		return;

	/*
	 * Disable the watchdog timer.
	 */
	gSystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

	/*
	 * Read current console size and clear the screen.
	 */
	{
		EFI_STATUS s;
		uint64_t rows = 0, columns = 0;

		s = gSystemTable->ConOut->QueryMode(gSystemTable->ConOut,
			gSystemTable->ConOut->Mode->Mode, &columns, &rows);

		if (s == EFI_SUCCESS) {
			gOutputColumns = columns;
			gOutputRows = rows;
		}

		u_set_colors(0x07);
		u_clear_screen();
	}

	/*
	 * Initialize log functions.
	 */
	u_log_init();
	u_log(welcome);

	/*
	 * Log some information from the system table.
	 */
	{
		EFI_SYSTEM_TABLE *st = gSystemTable;

		u_log("EFI System Table\n");
		u_log("\tHdr.Signature       %016llX\n", st->Hdr.Signature);
		u_log("\tHdr.Revision        %08X\n", st->Hdr.Revision);
		u_log("\tHdr.HeaderSize      %08X\n", st->Hdr.HeaderSize);

		{
			const uint16_t *vendor = st->FirmwareVendor;
			char buf[32];
			size_t i;

			for (i = 0; i < sizeof(buf) - 1; i++) {
				uint16_t w = *vendor++;
				if (w == 0x00)
					break;
				if (w >= 0x20 && w < 0x7F)
					buf[i] = (char)w;
				else
					buf[i] = '?';
			}
			buf[i] = '\0';
			u_log("\tFirmwareVendor      %s\n", &buf[0]);
		}

		u_log("\tFirmwareRevision    %08X\n", st->FirmwareRevision);
		u_log("\n");
	}

	/*
	 * Initialize video functions and print a welcome message.
	 */
	video_init();
	u_print(welcome);

	/*
	 * Initialize Serial I/O.
	 */
	serial_init();

	/*
	 * Initialize Simple Text Input Ex.
	 */
	key_init();

	/*
	 * Initialize memory functions.
	 */
	if (memory_init()) {
		boot_error();
		return;
	}

	/*
	 * Log the current memory map.
	 */
	memory_print_map(u_log);

	/*
	 * Initialize Block I/O.
	 */
	if (block_init()) {
		boot_error();
		return;
	}

	/*
	 * Read all the databases, config, and in_x64.at.
	 */
	if (file_init() || file_read_all()) {
		boot_error();
		return;
	}

	/*
	 * Show the video mode selection menu.
	 */
	video_show_menu();

	/*
	 * Start the in_x64.at executable. The boot loader should not
	 * return control back to the UEFI firmware anymore, although
	 * the ExitBootServices() function is not called yet.
	 */
	memory_export_map(0);
	syscall_init(in_x64_syscalls);
	syscall_jump(memory_in_x64[0]);

	/*
	 * This should not happen. The "syscall_jump" must not return
	 * unless there are serious problems (e.g. memory corruption).
	 */
	boot_error();
}
