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

static void log_global_variables(void)
{
	u_log("Global Variables\n");
	u_log("\tgImageHandle        %p\n", gImageHandle);
	u_log("\tgSystemTable        %p\n", gSystemTable);
	u_log("\tgBaseAddress        %p\n", gBaseAddress);
	u_log("\tgOriginalRsp        %p\n", gOriginalRsp);
	u_log("\tgOutputMode         %-3lld (decimal)\n", gOutputMode);
	u_log("\tgOutputMaxMode      %-3lld (decimal)\n", gOutputMaxMode);
	u_log("\tgOutputColumns      %-3lld (decimal)\n", gOutputColumns);
	u_log("\tgOutputRows         %-3lld (decimal)\n", gOutputRows);
	u_log("\n");
}

static void log_system_table(void)
{
	EFI_SYSTEM_TABLE *st = gSystemTable;

	u_log("EFI System Table\n");
	u_log("\tHdr.Signature       %016llX\n", st->Hdr.Signature);
	u_log("\tHdr.Revision        %08X\n", st->Hdr.Revision);
	u_log("\tHdr.HeaderSize      %08X\n", st->Hdr.HeaderSize);
	u_log("\tHdr.CRC32           %08X\n", st->Hdr.CRC32);
	u_log("\tHdr.Reserved        %08X\n", st->Hdr.Reserved);

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
	u_log("\tConsoleInHandle     %p\n", st->ConsoleInHandle);
	u_log("\tConIn               %p\n", st->ConIn);
	u_log("\tConsoleOutHandle    %p\n", st->ConsoleOutHandle);
	u_log("\tConOut              %p\n", st->ConOut);
	u_log("\tStandardErrorHandle %p\n", st->StandardErrorHandle);
	u_log("\tStdErr              %p\n", st->StdErr);
	u_log("\tRuntimeServices     %p\n", st->RuntimeServices);
	u_log("\tBootServices        %p\n", st->BootServices);
	u_log("\tTableEntries        %016llX\n", st->NumberOfTableEntries);
	u_log("\tConfigurationTable  %p\n", st->ConfigurationTable);
	u_log("\n");

	{
		EFI_CONFIGURATION_TABLE *t = st->ConfigurationTable;
		uint64_t table_entries = st->NumberOfTableEntries;
		uint64_t i, j;

		u_log("EFI Configuration Table\n");

		for (i = 0; i < table_entries; i++) {
			uint32_t d1 = t[i].VendorGuid.Data1;
			uint16_t d2 = t[i].VendorGuid.Data2;
			uint16_t d3 = t[i].VendorGuid.Data3;

			u_log("\t{0x%08X,0x%04hX,0x%04hX,{", d1, d2, d3);
			for (j = 0; j < 7; j++)
				u_log("0x%02hhX,", t[i].VendorGuid.Data4[j]);
			u_log("0x%02hhX}}\n", t[i].VendorGuid.Data4[7]);
		}
		u_log("\n");
	}
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

	query_output_mode();

	u_set_colors(0x07);
	u_clear_screen();
	u_print(welcome);

	u_log_init();
	u_log(welcome);

	log_global_variables();
	log_system_table();

	serial_init();

	if (memory_init()) {
		wait_until_return(20);
		return;
	}
	memory_print_map(u_log);

	if (block_init()) {
		memory_free();
		wait_until_return(20);
		return;
	}

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
		u_set_colors(0x05);
		u_print("Warning: the UEFI Boot Loader is not ready yet\n");
		u_set_colors(0x07);
	}

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
