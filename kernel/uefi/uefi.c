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

uint64_t gOutputMode;
uint64_t gOutputMaxMode;
uint64_t gOutputColumns;
uint64_t gOutputRows;

static void log_system_table(void)
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
	u_log("\tTableEntries        %lld\n", st->NumberOfTableEntries);
	u_log("\n");
}

static void query_output_mode(void)
{
	EFI_STATUS s;
	uint64_t mode, max_mode;
	uint64_t rows = 0, columns = 0;

	mode = gSystemTable->ConOut->Mode->Mode;
	max_mode = gSystemTable->ConOut->Mode->MaxMode;

	s = gSystemTable->ConOut->QueryMode(gSystemTable->ConOut,
		mode, &columns, &rows);

	if (s == EFI_SUCCESS) {
		gOutputMode = mode;
		gOutputMaxMode = max_mode;
		gOutputColumns = columns;
		gOutputRows = rows;
	}

	if (!gOutputColumns)
		gOutputColumns = 80;
	if (!gOutputRows)
		gOutputRows = 25;
}

static void wait_until_return(int seconds)
{
	static const char *m = "\rPress any key or wait %2d seconds...";
	EFI_INPUT_KEY key;
	int i;

	/*
	 * Do not allow key strokes immediately. The intention is to
	 * display the message and then accept _new_ key strokes. The
	 * key strokes in the buffer will be discarded.
	 */
	u_print("\n");
	u_print(m, seconds);
	u_stall(1000), seconds--;

	for (i = 0; i < 128; i++) {
		if (u_read_key(&key))
			break;
	}

	for (/* void */; seconds >= 0; seconds--) {
		u_print(m, seconds);
		u_stall(500);
		if (!u_read_key(&key))
			break;
		u_stall(500);
	}
	u_print("\n");
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
	 * Read current console size and clear the screen.
	 */
	query_output_mode();
	u_set_colors(0x07);
	u_clear_screen();

	/*
	 * Initialize log functions.
	 */
	u_log_init();
	u_log(welcome);
	log_system_table();

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
	 * Initialize memory functions and log the current memory map.
	 */
	if (memory_init()) {
		wait_until_return(20);
		return;
	}
	memory_print_map(u_log);

	/*
	 * Initialize Block I/O.
	 */
	if (block_init()) {
		memory_free();
		wait_until_return(20);
		return;
	}

	/*
	 * Read all the databases, config, and in_x64.at.
	 */
	if (file_init() || file_read_all()) {
		memory_free();
		wait_until_return(20);
		return;
	}

	/*
	 * Temporary code for testing purposes.
	 */
	{
		u_log_dump();
	}

	/*
	 * Show the video mode selection menu.
	 */
	video_show_menu();

	/*
	 * Start the in_x64.at executable.
	 */
	u_print("\nStarting the in_x64.at executable...\n\n");
	memory_export_map(0);
	syscall_init(in_x64_syscalls);
	syscall_jump(memory_in_x64[0]);

	/*
	 * This should not happen.
	 */
	u_print("Error: could not start in_x64.at\n");
	memory_free();
	wait_until_return(20);
}
