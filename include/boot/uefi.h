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
 * boot/uefi.h
 *      Header of Dancy Operating System
 */

#ifndef BOOT_UEFI_H
#define BOOT_UEFI_H

#ifndef DANCY_64
#error "Definition of DANCY_64 is not available"
#endif

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <arctic/include/ctype.h>
#include <arctic/include/stdio.h>
#include <arctic/include/stdlib.h>
#include <arctic/include/string.h>

#include <dancy/blob.h>
#include <dancy/keys.h>
#include <dancy/limits.h>
#include <dancy/types.h>

#include <boot/loader.h>

unsigned long crc32c(const void *obj, size_t len);

typedef void *                                  EFI_PVOID;
typedef void *                                  EFI_HANDLE;
typedef uint64_t                                EFI_STATUS;

#define EFI_SUCCESS                             0x0000000000000000ull
#define EFI_OUT_OF_RESOURCES                    0x8000000000000009ull
#define EFI_MEDIA_CHANGED                       0x800000000000000Dull
#define EFI_NOT_FOUND                           0x800000000000000Eull

typedef struct {
	uint64_t                                Signature;
	uint32_t                                Revision;
	uint32_t                                HeaderSize;
	uint32_t                                CRC32;
	uint32_t                                Reserved;
} EFI_TABLE_HEADER;

typedef struct {
	uint32_t                                Data1;
	uint16_t                                Data2;
	uint16_t                                Data3;
	uint8_t                                 Data4[8];
} EFI_GUID;

typedef struct {
	uint16_t                                Year;
	uint8_t                                 Month;
	uint8_t                                 Day;
	uint8_t                                 Hour;
	uint8_t                                 Minute;
	uint8_t                                 Second;
	uint8_t                                 Pad1;
	uint32_t                                Nanosecond;
	uint16_t                                TimeZone; /* INT16 */
	uint8_t                                 Daylight;
	uint8_t                                 Pad2;
} EFI_TIME;

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

typedef enum {
	AllHandles,
	ByRegisterNotify,
	ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

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

typedef EFI_STATUS (*EFI_HANDLE_PROTOCOL)(
	EFI_HANDLE                              Handle,
	EFI_GUID                                *Protocol,
	void                                    **Interface);

typedef EFI_STATUS (*EFI_LOCATE_HANDLE)(
	EFI_LOCATE_SEARCH_TYPE                  SearchType,
	EFI_GUID                                *Protocol,
	void                                    *SearchKey,
	uint64_t                                *BufferSize,
	EFI_HANDLE                              *Buffer);

typedef EFI_STATUS (*EFI_OPEN_PROTOCOL)(
	EFI_HANDLE                              Handle,
	EFI_GUID                                *Protocol,
	void                                    **Interface,
	EFI_HANDLE                              AgentHandle,
	EFI_HANDLE                              ControllerHandle,
	uint32_t                                Attributes);

typedef EFI_STATUS (*EFI_CLOSE_PROTOCOL)(
	EFI_HANDLE                              Handle,
	EFI_GUID                                *Protocol,
	EFI_HANDLE                              AgentHandle,
	EFI_HANDLE                              ControllerHandle);

typedef EFI_STATUS (*EFI_SET_WATCHDOG_TIMER)(
	uint64_t                                Timeout,
	uint64_t                                WatchdogCode,
	uint64_t                                Datasize,
	void                                    *WatchdogData);

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
	EFI_HANDLE_PROTOCOL                     HandleProtocol;
	EFI_PVOID                               Reserved;
	EFI_PVOID                               RegisterProtocolNotify;
	EFI_LOCATE_HANDLE                       LocateHandle;
	EFI_PVOID                               LocateDevicePath;
	EFI_PVOID                               InstallConfigurationTable;
	EFI_PVOID                               LoadImage;
	EFI_PVOID                               StartImage;
	EFI_PVOID                               Exit;
	EFI_PVOID                               UnloadImage;
	EFI_EXIT_BOOT_SERVICES                  ExitBootServices;
	EFI_PVOID                               GetNextMonotonicCount;
	EFI_STALL                               Stall;
	EFI_SET_WATCHDOG_TIMER                  SetWatchdogTimer;
	EFI_PVOID                               ConnectController;
	EFI_PVOID                               DisconnectController;
	EFI_OPEN_PROTOCOL                       OpenProtocol;
	EFI_CLOSE_PROTOCOL                      CloseProtocol;
	EFI_PVOID                               OpenProtocolInformation;
} EFI_BOOT_SERVICES;

