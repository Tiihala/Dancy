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
 * uefi/file.c
 *      Read config.at, databases, and in_x64.at
 */

#include <uefi.h>

extern void *file_info;
extern uint64_t file_info_size;

static EFI_FILE_PROTOCOL *volume;

int file_init(void)
{
	EFI_HANDLE_PROTOCOL HandleProtocol;
	EFI_LOADED_IMAGE_PROTOCOL *image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simple_fs;
	EFI_FILE_SYSTEM_INFO *fs;
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

	u_log("Loaded Image\n");
	u_log("\tImageBase           %p\n", image->ImageBase);
	u_log("\tImageSize           %016llX\n", image->ImageSize);
	u_log("\tImageCodeType       %u\n", (unsigned)image->ImageCodeType);
	u_log("\tImageDataType       %u\n", (unsigned)image->ImageDataType);
	u_log("\n");

	/*
	 * Get the EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.
	 */
	{
		EFI_HANDLE handle = image->DeviceHandle;
		EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

		s = HandleProtocol(handle, &guid, (void **)&simple_fs);

		if (s != EFI_SUCCESS) {
			u_print("Error: EFI_SIMPLE_FILE_SYSTEM_PROTOCOL\n");
			return 1;
		}
	}

	/*
	 * Open the volume.
	 */
	if ((s = simple_fs->OpenVolume(simple_fs, &volume)) != EFI_SUCCESS) {
		u_print("Error: OpenVolume (%016llX)\n", s);
		return 1;
	}

	/*
	 * Get the EFI_FILE_SYSTEM_INFO (use the file info buffer).
	 */
	{
		EFI_GUID guid = EFI_FILE_SYSTEM_INFO_ID;
		uint64_t size = file_info_size;

		s = volume->GetInfo(volume, &guid, &size, file_info);

		if (s != EFI_SUCCESS) {
			memset(file_info, 0, (size_t)file_info_size);
			u_print("Error: EFI_FILE_SYSTEM_INFO (%016llX)\n", s);
			/*
			 * This error is not critical. Return "success".
			 */
			return 0;
		}
		fs = file_info;
	}

	u_log("File System Info\n");
	u_log("\tReadOnly            %s\n", fs->ReadOnly ? "true" : "false");
	u_log("\tVolumeSize          %016llX\n", fs->VolumeSize);
	u_log("\tFreeSpace           %016llX\n", fs->FreeSpace);
	u_log("\tBlockSize           %u\n", fs->BlockSize);
	u_log("\n");

	memset(file_info, 0, (size_t)file_info_size);
	return 0;
}

static EFI_FILE_INFO *get_file_info(EFI_FILE_PROTOCOL *fp)
{
	EFI_GUID guid = EFI_FILE_INFO_ID;
	uint64_t size = file_info_size;

	memset(file_info, 0, (size_t)file_info_size);
	if (fp->GetInfo(fp, &guid, &size, file_info) != EFI_SUCCESS) {
		memset(file_info, 0, (size_t)file_info_size);
	}
	return file_info;
}

static void close_volume(void)
{
	(void)volume->Close(volume), volume = NULL;
}

