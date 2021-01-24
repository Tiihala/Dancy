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
 * uefi/serial.c
 *      Serial I/O Protocol
 */

#include <boot/uefi.h>

static EFI_HANDLE handle_com1;
static EFI_HANDLE handle_com2;

static EFI_SERIAL_IO_PROTOCOL *com1;
static EFI_SERIAL_IO_PROTOCOL *com2;

void serial_init(void)
{
	EFI_HANDLE_PROTOCOL HandleProtocol;
	EFI_LOCATE_HANDLE LocateHandle;
	EFI_HANDLE handles[128];
	uint64_t i, j, handles_count;
	EFI_STATUS s;

	HandleProtocol = gSystemTable->BootServices->HandleProtocol;
	LocateHandle = gSystemTable->BootServices->LocateHandle;

	u_log("Serial I/O Protocol\n");

	/*
	 * Find all the handles that support Serial I/O Protocol.
	 */
	{
		EFI_GUID guid = EFI_SERIAL_IO_PROTOCOL_GUID;
		uint64_t size = (uint64_t)(sizeof(handles));

		s = LocateHandle(ByProtocol, &guid, NULL, &size, &handles[0]);

		handles_count = size / (uint32_t)(sizeof(EFI_HANDLE));

		if (s != EFI_SUCCESS || handles_count == 0) {
			u_log("\tNone\n\n");
			return;
		}
	}

	for (i = 0; i < handles_count; i++) {
		EFI_GUID guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
		unsigned char *dp;

		s = HandleProtocol(handles[i], &guid, (void **)&dp);

		if (s != EFI_SUCCESS)
			continue;

		for (j = 0; j < 128; j++) {
			unsigned type = (unsigned)dp[0];
			unsigned sub = (unsigned)dp[1];
			unsigned len = (unsigned)LE16(&dp[2]);

			if (type == 0x7Fu && sub == 0xFF)
				break;

			/*
			 * ACPI Device Path
			 *
			 * _HID PNP0501, _UID Serial Port COM number 0-3
			 */
			if (type == 0x02 && sub == 0x01 && len >= 12) {
				uint32_t hid = (uint32_t)LE32(&dp[4]);
				uint32_t uid = (uint32_t)LE32(&dp[8]);

				if (hid == 0x050141D0u) {
					if (uid == 0 && handle_com1 == NULL)
						handle_com1 = handles[i];
					if (uid == 1 && handle_com2 == NULL)
						handle_com2 = handles[i];
				}
			}
			dp += len;
		}
	}

	if (handle_com1) {
		EFI_GUID guid = EFI_SERIAL_IO_PROTOCOL_GUID;

		s = HandleProtocol(handle_com1, &guid, (void **)&com1);

		if (s == EFI_SUCCESS)
			s = com1->Reset(com1);

		if (s == EFI_SUCCESS) {
			s = com1->SetAttributes(
				com1, 2400, 0, 0, NoParity, 8, OneStopBit);
		}

		if (s == EFI_SUCCESS) {
			u_log("\tCOM 1: ");
			u_log("%llu, ", com1->Mode->BaudRate);
			u_log("%u data bits, ", com1->Mode->DataBits);
			u_log("no parity, ");
			u_log("%u stop bit\n", com1->Mode->StopBits);
		} else {
			com1 = NULL;
		}
	}

	if (handle_com2) {
		EFI_GUID guid = EFI_SERIAL_IO_PROTOCOL_GUID;

		s = HandleProtocol(handle_com2, &guid, (void **)&com2);

		if (s == EFI_SUCCESS)
			s = com2->Reset(com2);

		if (s == EFI_SUCCESS) {
			s = com2->SetAttributes(
				com2, 2400, 0, 0, NoParity, 8, OneStopBit);
		}

		if (s == EFI_SUCCESS) {
			u_log("\tCOM 2: ");
			u_log("%llu, ", com2->Mode->BaudRate);
			u_log("%u data bits, ", com2->Mode->DataBits);
			u_log("no parity, ");
			u_log("%u stop bit\n", com2->Mode->StopBits);
		} else {
			com2 = NULL;
		}
	}

	u_log("\n");
}

unsigned long serial_get_byte(unsigned int port)
{
	unsigned char buf[2] = { 0, 0 };
	uint64_t size = 1;
	EFI_STATUS s = EFI_NOT_FOUND;

	if (port == 1 && com1 != NULL)
		s = com1->Read(com1, &size, &buf[0]);
	if (port == 2 && com2 != NULL)
		s = com2->Read(com2, &size, &buf[0]);

	if (s != EFI_SUCCESS || size == 0) {
		unsigned long r = ULONG_MAX;
		return (r << 8);
	}
	return (unsigned long)buf[0];
}

unsigned long serial_put_byte(unsigned int port, unsigned char c)
{
	unsigned char buf[2] = { c, 0 };
	uint64_t size = 1;
	EFI_STATUS s = EFI_NOT_FOUND;

	if (port == 1 && com1 != NULL)
		s = com1->Write(com1, &size, &buf[0]);
	if (port == 2 && com2 != NULL)
		s = com2->Write(com2, &size, &buf[0]);

	if (s != EFI_SUCCESS || size > 1)
		size = 0;
	return (unsigned long)size;
}
