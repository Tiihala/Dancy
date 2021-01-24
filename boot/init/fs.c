/*
 * Copyright (c) 2020 Antti Tiihala
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
 * init/fs.c
 *      File system functions
 */

#include <boot/init.h>

static void *fat;
static int media_id;
static size_t fat_block_size;
static size_t fat_block_total;

int fat_get_size(int id, size_t *block_size, size_t *block_total)
{
	if (id != media_id)
		return 1;

	*block_size = fat_block_size;
	*block_total = fat_block_total;

	return 0;
}

int fat_get_time(char iso_8601_format[19])
{
	struct b_time bt;
	char buf[32];

	if (rtc_read(&bt))
		return 1;

	snprintf(&buf[0], 32, "%04u-%02u-%02uT%02u:%02u:%02u",
		bt.year, bt.month, bt.day, bt.hour, bt.minute, bt.second);

	memcpy(&iso_8601_format[0], &buf[0], 19);

	return 0;
}

int fat_io_read(int id, size_t lba, size_t *size, void *buf)
{
	size_t requested_size = *size;
	unsigned blocks = (unsigned)(requested_size / fat_block_size);

	*size = 0;
	memset(buf, 0, requested_size);

	if (id != media_id || lba >= fat_block_total)
		return 1;

	if (requested_size == 0)
		return 0;
	if (requested_size >= 0x80000000)
		return 1;
	if ((requested_size % fat_block_size) != 0)
		return 1;

	if (!b_set_read_buffer(buf, (unsigned)requested_size))
		return 1;

	if ((b_read_blocks((unsigned)lba, blocks)) != blocks)
		return 1;

	return *size = requested_size, 0;
}

int fat_io_write(int id, size_t lba, size_t *size, const void *buf)
{
	size_t requested_size = *size;
	unsigned blocks = (unsigned)(requested_size / fat_block_size);

	*size = 0;

	if (id != media_id || lba >= fat_block_total)
		return 1;

	if (requested_size == 0)
		return 0;
	if (requested_size >= 0x80000000)
		return 1;
	if ((requested_size % fat_block_size) != 0)
		return 1;

	if (!b_set_write_buffer((void *)buf, (unsigned)requested_size))
		return 1;

	if ((b_write_blocks((unsigned)lba, blocks)) != blocks)
		return 1;

	return *size = requested_size, 0;
}

void fs_print_error(int r, const char *name)
{
	const char *err = "unknown error";

	switch (r) {
	case FAT_BLOCK_READ_ERROR:
		err = "block read error";
		break;
	case FAT_BLOCK_WRITE_ERROR:
		err = "block write error";
		break;
	case FAT_DIRECTORY_NOT_EMPTY:
		err = "directory not empty";
		break;
	case FAT_FILE_ALREADY_OPEN:
		err = "file already open";
		break;
	case FAT_FILE_NOT_FOUND:
		err = "file not found";
		break;
	case FAT_INCONSISTENT_STATE:
		err = "inconsistent file system";
		break;
	case FAT_INVALID_FILE_NAME:
		err = "invalid file name";
		break;
	case FAT_INVALID_PARAMETERS:
		err = "invalid parameters";
		break;
	case FAT_NOT_ENOUGH_SPACE:
		err = "not enough space";
		break;
	case FAT_NOT_READY:
		err = "file system not ready";
		break;
	case FAT_READ_ONLY_FILE:
		err = "read-only file";
		break;
	case FAT_READ_ONLY_RECORD:
		err = "read-only record";
		break;
	case FAT_SEEK_ERROR:
		err = "seek error";
		break;
	default:
		break;
	}

	b_print("Error: %s (%s)\n", err, name);
	b_pause(), b_pause();
}

int fs_init(void)
{
	fat_block_size = (size_t)b_get_parameter(B_BYTES_PER_BLOCK);
	fat_block_total = (size_t)b_get_parameter(B_TOTAL_BLOCKS);

	return fat_create(&fat, media_id);
}

void fs_free(void)
{
	(void)fat_delete(fat);
	fat = NULL;
}

int fs_update_config_at(struct b_video_info *vi)
{
	static const char *name = "system/config.at";
	unsigned char config_at[128];
	const int fd = 0;
	unsigned crc, val;
	size_t size;
	int r;

	if ((r = fat_open(fat, fd, name, "rb+")) != 0) {
		fs_print_error(r, name);
		return 1;
	}

	size = sizeof(config_at);
	if ((r = fat_read(fat, fd, &size, &config_at[0])) != 0) {
		fs_print_error(r, name);
		fat_close(fat, fd);
		return 1;
	}

	(void)fat_seek(fat, fd, 0, 0);

	if (size != sizeof(config_at)) {
		fat_close(fat, fd);
		return 1;
	}

	/*
	 * Check the CRC-32C value.
	 */
	crc = LE32(&config_at[0x18]);
	W_LE32(&config_at[0x18], 0);

	if (crc != crc32c(&config_at[0], sizeof(config_at))) {
		fat_close(fat, fd);
		return 1;
	}

	/*
	 * Preferred video mode
	 *
	 * Off 0x24 = width
	 *     0x26 = (height << 4) | (bits_per_pixel >> 2)
	 */
	{
		val = vi->width;
		W_LE16(&config_at[0x24], val);

		val = vi->height << 4;

		if (vi->mode <= 1)
			val |= (8 >> 2);
		else if (vi->mode == 2)
			val |= (15 >> 2);
		else if (vi->mode == 3)
			val |= (16 >> 2);
		else if (vi->mode <= 5)
			val |= (24 >> 2);
		else
			val |= (32 >> 2);

		W_LE16(&config_at[0x26], val);
	}

	val = crc32c(&config_at[0], sizeof(config_at));
	W_LE32(&config_at[0x18], val);

	r = fat_write(fat, fd, &size, &config_at[0]);

	(void)fat_close(fat, fd);

	return r;
}

void fs_write_logs(void)
{
	static const char *name;
	const int fd_init = 0, fd_uefi = 1;
	size_t size;
	int r;

	name = "system/logs/log_init.txt";
	if ((r = fat_open(fat, fd_init, name, "wb")) != 0) {
		fs_print_error(r, name);
		return;
	}

	if (boot_loader_type == BOOT_LOADER_TYPE_UEFI) {
		name = "system/logs/log_uefi.txt";
		if ((r = fat_open(fat, fd_uefi, name, "wb")) != 0) {
			fs_print_error(r, name);

			(void)fat_close(fat, fd_init);
			return;
		}
	}

	size = boot_log_size;
	if ((r = fat_write(fat, fd_init, &size, boot_log)) != 0)
		fs_print_error(r, name);

	(void)fat_close(fat, fd_init);

	if (boot_loader_type == BOOT_LOADER_TYPE_UEFI) {
		struct b_uefi_info ui;

		size = sizeof(struct b_uefi_info);

		if (b_get_structure(&ui, B_UEFI_INFO) == size) {
			const char *data = ui.log;

			size = ui.log_size;
			if ((r = fat_write(fat, fd_uefi, &size, data)) != 0)
				fs_print_error(r, name);
		}

		(void)fat_close(fat, fd_uefi);
	}
}
