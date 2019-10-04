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

void uefi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, ...)
{
	va_list va;
	va_start(va, SystemTable);
	gImageHandle = ImageHandle;
	gSystemTable = SystemTable;
	gBaseAddress = va_arg(va, void *);
	gOriginalRsp = va_arg(va, void *);
	va_end(va);

	u_clear_screen();
	u_print(welcome);

	u_print("gImageHandle = %p  (address of gImageHandle is %p)\n",
		gImageHandle, &gImageHandle);
	u_print("gSystemTable = %p  (address of gSystemTable is %p)\n",
		gSystemTable, &gSystemTable);
	u_print("gBaseAddress = %p  (address of gBaseAddress is %p)\n",
		gBaseAddress, &gBaseAddress);
	u_print("gOriginalRsp = %p  (address of gOriginalRsp is %p)\n",
		gOriginalRsp, &gOriginalRsp);

	u_print("\n");

	{
		long seconds;
		for (seconds = 20; seconds >= 0; seconds--) {
			u_print("\rWaiting %2ld seconds...", seconds);
			gSystemTable->BootServices->Stall(1000 * 1000);
		}
	}
}

void u_clear_screen(void)
{
	gSystemTable->ConOut->ClearScreen(gSystemTable->ConOut);
}
