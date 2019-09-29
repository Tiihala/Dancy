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
		for (seconds = 1; seconds < LONG_MAX; seconds++) {
			u_print("\rHalt [%ld]", seconds);
			gSystemTable->BootServices->Stall(1000 * 1000);
		}
	}
}

const void *L(const char *s)
{
	static char buf[4096];
	size_t i;

	for (i = 0; i < (sizeof(buf) - 4); i = i + 2) {
		if (*s == '\0')
			break;
		if (*s == '\n') {
			buf[i] = '\r', buf[i + 1] = '\0';
			i = i + 2;
		}
		buf[i] = *s++, buf[i + 1] = '\0';
	}

	buf[i] = '\0', buf[i + 1] = '\0';
	return &buf[0];
}

void u_clear_screen(void)
{
	gSystemTable->ConOut->ClearScreen(gSystemTable->ConOut);
}

void u_print(const char *format, ...)
{
	char buf[2048];
	int ret;

	va_list va;
	va_start(va, format);
	ret = vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	if (ret <= 0)
		return;
	gSystemTable->ConOut->OutputString(gSystemTable->ConOut, L(&buf[0]));
}