typedef EFI_STATUS (*EFI_GET_TIME)(EFI_TIME *Time, void *Capabilities);

typedef EFI_STATUS (*EFI_GET_VARIABLE)(
	void                                    *VariableName,
	EFI_GUID                                *VendorGuid,
	uint32_t                                *Attributes,
	uint64_t                                *DataSize,
	void                                    *Data);

typedef EFI_STATUS (*EFI_GET_NEXT_VARIABLE_NAME)(
	uint64_t                                *VariableNameSize,
	void                                    *VariableName,
	EFI_GUID                                *VendorGuid);

typedef struct {
	EFI_TABLE_HEADER                        Hdr;
	EFI_GET_TIME                            GetTime;
	EFI_PVOID                               SetTime;
	EFI_PVOID                               GetWakeupTime;
	EFI_PVOID                               SetWakeupTime;
	EFI_PVOID                               SetVirtualAddressMap;
	EFI_PVOID                               ConvertPointer;
	EFI_GET_VARIABLE                        GetVariable;
	EFI_GET_NEXT_VARIABLE_NAME              GetNextVariableName;
	EFI_PVOID                               SetVariable;
	EFI_PVOID                               GetNextHighMonotonicCount;
	EFI_PVOID                               ResetSystem;
} EFI_RUNTIME_SERVICES;

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
	EFI_RUNTIME_SERVICES                    *RuntimeServices;
	EFI_BOOT_SERVICES                       *BootServices;
	uint64_t                                NumberOfTableEntries;
	EFI_CONFIGURATION_TABLE                 *ConfigurationTable;
} EFI_SYSTEM_TABLE;

typedef struct {
	uint32_t                                KeyShiftState;
	uint8_t                                 KeyToggleState;
} EFI_KEY_STATE;

typedef struct {
	EFI_INPUT_KEY                           Key;
	EFI_KEY_STATE                           KeyState;
} EFI_KEY_DATA;

typedef EFI_STATUS (*EFI_INPUT_READ_KEY_EX)(
	void                                    *This,
	EFI_KEY_DATA                            *KeyData);

typedef EFI_STATUS (*EFI_SET_STATE)(
	void                                    *This,
	uint8_t                                 *KeyToggleState);

#define EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID \
	{ 0xDD9E7534,0x7762,0x4698,{0x8C,0x14,0xF5,0x85,0x17,0xA6,0x25,0xAA} }

typedef struct {
	EFI_PVOID                               Reset;
	EFI_INPUT_READ_KEY_EX                   ReadKeyStrokeEx;
	EFI_PVOID                               WaitForKeyEx;
	EFI_SET_STATE                           SetState;
	EFI_PVOID                               RegisterKeyNotify;
	EFI_PVOID                               UnregisterKeyNotify;
} EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL;

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
	{ 0x5B1B31A1,0x9562,0x11D2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B} }

