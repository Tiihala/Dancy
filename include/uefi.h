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
#include <dancy/stdlib.h>
#include <dancy/string.h>
#include <dancy/types.h>
#include <kernel/boot.h>

int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

typedef void *                                  EFI_PVOID;
typedef void *                                  EFI_HANDLE;
typedef uint64_t                                EFI_STATUS;

#define EFI_SUCCESS                             0x0000000000000000ull
#define EFI_OUT_OF_RESOURCES                    0x8000000000000009ull
#define EFI_NOT_FOUND                           0x800000000000000Eull

typedef struct {
	uint64_t                                Signature;
	uint32_t                                Revision;
	uint32_t                                HeaderSize;
	uint32_t                                CRC32;
	uint32_t                                Reserved;
} EFI_TABLE_HEADER;

typedef struct {
	uint32_t                                Type;
	uint32_t                                Pad;
	uint64_t                                PhysicalAddress;
	uint64_t                                VirtualAddress;
	uint64_t                                NumberOfPages;
	uint64_t                                Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory
} EFI_MEMORY_TYPE;

typedef enum {
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress
} EFI_ALLOCATE_TYPE;

typedef EFI_STATUS (*EFI_ALLOCATE_PAGES)(
	EFI_ALLOCATE_TYPE                       Type,
	EFI_MEMORY_TYPE                         MemoryType,
	uint64_t                                Pages,
	uint64_t                                *Memory);

typedef EFI_STATUS (*EFI_GET_MEMORY_MAP)(
	uint64_t                                *MemoryMapSize,
	EFI_MEMORY_DESCRIPTOR                   *MemoryMap,
	uint64_t                                *Mapkey,
	uint64_t                                *DescriptorSize,
	uint32_t                                *DescriptorVersion);

typedef EFI_STATUS (*EFI_FREE_PAGES)(uint64_t Memory, uint64_t Pages);
typedef EFI_STATUS (*EFI_EXIT_BOOT_SERVICES)(void *Image, uint64_t MapKey);
typedef EFI_STATUS (*EFI_STALL)(uint64_t Microseconds);

typedef struct {
	EFI_TABLE_HEADER                        Hdr;
	EFI_PVOID                               RaiseTPL;
	EFI_PVOID                               RestoreTPL;
	EFI_ALLOCATE_PAGES                      AllocatePages;
	EFI_FREE_PAGES                          FreePages;
	EFI_GET_MEMORY_MAP                      GetMemoryMap;
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
	EFI_EXIT_BOOT_SERVICES                  ExitBootServices;
	EFI_PVOID                               GetNextMonotonicCount;
	EFI_STALL                               Stall;
	EFI_PVOID                               SetWatchdogTimer;
} EFI_BOOT_SERVICES;

typedef struct {
	uint32_t                                MaxMode;
	uint32_t                                Mode;
	uint32_t                                Attribute;
	uint32_t                                CursorColumn;
	uint32_t                                CursorRow;
	uint8_t                                 CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN)(void *This);
typedef EFI_STATUS (*EFI_TEXT_ENABLE_CURSOR)(void *This, uint8_t Visible);
typedef EFI_STATUS (*EFI_TEXT_SET_ATTRIBUTE)(void *This, uint64_t Attribute);
typedef EFI_STATUS (*EFI_TEXT_STRING)(void *This, const void *String);

typedef EFI_STATUS (*EFI_TEXT_QUERY_MODE)(
	void                                    *This,
	uint64_t                                ModeNumber,
	uint64_t                                *Columns,
	uint64_t                                *Rows);

typedef EFI_STATUS (*EFI_TEXT_SET_CURSOR_POSITION)(
	void                                    *This,
	uint64_t                                Column,
	uint64_t                                Row);

typedef struct {
	EFI_PVOID                               Reset;
	EFI_TEXT_STRING                         OutputString;
	EFI_PVOID                               TestString;
	EFI_TEXT_QUERY_MODE                     QueryMode;
	EFI_PVOID                               SetMode;
	EFI_TEXT_SET_ATTRIBUTE                  SetAttribute;
	EFI_TEXT_CLEAR_SCREEN                   ClearScreen;
	EFI_TEXT_SET_CURSOR_POSITION            SetCursorPosition;
	EFI_TEXT_ENABLE_CURSOR                  EnableCursor;
	SIMPLE_TEXT_OUTPUT_MODE                 *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
	uint32_t                                Data1;
	uint16_t                                Data2;
	uint16_t                                Data3;
	uint8_t                                 Data4[8];
} EFI_GUID;

typedef struct {
	EFI_GUID                                VendorGuid;
	EFI_PVOID                               VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct {
	uint16_t                                ScanCode;
	uint16_t                                UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (*EFI_INPUT_READ_KEY)(void *This, EFI_INPUT_KEY *Key);

typedef struct {
	EFI_PVOID                               Reset;
	EFI_INPUT_READ_KEY                      ReadKeyStroke;
	EFI_PVOID                               WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct {
	EFI_TABLE_HEADER                        Hdr;
	EFI_PVOID                               FirmwareVendor;
	uint32_t                                FirmwareRevision;
	EFI_PVOID                               ConsoleInHandle;
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL          *ConIn;
	EFI_PVOID                               ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL         *ConOut;
	EFI_PVOID                               StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL         *StdErr;
	EFI_PVOID                               RuntimeServices;
	EFI_BOOT_SERVICES                       *BootServices;
	uint64_t                                NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE                 *ConfigurationTable;
} EFI_SYSTEM_TABLE;


/*
 * Global variables
 */
extern EFI_HANDLE                               gImageHandle;
extern EFI_SYSTEM_TABLE                         *gSystemTable;
extern void                                     *gBaseAddress;
extern void                                     *gOriginalRsp;
extern uint64_t                                 gOutputMode;
extern uint64_t                                 gOutputMaxMode;
extern uint64_t                                 gOutputColumns;
extern uint64_t                                 gOutputRows;
extern uint64_t                                 gMapkey;

extern char                                     *uefi_log;
extern size_t                                   uefi_log_size;

extern void                                     *memory_db_all[];
extern void                                     *memory_in_x64[];


/*
 * Declarations of log.c
 */
void u_log_dump(void);
void u_log_init(void);
void u_log(const char *format, ...);


/*
 * Declarations of memory.c
 */
int memory_export_map(void);
void memory_free(void);
int memory_init(void);
void memory_print_map(void (*print)(const char *, ...));
int memory_update_map(void);


/*
 * Declarations of misc.c
 */
void u_clear_screen(void);
void u_get_cursor(uint64_t *c, uint64_t *r);
int u_read_key(EFI_INPUT_KEY *key);
void u_set_colors(uint64_t attribute);
void u_set_cursor(uint64_t c, uint64_t r);
void u_stall(uint64_t milliseconds);


/*
 * Declarations of print.c
 */
const void *L(const char *s);
void u_print(const char *format, ...);


/*
 * Declarations of uefi.c
 */
void uefi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, ...);

#endif
