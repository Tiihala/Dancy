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
 * uefi/block.c
 *      Block I/O Protocol
 */

#include <boot/uefi.h>

uint32_t b_bytes_per_block;
uint32_t b_total_blocks;
uint32_t b_hidden_blocks;
uint32_t b_drive_number;
uint32_t b_media_changed;

static EFI_BLOCK_IO_PROTOCOL *block_io;
static uint32_t media_id;

static void *read_buffer;
static void *write_buffer;

static uint64_t read_buffer_size;
static uint64_t write_buffer_size;

int block_init(void)
{
	EFI_HANDLE_PROTOCOL HandleProtocol;
	EFI_LOADED_IMAGE_PROTOCOL *image;
	EFI_BLOCK_IO_MEDIA *media;
	EFI_STATUS s;

	HandleProtocol = gSystemTable->BootServices->HandleProtocol;

	/*
	 * Get the EFI_LOADED_IMAGE_PROTOCOL.
	 */
	{
		EFI_HANDLE handle = gImageHandle;
		EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

		s = HandleProtocol(handle, &guid, (void **)&image);

		if (s != EFI_SUCCESS) {
			u_print("Error: EFI_LOADED_IMAGE_PROTOCOL\n");
			return 1;
		}
	}

	/*
	 * Get the EFI_BLOCK_IO_PROTOCOL.
	 */
	{
		EFI_HANDLE handle = image->DeviceHandle;
		EFI_GUID guid = EFI_BLOCK_IO_PROTOCOL_GUID;

		s = HandleProtocol(handle, &guid, (void **)&block_io);

		if (s != EFI_SUCCESS) {
			u_print("Error: EFI_BLOCK_IO_PROTOCOL\n");
			/*
			 * This is not a critical error. The boot loader
			 * syscalls for block I/O simply do not work.
			 */
			return block_io = NULL, 0;
		}
		media = block_io->Media;
		media_id = media->MediaId;
	}

	u_log("Block I/O Protocol\n");
	u_log("\tMediaId             %08X\n", media_id);
	u_log("\tRemovableMedia      %hhu\n", media->RemovableMedia);
	u_log("\tMediaPresent        %hhu\n", media->MediaPresent);
	u_log("\tLogicalPartition    %hhu\n", media->LogicalPartition);
	u_log("\tReadOnly            %hhu\n", media->ReadOnly);
	u_log("\tWriteCaching        %hhu\n", media->WriteCaching);
	u_log("\tBlockSize           %u\n", media->BlockSize);
	u_log("\tIoAlign             %u\n", media->IoAlign);
	u_log("\tLastBlock           %08llX\n", media->LastBlock);
	u_log("\n");

	/*
	 * Set the default values for the b_get_parameter function.
	 */
	b_bytes_per_block = media->BlockSize;
	b_total_blocks = 0;
	b_hidden_blocks = 0;
	b_drive_number = media_id;
	b_media_changed = 0;

	/*
	 * Only supported if BlockSize <= 4096 and IoAlign <= 4096.
	 */
	if (media->BlockSize > 4096 || media->IoAlign > 4096)
		return block_io = NULL, 0;

	/*
	 * Read the block 0 and get values from the "Bios Parameter Blocks".
	 */
	{
		uint64_t Memory = 0;
		unsigned char bpb[64];

		memset(&bpb[0], 0, 64);

		s = gSystemTable->BootServices->AllocatePages(
			AllocateAnyPages, EfiLoaderData, 2, &Memory);

		if (s == EFI_SUCCESS) {
			block_io->ReadBlocks(block_io, media_id, 0,
				media->BlockSize, (void *)Memory);
			memcpy(&bpb[0], (const void *)Memory, 62);
			gSystemTable->BootServices->FreePages(Memory, 2);
		}

		if (LE16(&bpb[0x0B]) != (unsigned long)media->BlockSize)
			return block_io = NULL, 0;

		b_total_blocks = (uint32_t)LE16(&bpb[0x13]);
		b_hidden_blocks = (uint32_t)LE32(&bpb[0x1C]);
	}

	u_log("\tb_total_blocks      %08X\n", b_total_blocks);
	u_log("\tb_hidden_blocks     %08X\n", b_hidden_blocks);
	u_log("\n");
	return 0;
}

unsigned long block_set_read_buffer(void *addr, unsigned int size)
{
	uint64_t memory = (uint64_t)addr;
	uint32_t io_align;

	if (memory == 0 || block_io == NULL)
		return 0;
	if ((io_align = block_io->Media->IoAlign) == 0)
		io_align = 1;
	if ((memory % io_align) != 0)
		return 0;

	read_buffer = addr;
	read_buffer_size = (uint64_t)size;
	return (unsigned long)b_bytes_per_block;
}

unsigned long block_read_blocks(unsigned int lba, unsigned int blocks)
{
	uint64_t next_block = (uint64_t)lba + blocks;
	uint64_t size;
	EFI_STATUS s;

	if (b_media_changed != 0)
		return 0;
	if (blocks == 0 || block_io == NULL || read_buffer == NULL)
		return 0;
	if (next_block > b_total_blocks)
		return 0;

	size = (uint64_t)blocks * block_io->Media->BlockSize;
	if (size > read_buffer_size)
		return 0;

	s = block_io->ReadBlocks(block_io, media_id,
		(EFI_LBA)lba, size, read_buffer);

	if (s == EFI_MEDIA_CHANGED) {
		b_media_changed = 1;
		return 0;
	}
	return (s == EFI_SUCCESS) ? (unsigned long)blocks : 0;
}

unsigned long block_set_write_buffer(void *addr, unsigned int size)
{
	uint64_t memory = (uint64_t)addr;
	uint32_t io_align;

	if (memory == 0 || block_io == NULL)
		return 0;
	if ((io_align = block_io->Media->IoAlign) == 0)
		io_align = 1;
	if ((memory % io_align) != 0)
		return 0;

	write_buffer = addr;
	write_buffer_size = (uint64_t)size;
	return (unsigned long)b_bytes_per_block;
}

unsigned long block_write_blocks(unsigned int lba, unsigned int blocks)
{
	uint64_t next_block = (uint64_t)lba + blocks;
	uint64_t size;
	EFI_STATUS s;

	if (b_media_changed != 0)
		return 0;
	if (blocks == 0 || block_io == NULL || write_buffer == NULL)
		return 0;
	if (next_block > b_total_blocks)
		return 0;

	size = (uint64_t)blocks * block_io->Media->BlockSize;
	if (size > write_buffer_size)
		return 0;

	s = block_io->WriteBlocks(block_io, media_id,
		(EFI_LBA)lba, size, write_buffer);

	if (s == EFI_MEDIA_CHANGED) {
		b_media_changed = 1;
		return 0;
	}

	if (block_io->Media->WriteCaching)
		s = block_io->FlushBlocks(block_io);

	return (s == EFI_SUCCESS) ? (unsigned long)blocks : 0;
}