typedef struct {
	uint32_t                                Revision;
	EFI_HANDLE                              ParentHandle;
	EFI_SYSTEM_TABLE                        *SystemTable;
	EFI_HANDLE                              DeviceHandle;
	void                                    *FilePath;
	void                                    *Reserved;
	uint32_t                                LoadOptionsSize;
	void                                    *LoadOptions;
	void                                    *ImageBase;
	uint64_t                                ImageSize;
	EFI_MEMORY_TYPE                         ImageCodeType;
	EFI_MEMORY_TYPE                         ImageDataType;
	EFI_PVOID                               UnLoad;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef EFI_STATUS (*EFI_FILE_OPEN)(
	void                                    *This,
	void                                    **NewHandle,
	void                                    *FileName,
	uint64_t                                OpenMode,
	uint64_t                                Attributes);

typedef EFI_STATUS (*EFI_FILE_CLOSE)(void *This);

typedef EFI_STATUS (*EFI_FILE_READ)(
	void                                    *This,
	uint64_t                                *BufferSize,
	void                                    *Buffer);

typedef EFI_STATUS (*EFI_FILE_GET_INFO)(
	void                                    *This,
	EFI_GUID                                *InformationType,
	uint64_t                                *BufferSize,
	void                                    *Buffer);

#define EFI_FILE_INFO_ID \
	{ 0x09576E92,0x6D3F,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B} }

typedef struct {
	uint64_t                                Size;
	uint64_t                                FileSize;
	uint64_t                                PhysicalSize;
	EFI_TIME                                CreateTime;
	EFI_TIME                                LastAccessTime;
	EFI_TIME                                ModificationTime;
	uint64_t                                Attribute;
} EFI_FILE_INFO;

#define EFI_FILE_SYSTEM_INFO_ID \
	{ 0x09576E93,0x6D3F,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0X72,0x3B} }

typedef struct {
	uint64_t                                Size;
	uint8_t                                 ReadOnly;
	uint64_t                                VolumeSize;
	uint64_t                                FreeSpace;
	uint32_t                                BlockSize;
} EFI_FILE_SYSTEM_INFO;

typedef struct {
	uint64_t                                Revision;
	EFI_FILE_OPEN                           Open;
	EFI_FILE_CLOSE                          Close;
	EFI_PVOID                               Delete;
	EFI_FILE_READ                           Read;
	EFI_PVOID                               Write;
	EFI_PVOID                               GetPosition;
	EFI_PVOID                               SetPosition;
	EFI_FILE_GET_INFO                       GetInfo;
	EFI_PVOID                               SetInfo;
	EFI_PVOID                               Flush;
} EFI_FILE_PROTOCOL;

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
	{ 0x964E5B22,0x6459,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B} }

typedef EFI_STATUS (*SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
	void                                    *This,
	EFI_FILE_PROTOCOL                       **Root);

typedef struct {
	uint64_t                                Revision;
	SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef uint64_t                                EFI_LBA;

typedef struct {
	uint32_t                                MediaId;
	uint8_t                                 RemovableMedia;
	uint8_t                                 MediaPresent;
	uint8_t                                 LogicalPartition;
	uint8_t                                 ReadOnly;
	uint8_t                                 WriteCaching;
	uint32_t                                BlockSize;
	uint32_t                                IoAlign;
	EFI_LBA                                 LastBlock;
} EFI_BLOCK_IO_MEDIA;

typedef EFI_STATUS (*EFI_BLOCK_READ)(
	void                                    *This,
	uint32_t                                MediaId,
	EFI_LBA                                 LBA,
	uint64_t                                BufferSize,
	void                                    *Buffer);

typedef EFI_STATUS (*EFI_BLOCK_WRITE)(
	void                                    *This,
	uint32_t                                MediaId,
	EFI_LBA                                 LBA,
	uint64_t                                BufferSize,
	void                                    *Buffer);

typedef EFI_STATUS (*EFI_BLOCK_FLUSH)(void *This);

#define EFI_BLOCK_IO_PROTOCOL_GUID \
	{ 0x964E5B21,0x6459,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B} }

typedef struct {
	uint64_t                                Revision;
	EFI_BLOCK_IO_MEDIA                      *Media;
	EFI_PVOID                               Reset;
	EFI_BLOCK_READ                          ReadBlocks;
	EFI_BLOCK_WRITE                         WriteBlocks;
	EFI_BLOCK_FLUSH                         FlushBlocks;
} EFI_BLOCK_IO_PROTOCOL;

