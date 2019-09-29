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
 * uefi.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_UEFI_H
#define DANCY_UEFI_H

#if !defined(DANCY_64)
#error Definition of DANCY_64 is not available
#endif

#include <dancy/ctype.h>
#include <dancy/limits.h>
#include <dancy/stdarg.h>
#include <dancy/string.h>
#include <dancy/types.h>

int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

typedef void *                                  EFI_PVOID;
typedef void *                                  EFI_HANDLE;
typedef uint64_t                                EFI_STATUS;

typedef struct {
	uint64_t                                Signature;
	uint32_t                                Revision;
	uint32_t                                HeaderSize;
	uint32_t                                CRC32;
	uint32_t                                Reserved;
} EFI_TABLE_HEADER;

typedef EFI_STATUS (*EFI_STALL)(uint64_t Microseconds);

typedef struct {
	EFI_TABLE_HEADER                        Hdr;
	EFI_PVOID                               RaiseTPL;
	EFI_PVOID                               RestoreTPL;
	EFI_PVOID                               AllocatePages;
	EFI_PVOID                               FreePages;
	EFI_PVOID                               GetMemoryMap;
	EFI_PVOID                               AllocatePool;
	EFI_PVOID                               FreePool;
	EFI_PVOID                               CreateEvent;
	EFI_PVOID                               SetTimer;
	EFI_PVOID                               WaitForEvent;
	EFI_PVOID                               SignalEvent;
	EFI_PVOID                               CloseEvent;
	EFI_PVOID                               CheckEvent;
	EFI_PVOID                               InstallProtocolInterface;
	EFI_PVOID                               ReinstallProtocolInterface;
	EFI_PVOID                               UninstallProtocolInterface;
	EFI_PVOID                               HandleProtocol;
	EFI_PVOID                               Reserved;
	EFI_PVOID                               RegisterProtocolNotify;
	EFI_PVOID                               LocateHandle;
	EFI_PVOID                               LocateDevicePath;
	EFI_PVOID                               InstallConfigurationTable;
	EFI_PVOID                               LoadImage;
	EFI_PVOID                               StartImage;
	EFI_PVOID                               Exit;
	EFI_PVOID                               UnloadImage;
	EFI_PVOID                               ExitBootServices;
	EFI_PVOID                               GetNextMonotonicCount;
	EFI_STALL                               Stall;
	EFI_PVOID                               SetWatchdogTimer;
} EFI_BOOT_SERVICES;

typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN)(void *This);
typedef EFI_STATUS (*EFI_TEXT_STRING)(void *This, const void *String);

typedef struct {
	EFI_PVOID                               Reset;
	EFI_TEXT_STRING                         OutputString;
	EFI_PVOID                               TestString;
	EFI_PVOID                               QueryMode;
	EFI_PVOID                               SetMode;
	EFI_PVOID                               SetAttribute;
	EFI_TEXT_CLEAR_SCREEN                   ClearScreen;
	EFI_PVOID                               SetCursorPosition;
	EFI_PVOID                               EnableCursor;
	EFI_PVOID                               Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
	EFI_TABLE_HEADER                        Hdr;
	EFI_PVOID                               FirmwareVendor;
	uint32_t                                FirmwareRevision;
	EFI_PVOID                               ConsoleInHandle;
	EFI_PVOID                               ConIn;
	EFI_PVOID                               ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL         *ConOut;
	EFI_PVOID                               StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL         *StdErr;
	EFI_PVOID                               RuntimeServices;
	EFI_BOOT_SERVICES                       *BootServices;
	uint64_t                                NumberOfTableEntries;
	EFI_PVOID                               ConfigurationTable;
} EFI_SYSTEM_TABLE;


/*
 * Global variables
 */
extern EFI_HANDLE                               gImageHandle;
extern EFI_SYSTEM_TABLE                         *gSystemTable;
extern void                                     *gBaseAddress;
extern void                                     *gOriginalRsp;

extern char                                     *uefi_log;
extern size_t                                   uefi_log_size;


/*
 * Declarations of log.c
 */
void u_log_init(void);
void u_log(const char *format, ...);


/*
 * Declarations of print.c
 */
const void *L(const char *s);
void u_print(const char *format, ...);


/*
 * Declarations of uefi.c
 */
void uefi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, ...);
void u_clear_screen(void);

#endif