int file_read_all(void)
{
	EFI_FILE_PROTOCOL *fp;
	EFI_FILE_INFO *info;
	uint64_t db, size, buffer_size;
	int db_statistics[2] = { 0, 0 };
	EFI_STATUS s;

	if (volume == NULL)
		return 1;

	/*
	 * Read database files.
	 */
	for (db = 0; db < 1000; db++) {
		static const unsigned char file_header[16] = {
			0x8D, 0x41, 0x54, 0x0D, 0x0A, 0x73, 0x74, 0x64,
			0x0C, 0x44, 0x0C, 0x42, 0x0C, 0x0A, 0x71, 0xF8
		};
		wchar_t wname[24];
		char name[24];

		snprintf(&name[0], 24, "system\\db_%03d.at", db);
		for (size = 0; /* void */; size++) {
			char c = name[size];
			wname[size] = (wchar_t)c;
			if (c == '\0')
				break;
		}

		s = volume->Open(volume, (void **)&fp, &wname[0], 1, 0);

		if (s == EFI_NOT_FOUND) {
			continue;
		}

		db_statistics[1] += 1;

		if (s != EFI_SUCCESS) {
			u_print("Error: opening %s (%016llX)\n", &name[7], s);
			continue;
		}

		info = get_file_info(fp);
		size = info->FileSize;

		if ((info->Attribute & 0x10ull) != 0) {
			u_print("Error: %s is a directory\n", &name[7]);
			(void)fp->Close(fp);
			continue;
		}

		if (size != 65536) {
			u_print("Error: size of %s (%lld)\n", &name[7], size);
			(void)fp->Close(fp);
			continue;
		}

		buffer_size = size;

		s = fp->Read(fp, &buffer_size, memory_db_all[db]);
		(void)fp->Close(fp);

		if (s != EFI_SUCCESS || buffer_size != size) {
			u_print("Error: reading %s (%016llX)\n", &name[7], s);
			memset(memory_db_all[db], 0, 65536);
			continue;
		}

		if (memcmp(memory_db_all[db], &file_header[0], 16)) {
			u_print("Error: format of %s (header)\n", &name[7]);
			memset(memory_db_all[db], 0, 65536);
			continue;
		}

		if (*(((uint64_t *)memory_db_all[db]) + 2) != buffer_size) {
			u_print("Error: format of %s (size)\n", &name[7]);
			memset(memory_db_all[db], 0, 65536);
			continue;
		}

		if (*(((uint32_t *)memory_db_all[db]) + 6) != 0u) {
			u_print("Error: format of %s (reserved)\n", &name[7]);
			memset(memory_db_all[db], 0, 65536);
			continue;
		}

		if (*(((uint32_t *)memory_db_all[db]) + 7) != (uint32_t)db) {
			u_print("Error: format of %s (number)\n", &name[7]);
			memset(memory_db_all[db], 0, 65536);
			continue;
		}
		db_statistics[0] += 1;
	}

	/*
	 * Print and log database statistics.
	 */
	{
		if (db_statistics[0] != db_statistics[1])
			u_print("\n");
		u_print("System Databases\n");
		u_print("  %d of %d files read successfully\n\n",
			db_statistics[0], db_statistics[1]);
		u_log("System Databases\n");
		u_log("\t%d of %d files read successfully\n\n",
			db_statistics[0], db_statistics[1]);
	}

	/*
	 * Read file in_x64.at (must be available)
	 */
	{
		static const unsigned char file_header[16] = {
			0x8D, 0x41, 0x54, 0x0D, 0x0A, 0x73, 0x74, 0x64,
			0x0C, 0x49, 0x0C, 0x4E, 0x0C, 0x0A, 0x71, 0xF8
		};
		unsigned long file_crc;
		wchar_t wname[] = L"system\\in_x64.at";
		s = volume->Open(volume, (void **)&fp, &wname[0], 1, 0);

		if (s == EFI_NOT_FOUND) {
			u_print("Error: in_x64.at could not be found\n");
			return close_volume(), 1;
		}
		if (s != EFI_SUCCESS) {
			u_print("Error: opening in_x64.at (%016llX)\n", s);
			return close_volume(), 1;
		}

		info = get_file_info(fp);
		size = info->FileSize;

		if ((info->Attribute & 0x10ull) != 0) {
			u_print("Error: in_x64.at is a directory\n");
			return (void)fp->Close(fp), close_volume(), 1;
		}

		if (size < 32 || size > 65536) {
			u_print("Error: size of in_x64.at (%lld)\n", size);
			return (void)fp->Close(fp), close_volume(), 1;
		}

		buffer_size = size;

		s = fp->Read(fp, &buffer_size, memory_in_x64[0]);
		(void)fp->Close(fp);

		if (s != EFI_SUCCESS || buffer_size != size) {
			u_print("Error: reading in_x64.at (%016llX)\n", s);
			return close_volume(), 1;
		}

		if (memcmp(memory_in_x64[0], &file_header[0], 16)) {
			u_print("Error: format of in_x64.at (header)\n");
			return close_volume(), 1;
		}

		if (*(((uint64_t *)memory_in_x64[0]) + 2) != buffer_size) {
			u_print("Error: format of in_x64.at (size)\n");
			return close_volume(), 1;
		}

		file_crc = *(((uint32_t *)memory_in_x64[0]) + 6);
		*(((uint32_t *)memory_in_x64[0]) + 6) = 0u;

		if (crc32c(memory_in_x64[0], (size_t)size) != file_crc) {
			u_print("Error: format of in_x64.at (crc32c)\n");
			return close_volume(), 1;
		}

		*(((uint32_t *)memory_in_x64[0]) + 6) = (uint32_t)file_crc;

		if (*(((uint8_t *)memory_in_x64[0]) + 28) != 0x64u) {
			u_print("Error: format of in_x64.at (type)\n");
			return close_volume(), 1;
		}
	}

	/*
	 * Read file config.at (optional)
	 */
	{
		static const unsigned char file_header[16] = {
			0x8D, 0x41, 0x54, 0x0D, 0x0A, 0x73, 0x74, 0x64,
			0x0C, 0x43, 0x0C, 0x30, 0x0C, 0x0A, 0x71, 0xF8
		};
		unsigned long file_crc;
		wchar_t wname[] = L"system\\config.at";
		s = volume->Open(volume, (void **)&fp, &wname[0], 1, 0);

		if (s == EFI_NOT_FOUND) {
			/*
			 * This is not an error. Return "success".
			 */
			return close_volume(), 0;
		}
		if (s != EFI_SUCCESS) {
			u_print("Error: opening config.at (%016llX)\n", s);
			return close_volume(), 1;
		}

		info = get_file_info(fp);
		size = info->FileSize;

		if ((info->Attribute & 0x10ull) != 0) {
			u_print("Error: config.at is a directory\n");
			return (void)fp->Close(fp), close_volume(), 1;
		}

		if (size < 128 || size > (uint64_t)config_file_size) {
			u_print("Error: size of config.at (%lld)\n", size);
			return (void)fp->Close(fp), close_volume(), 1;
		}

		buffer_size = size;

		s = fp->Read(fp, &buffer_size, config_file);
		(void)fp->Close(fp);

		if (s != EFI_SUCCESS || buffer_size != size) {
			u_print("Error: reading config.at (%016llX)\n", s);
			return close_volume(), 1;
		}

		if (memcmp(config_file, &file_header[0], 16)) {
			u_print("Error: format of config.at (header)\n");
			return close_volume(), 1;
		}

		if (*(((uint64_t *)config_file) + 2) != buffer_size) {
			u_print("Error: format of config.at (size)\n");
			return close_volume(), 1;
		}

		file_crc = *(((uint32_t *)config_file) + 6);
		*(((uint32_t *)config_file) + 6) = 0u;

		if (crc32c(config_file, (size_t)size) != file_crc) {
			u_print("Error: format of config.at (crc32c)\n");
			return close_volume(), 1;
		}

		*(((uint32_t *)config_file) + 6) = (uint32_t)file_crc;
	}

	return close_volume(), 0;
}