typedef struct {
	uint32_t                                ControlMask;
	uint32_t                                Timeout;
	uint64_t                                BaudRate;
	uint32_t                                ReceiveFifoDepth;
	uint32_t                                DataBits;
	uint32_t                                Parity;
	uint32_t                                StopBits;
} SERIAL_IO_MODE;

typedef enum {
	DefaultParity,
	NoParity
} EFI_PARITY_TYPE;

typedef enum {
	DefaultStopBits,
	OneStopBit
} EFI_STOP_BITS_TYPE;

typedef EFI_STATUS (*EFI_SERIAL_RESET)(void *This);

typedef EFI_STATUS (*EFI_SERIAL_SET_ATTRIBUTES)(
	void                                    *This,
	uint64_t                                BaudRate,
	uint32_t                                ReceiveFifoDepth,
	uint32_t                                TimeOut,
	EFI_PARITY_TYPE                         Parity,
	uint8_t                                 DataBits,
	EFI_STOP_BITS_TYPE                      StopBits);

typedef EFI_STATUS (*EFI_SERIAL_SET_CONTROL_BITS)(
	void                                    *This,
	uint32_t                                Control);

typedef EFI_STATUS (*EFI_SERIAL_GET_CONTROL_BITS)(
	void                                    *This,
	uint32_t                                *Control);

typedef EFI_STATUS (*EFI_SERIAL_WRITE)(
	void                                    *This,
	uint64_t                                *BufferSize,
	void                                    *Buffer);

typedef EFI_STATUS (*EFI_SERIAL_READ)(
	void                                    *This,
	uint64_t                                *BufferSize,
	void                                    *Buffer);

#define EFI_SERIAL_IO_PROTOCOL_GUID \
	{ 0xBB25CF6F,0xF1D4,0x11D2,{0x9A,0x0C,0x00,0x90,0x27,0x3F,0xC1,0xFD} }

typedef struct {
	uint32_t                                Revision;
	EFI_SERIAL_RESET                        Reset;
	EFI_SERIAL_SET_ATTRIBUTES               SetAttributes;
	EFI_SERIAL_SET_CONTROL_BITS             SetControl;
	EFI_SERIAL_GET_CONTROL_BITS             GetControl;
	EFI_SERIAL_WRITE                        Write;
	EFI_SERIAL_READ                         Read;
	SERIAL_IO_MODE                          *Mode;
} EFI_SERIAL_IO_PROTOCOL;

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
	{ 0x09576E91,0x6D3F,0x11D2,{0x8E,0x39,0x00,0xA0,0xC9,0x69,0x72,0x3B} }

typedef struct {
	uint8_t                                 Type;
	uint8_t                                 SubType;
	uint8_t                                 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

#define EFI_ACPI_TABLE_GUID \
	{ 0x8868E871,0xE4F1,0x11D3,{0xBC,0x22,0x00,0x80,0xC7,0x3C,0x88,0x81} }

#define ACPI_TABLE_GUID \
	{ 0xEB9D2D30,0x2D88,0x11D3,{0x9A,0x16,0x00,0x90,0x27,0x3F,0xC1,0x4D} }

typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
	uint32_t                                RedMask;
	uint32_t                                GreenMask;
	uint32_t                                BlueMask;
	uint32_t                                ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
	uint32_t                                Version;
	uint32_t                                HorizontalResolution;
	uint32_t                                VerticalResolution;
	EFI_GRAPHICS_PIXEL_FORMAT               PixelFormat;
	EFI_PIXEL_BITMASK                       PixelInformation;
	uint32_t                                PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
	uint32_t                                MaxMode;
	uint32_t                                Mode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    *Info;
	uint64_t                                SizeOfInfo;
	uint64_t                                FrameBufferBase;
	uint64_t                                FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(
	void                                    *This,
	uint32_t                                ModeNumber,
	uint64_t                                *SizeOfInfo,
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION    **Info);

typedef EFI_STATUS (*EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
	void                                    *This,
	uint32_t                                ModeNumber);

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
	{ 0x9042A9DE,0x23DC,0x4A38,{0x96,0xFB,0x7A,0xDE,0xD0,0x80,0x51,0x6A} }

typedef struct {
	EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;
	EFI_PVOID                               Blt;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE       *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

#define EFI_EDID_DISCOVERED_PROTOCOL_GUID \
	{ 0x1C0C34F6,0xD380,0x41FA,{0xA0,0x49,0x8A,0xD0,0x6C,0x1A,0x66,0xAA} }

typedef struct {
	uint32_t                                SizeOfEdid;
	uint8_t                                 *Edid;
} EFI_EDID_DISCOVERED_PROTOCOL;

#define EFI_EDID_ACTIVE_PROTOCOL_GUID \
	{ 0xBD8C1056,0x9F36,0x44EC,{0x92,0xA8,0xA6,0x33,0x7F,0x81,0x79,0x86} }

typedef struct {
	uint32_t                                SizeOfEdid;
	uint8_t                                 *Edid;
} EFI_EDID_ACTIVE_PROTOCOL;


/*
 * Global variables
 */
extern EFI_HANDLE                               gImageHandle;
extern EFI_SYSTEM_TABLE                         *gSystemTable;
extern void                                     *gBaseAddress;
extern void                                     *gOriginalRsp;
extern uint64_t                                 gOutputColumns;
extern uint64_t                                 gOutputRows;
extern uint64_t                                 gMapkey;

extern char                                     *uefi_log;
extern size_t                                   uefi_log_size;

extern void                                     *config_file;
extern size_t                                   config_file_size;

extern void                                     *memory_db_all[];
extern void                                     *memory_in_x64[];
extern void                                     *in_x64_syscalls;

extern uint32_t                                 b_bytes_per_block;
extern uint32_t                                 b_total_blocks;
extern uint32_t                                 b_hidden_blocks;
extern uint32_t                                 b_drive_number;
extern uint32_t                                 b_media_changed;

extern uint32_t                                 video_active;
extern uint32_t                                 video_column;
extern uint32_t                                 video_columns;
extern uint32_t                                 video_row;
extern uint32_t                                 video_rows;


/*
 * Declarations of block.c
 */
int block_init(void);
unsigned long block_set_read_buffer(void *addr, unsigned int size);
unsigned long block_read_blocks(unsigned int lba, unsigned int blocks);
unsigned long block_set_write_buffer(void *addr, unsigned int size);
unsigned long block_write_blocks(unsigned int lba, unsigned int blocks);


/*
 * Declarations of file.c
 */
int file_init(void);
int file_read_all(void);


/*
 * Declarations of key.c
 */
void key_init(void);
unsigned long key_get_code(void);


/*
 * Declarations of log.c
 */
void u_log_dump(void);
void u_log_init(void);
void u_log(const char *format, ...);


/*
 * Declarations of memory.c
 */
int memory_export_map(int finalize);
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
 * Declarations of serial.c
 */
void serial_init(void);
unsigned long serial_get_byte(unsigned int port);
unsigned long serial_put_byte(unsigned int port, unsigned char c);


/*
 * Declarations of syscall.asm and syscall.c
 *
 * The boot loader syscalls have been defined in the boot.h file.
 */
void syscall_init(void *addr);
void syscall_jump(void *addr);
void syscall_halt(void);
unsigned long syscall_exit(void);


/*
 * Declarations of uefi.c
 */
void uefi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, ...);


/*
 * Declarations of video.c
 */
void video_init(void);
void video_clear(int mode);
size_t video_get_edid(struct b_video_edid *out);
size_t video_get_info(struct b_video_info *out);
void video_output_string(const char *str, unsigned int len, int hl, int cr);
void video_show_menu(void);

#endif
