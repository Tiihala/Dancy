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
 * fat.c
 *      File system driver (FAT)
 */

#include <limits.h>
#include <stddef.h>

int fat_create(void **instance, int id);
int fat_delete(void *fat);

int fat_close(void *fat, int fd);
int fat_control(void *fat, int fd, int write, unsigned char record[32]);
int fat_eof(void *fat, int fd);
int fat_open(void *fat, int fd, const char *name, const char *mode);
int fat_read(void *fat, int fd, size_t *size, void *buf);
int fat_remove(void *fat, const char *name);
int fat_rename(void *fat, const char *old_name, const char *new_name);
int fat_seek(void *fat, int fd, int offset, int whence);
int fat_tell(void *fat, int fd, unsigned int *offset);
int fat_write(void *fat, int fd, size_t *size, const void *buf);

extern int fat_get_size(int id, size_t *block_size, size_t *block_total);
extern int fat_get_time(char iso_8601_format[19]);
extern int fat_io_read(int id, size_t lba, size_t *size, void *buf);
extern int fat_io_write(int id, size_t lba, size_t *size, const void *buf);

#define FAT_BLOCK_READ_ERROR      (0x10)  /* "block read error"         */
#define FAT_BLOCK_WRITE_ERROR     (0x11)  /* "block write error"        */
#define FAT_DIRECTORY_NOT_EMPTY   (0x12)  /* "directory not empty"      */
#define FAT_FILE_ALREADY_OPEN     (0x13)  /* "file already open"        */
#define FAT_FILE_NOT_FOUND        (0x14)  /* "file not found"           */
#define FAT_INCONSISTENT_STATE    (0x15)  /* "inconsistent file system" */
#define FAT_INVALID_FILE_NAME     (0x16)  /* "invalid file name"        */
#define FAT_INVALID_PARAMETERS    (0x17)  /* "invalid parameters"       */
#define FAT_NOT_ENOUGH_SPACE      (0x18)  /* "not enough space"         */
#define FAT_NOT_READY             (0x19)  /* "file system not ready"    */
#define FAT_READ_ONLY_FILE        (0x1A)  /* "read-only file"           */
#define FAT_READ_ONLY_RECORD      (0x1B)  /* "read-only record"         */
#define FAT_SEEK_ERROR            (0x1C)  /* "seek error"               */

extern void free(void *ptr);
extern void *malloc(size_t size);
extern void *memcpy(void *s1, const void *s2, size_t n);
extern void *memset(void *s, int c, size_t n);

#if CHAR_BIT != 8 || INT_MAX < 2147483647
#error "Unsupported implementation-defined characteristics"
#endif

#define B8(a,b,c) (((unsigned)((a)[(b)]) & 0xFFu) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))
#define LE32(a) (B8((a),0,0) | B8((a),1,8) | B8((a),2,16) | B8((a),3,24))

#define W_LE16(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >> 0) & 0xFFu), \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >> 8) & 0xFFu))

#define W_LE32(a,d) ( \
	*((a) + 0) = (unsigned char)(((unsigned)(d) >>  0) & 0xFFu), \
	*((a) + 1) = (unsigned char)(((unsigned)(d) >>  8) & 0xFFu), \
	*((a) + 2) = (unsigned char)(((unsigned)(d) >> 16) & 0xFFu), \
	*((a) + 3) = (unsigned char)(((unsigned)(d) >> 24) & 0xFFu))

#define MODE_APPEND   (0x01)
#define MODE_CREATE   (0x02)
#define MODE_MODIFY   (0x04)
#define MODE_READ     (0x08)
#define MODE_ROOT_DIR (0x10)
#define MODE_WRITE    (0x20)

#define FAT_READY     (0x00746166)

struct fat_name {
	char name[12];
	char length;
	char is_dir;
	char padding[2];
};

struct fat_fd {
	int opened;
	unsigned int mode;

	unsigned char record[32];
	unsigned int record_offset;
	unsigned int record_sector;

	int eof;
	unsigned int offset;
	unsigned int cluster_idx;
	unsigned int cluster_val;
};

struct fat_instance {
	int ready;
	int id;

	unsigned int fd_entries;
	unsigned int fd_special;
	struct fat_fd *fd;

	unsigned int block_size;
	unsigned int block_total;
	unsigned char *block_buffer;

	unsigned int cluster_size;
	unsigned int cluster_sector;
	unsigned char *cluster_buffer;

	unsigned char *table_buffer;
	unsigned char *table_sectors;

	unsigned int table_buffer_size;
	unsigned int table_entries;
	unsigned int table_offset;

	unsigned int fs_bytes_per_sector;
	unsigned int fs_cluster_sectors;
	unsigned int fs_reserved_sectors;
	unsigned int fs_tables;

	unsigned int fs_directory_entries;
	unsigned int fs_directory_sectors;
	unsigned int fs_total_sectors;

	unsigned int fs_table_sectors;
	unsigned int fs_table_size;

	unsigned int fs_data_sectors;
	unsigned int fs_clusters;
	unsigned int fs_root_cluster;
	unsigned int fs_last_table_sector;

	unsigned int fs_info_modified;
	unsigned int fs_info_sector;
	unsigned int fs_info_free;
	unsigned int fs_info_next;

	int type_12;
	int type_16;
	int type_32;

	unsigned int path_entries;
	struct fat_name *path_buffer;

	size_t block_buffer_lba;
	size_t cluster_buffer_lba;
	size_t io_mul;

	unsigned int maximum_file_size;
	unsigned int last_allocated;
};

#define this_fat ((struct fat_instance *)(fat))

static int check_fd(void *fat, int fd, int opened)
{
	const int maximum_fd_entries = 65536;
	int fd_entries = (int)this_fat->fd_entries;

	if (fd < 0 || fd >= maximum_fd_entries)
		return 1;

	/*
	 * Allocate more file descriptor entries if needed.
	 */
	if (fd >= fd_entries) {
		int new_fd_entries = fd + 2;
		size_t size;
		void *buf;

		if (new_fd_entries - fd_entries < 8)
			new_fd_entries = fd_entries + 8;

		if (new_fd_entries > (maximum_fd_entries + 1))
			new_fd_entries = (maximum_fd_entries + 1);

		size = (size_t)new_fd_entries * sizeof(struct fat_fd);

		buf = malloc(size);
		if (buf == NULL)
			return 1;

		memset(buf, 0, size);

		/*
		 * Do not copy the special entry.
		 */
		size = (size_t)fd_entries * sizeof(struct fat_fd);
		memcpy(buf, this_fat->fd, size);

		free(this_fat->fd);
		this_fat->fd = buf;

		/*
		 * Reserve the last fd_entry.
		 */
		this_fat->fd_entries = (unsigned int)(new_fd_entries - 1);
		this_fat->fd_special = this_fat->fd_entries;
	}

	return ((this_fat->fd[fd].opened != 0) != (opened != 0));
}

static void *get_record(void *fat, int fd)
{
	return &this_fat->fd[fd].record[0];
}

static void init_fat_fd(void *fat, int fd)
{
	void *fd_entry = &this_fat->fd[fd];

	memset(fd_entry, 0, sizeof(struct fat_fd));
}

static int fat_strncmp(const char *s1, const char *s2, size_t n)
{
	unsigned char c1, c2;

	while (n != 0) {
		c1 = *((const unsigned char *)s1++);
		c2 = *((const unsigned char *)s2++);
		if (!--n || !c1 || c1 != c2) {
			if (!c1 && !c2)
				break;
			if (c1 < c2)
				return -1;
			if (c1 > c2)
				return 1;
		}
	}
	return 0;
}

static int parse_number(const char *str, int len)
{
	int val = 0;
	int i;

	for (i = 0; i < len; i++) {
		char c = str[i];
		if (c < '0' || c > '9')
			return -1;
		val = (val * 10) + ((int)c - '0');
	}

	return val;
}

static void write_timestamps(unsigned char *entry, int created, int modified)
{
	char iso_8601_format[19];
	int year, mon, day, hour, min, sec;
	unsigned int fat_date, fat_time;
	int err = 0;

	memset(&iso_8601_format[0], 0, 19);

	/*
	 * "YYYY-MM-DDThh:mm:ss"
	 */
	if (fat_get_time(iso_8601_format)) {
		memset(&iso_8601_format[0], 0, 19);
		err = 1;
	}

	if (iso_8601_format[4] != '-' || iso_8601_format[7] != '-')
		err = 1;
	if (iso_8601_format[10] != 'T')
		err = 1;
	if (iso_8601_format[13] != ':' || iso_8601_format[16] != ':')
		err = 1;

	year = parse_number(&iso_8601_format[0], 4);
	mon  = parse_number(&iso_8601_format[5], 2);
	day  = parse_number(&iso_8601_format[8], 2);
	hour = parse_number(&iso_8601_format[11], 2);
	min  = parse_number(&iso_8601_format[14], 2);
	sec  = parse_number(&iso_8601_format[17], 2);

	if (year > 2099) {
		year = 2099, mon = 12, day = 31;
		hour = 23, min = 59, sec = 58;
	}

	err |= (mon  >= 0 && mon  <= 11) ? 0 : 1;
	err |= (day  >= 1 && day  <= 31) ? 0 : 1;
	err |= (hour >= 0 && hour <= 23) ? 0 : 1;
	err |= (min  >= 0 && min  <= 59) ? 0 : 1;
	err |= (sec  >= 0 && sec  <= 59) ? 0 : 1;

	if (err != 0 || year < 1980) {
		year = 1980, mon = 1, day = 1;
		hour = 0, min = 0, sec = 0;
	}

	fat_date = (unsigned)day & 0x1Fu;
	fat_date |= ((unsigned)mon & 0x0Fu) << 5;
	fat_date |= (((unsigned)year - 1980u) & 0x7Fu) << 9;

	fat_time = ((unsigned)sec >> 1) & 0x1Fu;
	fat_time |= ((unsigned)min & 0x3Fu) << 5;
	fat_time |= ((unsigned)hour & 0x1Fu) << 11;

	/*
	 * Always update accessed date.
	 */
	W_LE16(&entry[18], fat_date);

	/*
	 * Update created date and time if requested.
	 */
	if (created) {
		W_LE16(&entry[14], fat_time);
		W_LE16(&entry[16], fat_date);
	}

	/*
	 * Update modified date and time if requested.
	 */
	if (modified) {
		W_LE16(&entry[22], fat_time);
		W_LE16(&entry[24], fat_date);
	}
}

static int read_bios_parameter_block(void *fat, int id)
{
	unsigned char *buf = this_fat->block_buffer;
	size_t block_size = 0, block_total = 0;
	unsigned int val;

	if (fat_get_size(id, &block_size, &block_total))
		return 1;

	if (block_size < 1 || block_size > 4096)
		return 1;
	if ((block_size & (block_size - 1)) != 0)
		return 1;
	if (!(block_total <= 0xFFFFFFFF))
		return 1;

	this_fat->block_size = (unsigned int)block_size;
	this_fat->block_total = (unsigned int)block_total;

	{
		size_t size = block_size;

		while (size < 512)
			size *= 2;

		if (fat_io_read(id, 0, &size, buf))
			return 1;
	}

	/*
	 * Check the jump instruction.
	 */
	if (buf[0] == 0xEB) {
		val = buf[1];
		if (val < 0x3C || (val >= 0x80 && val != 0xFE))
			return 1;
	} else if (buf[0] == 0xE9) {
		val = LE16(&buf[1]);
		if (val < 0x3C || (val >= 0x1FD && val != 0xFFFD))
			return 1;
	} else {
		return 1;
	}

	/*
	 * Check the boot signature.
	 */
	if (LE16(&buf[510]) != 0xAA55)
		return 1;

	/*
	 * Sector sizes 512, 1024, 2048, and 4096 are supported. The block
	 * size can not be bigger than the file system sector size.
	 */
	val = LE16(&buf[11]);
	if (val < 512 || val > 4096 || (val & (val - 1)) != 0)
		return 1;
	if (val < block_size)
		return 1;
	this_fat->fs_bytes_per_sector = val;
	this_fat->io_mul = (size_t)(val / block_size);

	/*
	 * Sectors per cluster.
	 */
	val = buf[13];
	if (val < 1 || val > 128 || (val & (val - 1)) != 0)
		return 1;
	this_fat->fs_cluster_sectors = val;

	/*
	 * Reserved sectors.
	 */
	val = LE16(&buf[14]);
	if (val == 0)
		return 1;
	this_fat->fs_reserved_sectors = val;

	/*
	 * Number of FATs.
	 */
	val = buf[16];
	if (val != 1 && val != 2)
		return 1;
	this_fat->fs_tables = val;

	/*
	 * Root directory entries.
	 */
	this_fat->fs_directory_entries = LE16(&buf[17]);
	this_fat->fs_directory_sectors = 0;

	if (this_fat->fs_directory_entries) {
		unsigned int t1 = this_fat->fs_directory_entries * 32;
		unsigned int t2 = t1 / this_fat->fs_bytes_per_sector;

		if ((t1 % this_fat->fs_bytes_per_sector) != 0)
			return 1;
		this_fat->fs_directory_sectors = t2;
	}

	/*
	 * Total sectors.
	 */
	this_fat->fs_total_sectors = LE16(&buf[19]);
	val = LE32(&buf[32]);

	if (this_fat->fs_total_sectors != 0) {
		if (val != 0 && val != this_fat->fs_total_sectors)
			return 1;
		val = this_fat->fs_total_sectors;
	} else {
		this_fat->fs_total_sectors = val;
	}
	if (block_total < ((size_t)val / this_fat->io_mul))
		return 1;

	/*
	 * Sectors per FAT.
	 */
	val = LE16(&buf[22]);
	this_fat->fs_table_sectors = (val != 0) ? val : LE32(&buf[36]);
	val = this_fat->fs_bytes_per_sector;
	if (this_fat->fs_table_sectors > (0x40000000 / val))
		return 1;

	/*
	 * Single FAT size (reserved space on disk) in bytes.
	 */
	this_fat->fs_table_size = this_fat->fs_table_sectors * val;

	/*
	 * Number of data sectors and clusters.
	 */
	{
		unsigned int t1 = this_fat->fs_reserved_sectors;
		unsigned int t2 = this_fat->fs_table_sectors;
		unsigned int t3 = this_fat->fs_tables;
		unsigned int t4 = this_fat->fs_directory_sectors;
		unsigned int t5 = this_fat->fs_cluster_sectors;
		unsigned int t6 = this_fat->fs_total_sectors;

		if ((t1 + (t2 * t3) + t4 + t5) > t6)
			return 1;

		val = t6 - (t1 + (t2 * t3) + t4);

		this_fat->fs_data_sectors = val;
		this_fat->fs_clusters = val / t5;

		if (this_fat->fs_clusters == 0)
			return 1;
	}

	/*
	 * Determine the FAT type.
	 */
	val = this_fat->fs_clusters;
	this_fat->type_12 = 0, this_fat->type_16 = 0, this_fat->type_32 = 0;

	if (val < 4085)
		this_fat->type_12 = 1;
	else if (val < 65525)
		this_fat->type_16 = 1;
	else
		this_fat->type_32 = 1;

	this_fat->fs_root_cluster = 0;

	this_fat->fs_info_sector = 0;
	this_fat->fs_info_free = 0xFFFFFFFF;
	this_fat->fs_info_next = 0xFFFFFFFF;

	/*
	 * Type-specific tests and logic.
	 */
	if (this_fat->type_12 == 1 || this_fat->type_16 == 1) {
		if (this_fat->fs_directory_entries == 0)
			return 1;

		if (this_fat->fs_table_size > 0x20000)
			return 1;

		if (this_fat->type_12 == 1)
			val = (((this_fat->fs_clusters + 2) * 3) + 1) / 2;
		else
			val = (this_fat->fs_clusters + 2) * 2;

		if (this_fat->fs_table_size < val)
			return 1;

	} else {
		if (this_fat->fs_directory_entries != 0)
			return 1;
		if (LE16(&buf[19]) != 0 || LE16(&buf[22]) != 0)
			return 1;
		/*
		 * Check the file system version.
		 */
		if (LE16(&buf[42]) != 0)
			return 1;

		this_fat->fs_root_cluster = LE32(&buf[44]);

		if (this_fat->fs_root_cluster < 2)
			return 1;
		if (this_fat->fs_root_cluster - 2 >= this_fat->fs_clusters)
			return 1;

		if (this_fat->fs_clusters + 2 > 0x0FFFFFF8)
			return 1;

		val = (this_fat->fs_clusters + 2) * 4;

		if (this_fat->fs_table_size < val)
			return 1;

		this_fat->fs_info_sector = LE16(&buf[48]);

		if (this_fat->fs_info_sector == 0)
			return 1;
		if (this_fat->fs_info_sector >= this_fat->fs_reserved_sectors)
			return 1;
	}

	val = this_fat->fs_reserved_sectors;
	val += (this_fat->fs_tables * this_fat->fs_table_sectors);
	this_fat->fs_last_table_sector = val - 1;

	/*
	 * Allocate the cluster buffer.
	 */
	val = this_fat->fs_bytes_per_sector * this_fat->fs_cluster_sectors;
	if ((this_fat->cluster_buffer = malloc((size_t)val)) == NULL)
		return 1;

	this_fat->cluster_size = val;
	this_fat->cluster_sector = 0;

	return 0;
}

static int write_table(void *fat)
{
	unsigned char *table_sectors = this_fat->table_sectors;
	unsigned int fs_bytes_per_sector = this_fat->fs_bytes_per_sector;
	unsigned int fs_reserved_sectors = this_fat->fs_reserved_sectors;
	unsigned int fs_table_sectors = this_fat->fs_table_sectors;
	unsigned int table_entries = this_fat->table_entries;
	unsigned char *buffer;
	size_t lba, size;
	unsigned int i;

	/*
	 * The primary table.
	 */
	for (i = 0; i < table_entries; /* void */) {
		if (table_sectors[i] == 0) {
			i += 1;
			continue;
		}

		buffer = this_fat->table_buffer + (i * fs_bytes_per_sector);

		lba = (size_t)(fs_reserved_sectors + i);
		lba += (size_t)(this_fat->table_offset / fs_bytes_per_sector);
		lba *= this_fat->io_mul;
		size = fs_bytes_per_sector;

		i += 1;

		while (i < table_entries) {
			if (table_sectors[i] == 0)
				break;

			i += 1;
			size += fs_bytes_per_sector;
		}

		if (fat_io_write(this_fat->id, lba, &size, buffer))
			return this_fat->ready = 0, FAT_BLOCK_WRITE_ERROR;
	}

	if (this_fat->fs_tables < 2) {
		memset(table_sectors, 0, (size_t)table_entries);
		return 0;
	}

	/*
	 * The secondary table.
	 */
	for (i = 0; i < table_entries; /* void */) {
		if (table_sectors[i] == 0) {
			i += 1;
			continue;
		}

		buffer = this_fat->table_buffer + (i * fs_bytes_per_sector);

		lba = (size_t)(fs_reserved_sectors + i + fs_table_sectors);
		lba += (size_t)(this_fat->table_offset / fs_bytes_per_sector);
		lba *= this_fat->io_mul;
		size = fs_bytes_per_sector;

		i += 1;

		while (i < table_entries) {
			if (table_sectors[i] == 0)
				break;

			i += 1;
			size += fs_bytes_per_sector;
		}

		(void)fat_io_write(this_fat->id, lba, &size, buffer);
	}

	memset(table_sectors, 0, (size_t)table_entries);

	return 0;
}

static int read_full_table(void *fat)
{
	size_t table_sectors = this_fat->fs_table_sectors;
	size_t lba = this_fat->io_mul * (size_t)this_fat->fs_reserved_sectors;
	size_t size = (size_t)this_fat->fs_table_size;

	this_fat->table_buffer = malloc(size);
	if (this_fat->table_buffer == NULL)
		return 1;

	this_fat->table_sectors = malloc(table_sectors);
	if (this_fat->table_sectors == NULL) {
		free(this_fat->table_buffer);
		this_fat->table_buffer = NULL;
		return 1;
	}

	memset(this_fat->table_buffer, 0, size);
	memset(this_fat->table_sectors, 0, table_sectors);

	this_fat->table_entries = (unsigned int)table_sectors;

	if (fat_io_read(this_fat->id, lba, &size, this_fat->table_buffer))
		return 1;

	return 0;
}

static int read_table_buffer(void *fat, unsigned int *table_offset)
{
	const size_t table_buffer_size = 0x00020000;
	const unsigned int table_mask1 = 0x0001FFFF;
	const unsigned int table_mask2 = 0xFFFE0000;
	unsigned int fs_bytes_per_sector;
	unsigned int off1, off2;
	size_t table_sectors;
	size_t lba, size;

	off1 = (*table_offset & table_mask1);
	off2 = (*table_offset & table_mask2);

	*table_offset = off1;

	if (this_fat->table_offset == off2)
		return 0;

	fs_bytes_per_sector = this_fat->fs_bytes_per_sector;
	table_sectors = (size_t)(table_buffer_size / fs_bytes_per_sector);

	if (this_fat->table_buffer == NULL) {
		this_fat->table_buffer = malloc(table_buffer_size);
		if (this_fat->table_buffer == NULL)
			return FAT_INCONSISTENT_STATE;

		this_fat->table_sectors = malloc(table_sectors);
		if (this_fat->table_sectors == NULL) {
			free(this_fat->table_buffer);
			this_fat->table_buffer = NULL;
			return FAT_INCONSISTENT_STATE;
		}

		memset(this_fat->table_buffer, 0, table_buffer_size);
		memset(this_fat->table_sectors, 0, table_sectors);

		this_fat->table_buffer_size = (unsigned int)table_buffer_size;
		this_fat->table_entries = (unsigned int)table_sectors;
		this_fat->table_offset = 0;

	} else {
		if (write_table(fat)) {
			memset(this_fat->table_buffer, 0, table_buffer_size);
			memset(this_fat->table_sectors, 0, table_sectors);
			return FAT_BLOCK_WRITE_ERROR;
		}
	}

	this_fat->table_offset = off2;

	lba = (size_t)this_fat->fs_reserved_sectors;
	lba += (size_t)(this_fat->table_offset / fs_bytes_per_sector);
	lba *= this_fat->io_mul;
	size = table_buffer_size;

	if ((this_fat->table_offset + size) > this_fat->fs_table_size) {
		memset(this_fat->table_buffer, 0, table_buffer_size);

		size = (size_t)(this_fat->table_offset + size);
		size -= (size_t)this_fat->fs_table_size;

		if (table_buffer_size < size)
			return this_fat->ready = 0, FAT_INCONSISTENT_STATE;

		size = table_buffer_size - size;
	}

	if (fat_io_read(this_fat->id, lba, &size, this_fat->table_buffer))
		return this_fat->ready = 0, FAT_BLOCK_READ_ERROR;

	return 0;
}

static unsigned int get_table_value(void *fat, unsigned int off)
{
	unsigned char *table_buffer = this_fat->table_buffer;
	unsigned int ret_val;

	if (off >= this_fat->fs_clusters + 2)
		return 1;

	if (this_fat->type_12) {
		table_buffer += (off + (off >> 1));
		ret_val = LE16(&table_buffer[0]);
		if (off & 1)
			ret_val >>= 4;
		ret_val &= 0x0FFF;
		if (ret_val >= 0x0FF8)
			ret_val |= 0x0FFFF000;
		return ret_val;
	}

	if (this_fat->type_16) {
		table_buffer += (off << 1);
		ret_val = LE16(&table_buffer[0]);
		if (ret_val >= 0xFFF8)
			ret_val |= 0x0FFF0000;
		return ret_val;
	}

	if (this_fat->type_32) {
		unsigned int add = (off << 2);

		if (read_table_buffer(fat, &add))
			return 1;

		table_buffer += add;
		ret_val = LE32(&table_buffer[0]) & 0x0FFFFFFF;
		return ret_val;
	}

	return 1;
}

static int set_table_value(void *fat, unsigned int off, unsigned int val)
{
	unsigned char *table_buffer = this_fat->table_buffer;
	unsigned char *table_sectors = this_fat->table_sectors;
	unsigned int fs_bytes_per_sector = this_fat->fs_bytes_per_sector;

	if (off >= this_fat->fs_clusters + 2)
		return FAT_INCONSISTENT_STATE;

	if (this_fat->type_12) {
		unsigned int add = off + (off >> 1);
		unsigned int old_val, sec;

		table_buffer += add;
		old_val = LE16(&table_buffer[0]);
		if (off & 1) {
			val = val << 4;
			val = (val & 0xFFF0) | (old_val & 0x000F);
		} else {
			val = (val & 0x0FFF) | (old_val & 0xF000);
		}

		sec = add / fs_bytes_per_sector;
		if (add == fs_bytes_per_sector - 1)
			table_sectors[sec + 1] = 1;

		table_sectors[sec] = 1;
		W_LE16(&table_buffer[0], val);
		return 0;
	}

	if (this_fat->type_16) {
		unsigned int add = (off << 1);

		table_buffer += add;
		table_sectors[add / fs_bytes_per_sector] = 1;
		W_LE16(&table_buffer[0], val);
		return 0;
	}

	if (this_fat->type_32) {
		unsigned int add = (off << 2);
		unsigned int old_val;
		int r;

		if ((r = read_table_buffer(fat, &add)) != 0)
			return r;

		table_buffer += add;
		table_sectors[add / fs_bytes_per_sector] = 1;

		old_val = LE32(&table_buffer[0]);

		if (this_fat->fs_info_free != 0xFFFFFFFF) {
			if ((old_val & 0x0FFFFFFF) == 0) {
				if (val != 0)
					this_fat->fs_info_free -= 1;
			} else {
				if (val == 0)
					this_fat->fs_info_free += 1;
			}
			if (val != 0)
				this_fat->fs_info_next = off;
		}

		val = (val & 0x0FFFFFFF) | (old_val & 0xF0000000);
		W_LE32(&table_buffer[0], val);
		return 0;
	}

	return FAT_NOT_READY;
}

static int read_block(void *fat, size_t lba)
{
	size_t size = (size_t)this_fat->block_size;

	if (lba == 0)
		return 1;
	if (this_fat->block_buffer_lba == lba)
		return 0;

	/*
	 * Check if the same lba content is in the cluster buffer.
	 */
	if (this_fat->cluster_buffer_lba == lba) {
		const void *src = this_fat->cluster_buffer;
		void *dst = this_fat->block_buffer;

		memcpy(dst, src, size);
		return 0;
	}

	if (fat_io_read(this_fat->id, lba, &size, this_fat->block_buffer)) {
		this_fat->block_buffer_lba = 0;
		return 1;
	}

	this_fat->block_buffer_lba = lba;

	return 0;
}

static int write_block(void *fat, size_t lba)
{
	size_t size = (size_t)this_fat->block_size;

	/*
	 * Invalidate the cluster buffer when writing block data.
	 */
	this_fat->cluster_buffer_lba = 0;

	if (fat_io_write(this_fat->id, lba, &size, this_fat->block_buffer))
		return 1;

	return 0;
}

static int read_cluster(void *fat, unsigned int cluster, int skip_io)
{
	unsigned char *cluster_buffer = this_fat->cluster_buffer;
	unsigned int fs_total_sectors = this_fat->fs_total_sectors;
	unsigned int fs_data_sectors = this_fat->fs_data_sectors;
	size_t lba, size;

	if ((cluster - 2) >= this_fat->fs_clusters)
		return FAT_INCONSISTENT_STATE;

	lba = (size_t)(fs_total_sectors - fs_data_sectors);
	lba += (size_t)((cluster - 2) * this_fat->fs_cluster_sectors);

	this_fat->cluster_sector = (unsigned int)lba;

	if (skip_io)
		return (int)(this_fat->cluster_buffer_lba = 0);

	lba *= this_fat->io_mul;
	size = this_fat->cluster_size;

	if (this_fat->cluster_buffer_lba == lba)
		return 0;

	if (fat_io_read(this_fat->id, lba, &size, cluster_buffer)) {
		memset(cluster_buffer, 0, (size_t)this_fat->cluster_size);
		this_fat->cluster_buffer_lba = 0;
		return FAT_BLOCK_READ_ERROR;
	}

	this_fat->cluster_buffer_lba = lba;

	return 0;
}

static int write_cluster(void *fat)
{
	const unsigned char *cluster_buffer = this_fat->cluster_buffer;
	size_t lba, size;

	if (this_fat->cluster_sector <= this_fat->fs_last_table_sector)
		return FAT_INCONSISTENT_STATE;

	lba = (size_t)this_fat->cluster_sector;
	lba *= this_fat->io_mul;
	size = this_fat->cluster_size;

	/*
	 * Invalidate the block buffer when writing cluster data.
	 */
	this_fat->block_buffer_lba = 0;

	if (fat_io_write(this_fat->id, lba, &size, cluster_buffer))
		return FAT_BLOCK_WRITE_ERROR;

	return 0;
}

static int write_record_buffer(void *fat, int fd)
{
	struct fat_fd *fd_entry = &this_fat->fd[fd];
	unsigned char *block_buffer = this_fat->block_buffer;
	unsigned char *record = &fd_entry->record[0];
	unsigned int record_offset = fd_entry->record_offset;
	unsigned int record_sector = fd_entry->record_sector;
	size_t lba;

	if (record_offset > this_fat->fs_bytes_per_sector)
		return FAT_INCONSISTENT_STATE;

	if (record_sector <= this_fat->fs_last_table_sector)
		return FAT_INCONSISTENT_STATE;

	lba = record_sector;
	lba *= this_fat->io_mul;

	if (read_block(fat, lba))
		return FAT_BLOCK_READ_ERROR;

	memcpy(&block_buffer[record_offset], record, 32);

	if (write_block(fat, lba))
		return FAT_BLOCK_WRITE_ERROR;

	return 0;
}

static int allocate_cluster(void *fat, unsigned int *cluster)
{
	unsigned int clusters = this_fat->fs_clusters + 2;
	unsigned int i;

	if (this_fat->last_allocated > 2) {
		for (i = this_fat->last_allocated; i < clusters; i++) {
			if (get_table_value(fat, i) == 0) {
				this_fat->last_allocated = i;
				return *cluster = i, 0;
			}
		}
	}

	for (i = 2; i < clusters; i++) {
		if (get_table_value(fat, i) == 0) {
			this_fat->last_allocated = i;
			return *cluster = i, 0;
		}
	}

	return *cluster = 0, FAT_NOT_ENOUGH_SPACE;
}

static int append_cluster(void *fat, unsigned int current_cluster, int fast)
{
	const unsigned int end = 0x0FFFFFFF;
	unsigned int cluster;
	int r;

	cluster = get_table_value(fat, current_cluster);
	if (cluster > 2 && cluster < 0x0FFFFFF8)
		return FAT_INCONSISTENT_STATE;

	if (allocate_cluster(fat, &cluster))
		return FAT_NOT_ENOUGH_SPACE;

	if (fast == 0) {
		size_t cluster_size = (size_t)this_fat->cluster_size;

		if ((r = read_cluster(fat, cluster, 1)) != 0)
			return r;

		memset(this_fat->cluster_buffer, 0, cluster_size);

		if ((r = write_cluster(fat)) != 0)
			return r;
	}

	if ((r = set_table_value(fat, cluster, end)) != 0)
		return r;
	if (fast == 0 && (r = write_table(fat)) != 0)
		return r;

	if ((r = set_table_value(fat, current_cluster, cluster)) != 0)
		return r;
	if (fast == 0 && (r = write_table(fat)) != 0)
		return r;

	return 0;
}

static void delete_clusters(void *fat, unsigned int start_cluster)
{
	size_t cluster_size = (size_t)this_fat->cluster_size;
	unsigned int cluster = start_cluster;
	unsigned int next_cluster;

	while (cluster >= 2 && cluster < 0x0FFFFFF8) {
		if (read_cluster(fat, cluster, 1) == 0) {
			memset(this_fat->cluster_buffer, 0, cluster_size);
			(void)write_cluster(fat);
		}

		next_cluster = get_table_value(fat, cluster);
		(void)set_table_value(fat, cluster, 0);

		cluster = next_cluster;
	}

	this_fat->last_allocated = 0;
	(void)write_table(fat);
}

static void delete_extra_clusters(void *fat, int fd)
{
	unsigned char *record = get_record(fat, fd);
	unsigned int cluster_size = this_fat->cluster_size;
	unsigned int cluster, clusters, file_size;
	unsigned int i;

	cluster = (LE16(&record[20]) << 16) | LE16(&record[26]);
	file_size = LE32(&record[28]);

	this_fat->fd[fd].cluster_idx = 0;
	this_fat->fd[fd].cluster_val = 0;

	if (file_size == 0) {
		W_LE16(&record[20], 0);
		W_LE16(&record[26], 0);

		if (write_record_buffer(fat, fd)) {
			unsigned int cluster_hi = cluster >> 16;
			unsigned int cluster_lo = cluster & 0xFFFF;

			W_LE16(&record[20], cluster_hi);
			W_LE16(&record[26], cluster_lo);
			return;
		}
		delete_clusters(fat, cluster);
		return;
	}

	/*
	 * Calculate how many clusters are needed.
	 */
	clusters = file_size / cluster_size;
	if ((file_size % cluster_size) != 0)
		clusters += 1;

	/*
	 * Find the last cluster according to the file size.
	 */
	for (i = 0; i < clusters; i++) {
		unsigned int next = get_table_value(fat, cluster);

		if (!(i + 1 < clusters)) {
			if (set_table_value(fat, cluster, 0x0FFFFFFF))
				return;

			if ((file_size % cluster_size) != 0) {
				unsigned char *buf = this_fat->cluster_buffer;
				unsigned int used;
				size_t size;

				used = file_size % cluster_size;
				size = (size_t)(cluster_size - used);

				if (read_cluster(fat, cluster, 0) == 0) {
					memset(&buf[used], 0, size);
					(void)write_cluster(fat);
				}
			}
		}
		cluster = next;
	}

	if (cluster >= 2 && cluster < 0x0FFFFFF8)
		delete_clusters(fat, cluster);
}

static int check_reserved_name(const char *name)
{
	static const struct {
		const char *name;
		size_t length;
	} names[] = {
		{ "AUX " , 4 }, { "COM1 ", 5 }, { "COM2 ", 5 },
		{ "COM3 ", 5 }, { "COM4 ", 5 }, { "COM5 ", 5 },
		{ "COM6 ", 5 }, { "COM7 ", 5 }, { "COM8 ", 5 },
		{ "COM9 ", 5 }, { "CON " , 4 }, { "LPT1 ", 5 },
		{ "LPT2 ", 5 }, { "LPT3 ", 5 }, { "LPT4 ", 5 },
		{ "LPT5 ", 5 }, { "LPT6 ", 5 }, { "LPT7 ", 5 },
		{ "LPT8 ", 5 }, { "LPT9 ", 5 }, { "NUL " , 4 },
		{ "PRN " , 4 }
	};
	size_t i;

	if (name[8] != ' ')
		return 0;

	for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
		if (!fat_strncmp(name, names[i].name, names[i].length))
			return 1;
	}

	return 0;
}

static int fill_path_buffer(void *fat, const char *name)
{
	struct fat_name *path = this_fat->path_buffer;
	size_t entries = 0, i, j;
	int state = 0;

	this_fat->path_entries = 0;
	memset(&path[entries], 0, sizeof(path[0]));

	for (i = 0; name[i] != '\0'; i++) {
		int length = (int)path[entries].length;
		char c = name[i];

		if (c <= 0x20 || c > 0x7E || entries >= 255)
			return 1;

		if (c >= 0x61 && c <= 0x7A)
			c = (char)((int)c - 32);

		if (c == '<' || c == '>' || c == ':' || c == '"')
			return 1;
		if (c == '\\' || c == '|' || c == '?' || c == '*')
			return 1;

		if (c != '/') {
			if (length > 11)
				return 1;
			path[entries].name[length++] = c;
			path[entries].length = (char)length;
		}

		if (state == 0 && c == '.') {
			state = 1;
			if (name[i + 1] != '\0')
				continue;
			c = '/';
		}
		if (state == 1 && c == '/') {
			/*
			 * Current directory.
			 */
			memset(&path[entries], 0, sizeof(path[0]));
			state = 0;
			continue;
		}
		if (state == 1 && c == '.') {
			state = 2;
			if (name[i + 1] != '\0')
				continue;
			c = '/';
		}
		if (state == 2 && c == '/') {
			/*
			 * Parent directory.
			 */
			memset(&path[entries], 0, sizeof(path[0]));
			if (entries > 0) {
				entries -= 1;
				memset(&path[entries], 0, sizeof(path[0]));
			}
			state = 0;
			continue;
		}

		state = 3;

		if (c == '/') {
			if (length > 0) {
				path[entries].is_dir = 1;
				memset(&path[++entries], 0, sizeof(path[0]));
			}
			state = 0;
			continue;
		}

		if (name[i + 1] == '\0') {
			entries += 1;
			break;
		}
	}

	for (i = 0; i < entries; i++) {
		for (j = 11; j > 0; j--) {
			if (path[i].name[j] == '.')
				break;
		}

		if (j > 8 || (j == 0 && path[i].length > 8))
			return 1;

		if (j != 0) {
			char name_ext[3];

			if (((int)path[i].length - (int)j) > 4)
				return 1;

			name_ext[0] = path[i].name[j + 1];
			name_ext[1] = path[i].name[j + 2];
			name_ext[2] = path[i].name[j + 3];

			path[i].name[j + 0] = '\0';
			path[i].name[j + 1] = '\0';
			path[i].name[j + 2] = '\0';
			path[i].name[j + 3] = '\0';

			path[i].name[ 8] = name_ext[0];
			path[i].name[ 9] = name_ext[1];
			path[i].name[10] = name_ext[2];
		}

		for (j = 0; j < 11; j++) {
			if (path[i].name[j] == '\0')
				path[i].name[j] = ' ';
		}

		path[i].name[11] = '\0';

		/*
		 * Do not allow a name, even with a valid extension,
		 * that consists of periods (.) only.
		 */
		for (j = 0; /* void */; j++) {
			char c = path[i].name[j];
			if (c != ' ' && c != '.')
				break;
			if (j == 7)
				return 1;
		}

		if (check_reserved_name(&path[i].name[0]))
			return 1;
	}

	this_fat->path_entries = (unsigned int)entries;
	return (entries == 0) ? 1 : 0;
}

static int iterate_fixed_root_dir(void *fat, int fd, int find_empty)
{
	struct fat_fd *fd_entry = &this_fat->fd[fd];
	unsigned char *buf = this_fat->block_buffer;
	unsigned char *record = &fd_entry->record[0];
	const struct fat_name *path = this_fat->path_buffer;
	unsigned int fs_bytes_per_sector = this_fat->fs_bytes_per_sector;
	unsigned int fs_directory_sectors = this_fat->fs_directory_sectors;
	unsigned int i, j, k, sector;

	fd_entry->record_offset = 0;
	fd_entry->record_sector = 0;

	sector = this_fat->fs_reserved_sectors;
	sector += (this_fat->fs_tables * this_fat->fs_table_sectors);

	for (i = 0; i < fs_directory_sectors; i++) {
		size_t lba = (size_t)(sector * this_fat->io_mul);

		if (read_block(fat, lba))
			return FAT_BLOCK_READ_ERROR;

		for (j = 0; j < fs_bytes_per_sector; j += 32) {
			if (find_empty) {
				if (buf[j] == 0 || buf[j] == 0xE5) {
					memcpy(record, &buf[j], 32);
					fd_entry->record_offset = j;
					fd_entry->record_sector = sector;
					return 0;
				}
			} else {
				const char *name1 = &path[0].name[0];
				const char *name2 = (const char *)&buf[j];
				int record_found = 1;

				for (k = 0; k < 11; k++) {
					if (name1[k] != name2[k]) {
						record_found = 0;
						break;
					}
				}

				if ((buf[j + 11] & 0x08) != 0)
					record_found = 0;

				if (record_found) {
					memcpy(record, &buf[j], 32);
					fd_entry->record_offset = j;
					fd_entry->record_sector = sector;
					return 0;
				}
			}
			if (buf[j] == 0)
				return -1;
		}
		sector += 1;
	}

	return -1;
}

static int iterate_dir(void *fat, int fd, int path_i, unsigned int cluster)
{
	int find_empty = (path_i < 0) ? 1 : 0;
	struct fat_fd *fd_entry = &this_fat->fd[fd];
	const unsigned char *buf = this_fat->cluster_buffer;
	const struct fat_name *path = this_fat->path_buffer;
	unsigned int fs_bytes_per_sector = this_fat->fs_bytes_per_sector;
	unsigned int cluster_size = this_fat->cluster_size;
	unsigned int iterate_limit = (65536 * 32) / cluster_size;

	while (iterate_limit) {
		unsigned int j, k;
		unsigned int next_cluster;
		int r;

		if ((r = read_cluster(fat, cluster, 0)) != 0)
			return r;

		for (j = 0; j < cluster_size; j += 32) {
			unsigned char *record;
			unsigned int offset, sector;

			if (find_empty) {
				if (buf[j] == 0 || buf[j] == 0xE5) {
					offset = j % fs_bytes_per_sector;
					sector = j / fs_bytes_per_sector;
					sector += this_fat->cluster_sector;

					record = &fd_entry->record[0];

					memcpy(record, &buf[j], 32);
					fd_entry->record_offset = offset;
					fd_entry->record_sector = sector;
					return 0;
				}
			} else {
				const char *name1 = &path[path_i].name[0];
				const char *name2 = (const char *)&buf[j];
				int record_found = 1;

				for (k = 0; k < 11; k++) {
					if (name1[k] != name2[k]) {
						record_found = 0;
						break;
					}
				}

				if ((buf[j + 11] & 0x08) != 0)
					record_found = 0;

				if (record_found) {
					offset = j % fs_bytes_per_sector;
					sector = j / fs_bytes_per_sector;
					sector += this_fat->cluster_sector;

					record = &fd_entry->record[0];

					memcpy(record, &buf[j], 32);
					fd_entry->record_offset = offset;
					fd_entry->record_sector = sector;
					return 0;
				}
			}
			if (buf[j] == 0)
				return -1;
		}

		next_cluster = get_table_value(fat, cluster);
		if (next_cluster < 2 || next_cluster >= 0x0FFFFFF8)
			break;
		cluster = next_cluster;
		iterate_limit -= 1;
	}

	/*
	 * Extend the directory if an empty record was not found.
	 */
	if (find_empty && iterate_limit)
		(void)append_cluster(fat, cluster, 0);

	return -1;
}

static int fill_record_buffer(void *fat, int fd)
{
	struct fat_fd *fd_entry = &this_fat->fd[fd];
	int mode_create = (fd_entry->mode & MODE_CREATE) ? 1 : 0;
	const struct fat_name *path = this_fat->path_buffer;
	unsigned int cluster = this_fat->fs_root_cluster;
	int i;

	for (i = 0; i < (int)this_fat->path_entries; i++) {
		int create_record = 0;
		unsigned int cluster_hi, cluster_lo;
		unsigned char *record;
		int r = 0;

		if (i == 0 && this_fat->type_32 == 0) {
			if ((r = iterate_fixed_root_dir(fat, fd, 0)) == -1) {
				if (mode_create == 0)
					return FAT_FILE_NOT_FOUND;
				r = iterate_fixed_root_dir(fat, fd, 1);
				create_record = 1;
			}
		} else {
			if ((r = iterate_dir(fat, fd, i, cluster)) == -1) {
				if (mode_create == 0)
					return FAT_FILE_NOT_FOUND;
				r = iterate_dir(fat, fd, -1, cluster);
				if (r != 0)
					r = iterate_dir(fat, fd, -1, cluster);
				create_record = 1;
			}
		}

		if (r > 0)
			return r;

		record = get_record(fat, fd);

		/*
		 * Check that the record is not already in use.
		 */
		if (i + 1 == (int)this_fat->path_entries) {
			int fd_entries = (int)this_fat->fd_entries;
			int j;

			for (j = 0; j < fd_entries; j++) {
				if (j != fd && this_fat->fd[j].opened != 0) {
					unsigned int off1, off2;
					unsigned int sec1, sec2;

					off1 = this_fat->fd[j].record_offset;
					sec1 = this_fat->fd[j].record_sector;

					off2 = this_fat->fd[fd].record_offset;
					sec2 = this_fat->fd[fd].record_sector;

					if (off1 == off2 && sec1 == sec2)
						return FAT_FILE_ALREADY_OPEN;
				}
			}
		}

		if (create_record) {
			unsigned int new_cluster = 0;

			/*
			 * Set mode bit 31 if created.
			 */
			fd_entry->mode |= 0x80000000;

			if (path[i].is_dir) {
				const unsigned int end = 0x0FFFFFFF;
				unsigned char *dir = this_fat->cluster_buffer;

				if (allocate_cluster(fat, &new_cluster))
					return FAT_NOT_ENOUGH_SPACE;

				r = read_cluster(fat, new_cluster, 1);
				if (r != 0)
					return r;

				memset(dir, 0, this_fat->cluster_size);

				/*
				 * Create the "." record.
				 */
				W_LE32(&dir[0], 0x2020202E);
				W_LE32(&dir[4], 0x20202020);
				W_LE32(&dir[8], 0x10202020);
				write_timestamps(dir, 1, 1);

				cluster_hi = new_cluster >> 16;
				cluster_lo = new_cluster & 0xFFFF;

				W_LE16(&dir[20], cluster_hi);
				W_LE16(&dir[26], cluster_lo);

				/*
				 * Create the ".." record.
				 */
				memcpy(&dir[32], &dir[0], 32);
				W_LE16(&dir[32], 0x2E2E);

				if (i != 0) {
					cluster_hi = cluster >> 16;
					cluster_lo = cluster & 0xFFFF;
				} else {
					cluster_hi = 0;
					cluster_lo = 0;
				}

				W_LE16(&dir[52], cluster_hi);
				W_LE16(&dir[58], cluster_lo);

				/*
				 * Write the cluster and update the table.
				 */
				r = write_cluster(fat);
				if (r != 0)
					return r;

				r = set_table_value(fat, new_cluster, end);
				if (r != 0)
					return r;

				r = write_table(fat);
				if (r != 0)
					return r;

				/*
				 * Use the "." record as a template.
				 */
				memcpy(&record[0], &dir[0], 32);
			} else {
				write_timestamps(record, 1, 1);
				record[11] = 0x00;
			}

			record[12] = 0x00;
			memcpy(record, &path[i].name[0], 11);

			cluster_hi = new_cluster >> 16;
			cluster_lo = new_cluster & 0xFFFF;

			W_LE16(&record[20], cluster_hi);
			W_LE16(&record[26], cluster_lo);
			W_LE32(&record[28], 0);

			r = write_record_buffer(fat, fd);
			if (r != 0)
				return r;
		}

		cluster_hi = LE16(&record[20]);
		cluster_lo = LE16(&record[26]);

		cluster = (cluster_hi << 16) | cluster_lo;
	}

	return 0;
}

static int read_fixed_root_dir(void *fat, int fd, size_t *size, void *buf)
{
	size_t requested_size;
	unsigned char *buf_ptr = buf;
	unsigned char *block_buffer = this_fat->block_buffer;
	unsigned int dir_size = this_fat->fs_directory_entries * 32;
	unsigned int read_bytes_limit, read_bytes = 0;
	unsigned int offset = this_fat->fd[fd].offset;
	unsigned int sector;
	int r = 0;

	/*
	 * It has been tested that requested size is not zero.
	 */
	requested_size = *size;
	*size = 0;

	if (offset >= dir_size) {
		if (buf_ptr != NULL)
			memset(buf_ptr, 0, requested_size);
		this_fat->fd[fd].eof = 1;
		return 0;
	}

	if (requested_size > (dir_size - offset))
		read_bytes_limit = (dir_size - offset);
	else
		read_bytes_limit = (unsigned int)requested_size;

	sector = this_fat->fs_reserved_sectors;
	sector += (this_fat->fs_tables * this_fat->fs_table_sectors);
	sector += (offset / this_fat->fs_bytes_per_sector);

	offset = (offset % this_fat->fs_bytes_per_sector);

	/*
	 * Read the data. The destination buffer can be NULL.
	 */
	while (read_bytes < read_bytes_limit) {
		size_t lba = (size_t)sector * this_fat->io_mul;
		size_t sector_size = this_fat->fs_bytes_per_sector;

		if (sector < this_fat->fs_last_table_sector) {
			r = FAT_INCONSISTENT_STATE;
			break;
		}

		if (read_block(fat, lba)) {
			r = FAT_BLOCK_READ_ERROR;
			break;
		}

		if (buf_ptr != NULL) {
			while (offset < sector_size) {
				*buf_ptr++ = block_buffer[offset++];
				if (!(++read_bytes < read_bytes_limit))
					break;
			}
		} else {
			while ((offset++) < sector_size) {
				if (!(++read_bytes < read_bytes_limit))
					break;
			}
		}

		offset = 0;
		sector += 1;
	}

	if (read_bytes < requested_size) {
		size_t buf_end_size = requested_size - (size_t)read_bytes;
		if (buf_ptr != NULL)
			memset(buf_ptr, 0, buf_end_size);
	}

	this_fat->fd[fd].offset += read_bytes;

	if (this_fat->fd[fd].offset >= dir_size)
		this_fat->fd[fd].eof = 1;

	*size = read_bytes;

	return r;
}

static void compute_fs_info(void *fat)
{
	unsigned char *ptr, *table_buffer = this_fat->table_buffer;
	unsigned int table_buffer_size = this_fat->table_buffer_size;
	unsigned int table_entries = this_fat->fs_clusters + 2;
	unsigned int fs_info_free = 0, fs_info_next = 0xFFFFFFFF;
	unsigned int offset = 0, total_offset = 0;
	unsigned int i, val, prev = 0;

	if (read_table_buffer(fat, &offset))
		return;

	for (i = 0; i < table_entries; i++) {
		ptr = &table_buffer[offset];
		val = (LE32(&ptr[0]) & 0x0FFFFFFF);

		if (val == 0) {
			fs_info_free += 1;
			if (prev >= 2) {
				if (prev <= 0x0FFFFFEF || prev >= 0x0FFFFFF8)
					fs_info_next = (i + 2) - 1;
			}
		}

		offset += 4;
		total_offset += 4;

		prev = val;

		if (offset >= table_buffer_size) {
			offset = total_offset;
			if (read_table_buffer(fat, &offset))
				return;
		}
	}

	this_fat->fs_info_free = fs_info_free;
	this_fat->fs_info_next = fs_info_next;
}

static void write_fs_info(void *fat)
{
	unsigned char *buf = this_fat->block_buffer;
	size_t lba = (size_t)this_fat->fs_info_sector * this_fat->io_mul;

	unsigned int val_free = this_fat->fs_info_free;
	unsigned int val_next = this_fat->fs_info_next;
	unsigned int old_free, old_next;

	this_fat->fs_info_free = 0xFFFFFFFF;
	this_fat->fs_info_next = 0xFFFFFFFF;

	if (read_block(fat, lba))
		return;

	if (LE32(&buf[0]) != 0x41615252)
		return;
	if (LE32(&buf[484]) != 0x61417272)
		return;
	if (LE32(&buf[508]) != 0xAA550000)
		return;

	old_free = LE32(&buf[488]);
	old_next = LE32(&buf[492]);

	if (old_free != 0 && old_free <= this_fat->fs_clusters) {
		if (old_next >= 2 && old_next < this_fat->fs_clusters + 2) {
			this_fat->fs_info_free = old_free;
			this_fat->fs_info_next = old_next;
		}
	}

	while (val_next > 2 && val_next < this_fat->fs_clusters + 2) {
		unsigned int val = get_table_value(fat, val_next);

		if (val >= 2 && val <= 0x0FFFFFEF)
			break;
		val_next -= 1;
	}

	W_LE32(&buf[488], val_free);
	W_LE32(&buf[492], val_next);

	(void)write_block(fat, lba);
}

static int parse_mode(void *fat, int fd, const char *mode)
{
	struct fat_fd *fd_entry = &this_fat->fd[fd];
	int mode_a = 0, mode_b = 0, mode_r = 0, mode_w = 0, mode_plus = 0;
	int c;

	fd_entry->mode = 0;

	while ((c = (int)*mode++) != '\0') {
		int *m = NULL;

		switch (c) {
		case 'a':
			m = &mode_a;
			fd_entry->mode |= MODE_APPEND;
			fd_entry->mode |= MODE_CREATE;
			fd_entry->mode |= MODE_MODIFY;
			break;
		case 'b':
			m = &mode_b;
			break;
		case 'r':
			m = &mode_r;
			fd_entry->mode |= MODE_READ;
			break;
		case 'w':
			m = &mode_w;
			fd_entry->mode |= MODE_CREATE;
			fd_entry->mode |= MODE_MODIFY;
			fd_entry->mode |= MODE_WRITE;
			break;
		case '+':
			m = &mode_plus;
			fd_entry->mode |= MODE_MODIFY;
			break;
		default:
			break;
		}
		if (m == NULL || *m != 0)
			return fd_entry->mode = 0, 1;
		*m = 1;
	}

	if (mode_a + mode_r + mode_w != 1)
		return fd_entry->mode = 0, 1;

	return 0;
}

int fat_create(void **instance, int id)
{
	struct fat_instance *fat;
	const size_t block_buffer_size = 4096;
	const size_t path_buffer_size = (256 * sizeof(struct fat_name));
	size_t fd_size = (9 * sizeof(struct fat_fd));

	fat = malloc(sizeof(*fat));
	if ((*instance = fat) == NULL)
		return 1;

	memset(fat, 0, sizeof(*fat));

	fat->fd = NULL;
	fat->block_buffer = NULL;
	fat->cluster_buffer = NULL;
	fat->table_buffer = NULL;
	fat->table_sectors = NULL;
	fat->path_buffer = NULL;

	fat->fd_entries = (unsigned int)(fd_size / sizeof(struct fat_fd));
	fat->id = id;

	if ((fat->fd = malloc(fd_size)) == NULL)
		return !fat_delete(fat);

	if ((fat->block_buffer = malloc(block_buffer_size)) == NULL)
		return !fat_delete(fat);

	if ((fat->path_buffer = malloc(path_buffer_size)) == NULL)
		return !fat_delete(fat);

	memset(fat->fd, 0, fd_size);
	memset(fat->block_buffer, 0, block_buffer_size);
	memset(fat->path_buffer, 0, path_buffer_size);

	if (read_bios_parameter_block(fat, id))
		return !fat_delete(fat);

	if (fat->type_32 != 0) {
		unsigned int table_offset = 0;

		fat->table_offset = 0xF0000000;

		if (read_table_buffer(fat, &table_offset))
			return !fat_delete(fat);

	} else {
		if (read_full_table(fat))
			return !fat_delete(fat);
	}

	/*
	 * Reserve the last fd_entry.
	 */
	fat->fd_entries -= 1;
	fat->fd_special = fat->fd_entries;

	fat->maximum_file_size = 0xFFFFFFFF;
	fat->ready = FAT_READY;

	return 0;
}

int fat_delete(void *fat)
{
	if (fat == NULL)
		return 1;

	if (this_fat->fs_info_modified) {
		if (this_fat->fs_info_free == 0xFFFFFFFF)
			compute_fs_info(fat);
		write_fs_info(fat);
	}

	free(this_fat->fd);
	free(this_fat->block_buffer);
	free(this_fat->cluster_buffer);
	free(this_fat->table_buffer);
	free(this_fat->table_sectors);
	free(this_fat->path_buffer);

	this_fat->ready = 0;
	free(fat);

	return 0;
}

int fat_close(void *fat, int fd)
{
	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 1) != 0 && check_fd(fat, fd, 0) != 0)
		return FAT_INVALID_PARAMETERS;

	this_fat->fd[fd].opened = 0;

	return 0;
}

int fat_control(void *fat, int fd, int write, unsigned char record[32])
{
	unsigned char *record_ptr;

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 1) != 0)
		return FAT_INVALID_PARAMETERS;

	if ((this_fat->fd[fd].mode & MODE_ROOT_DIR) != 0)
		return FAT_INVALID_PARAMETERS;

	record_ptr = get_record(fat, fd);

	if (write == 0) {
		memcpy(&record[0], &record_ptr[0], 32);
		return 0;
	}

	/*
	 * The read-only, hidden, system, and archive
	 * attributes can be modified by this function.
	 */
	{
		unsigned int v1 = record_ptr[11] & 0xD8;
		unsigned int v2 = record[11] & 0x27;

		record_ptr[11] = (unsigned char)(v1 | v2);
	}

	/*
	 * Write all date and time fields.
	 */
	memcpy(&record_ptr[13], &record[13], 7);
	memcpy(&record_ptr[22], &record[22], 4);

	if (write_record_buffer(fat, fd))
		return FAT_BLOCK_WRITE_ERROR;

	/*
	 * The file size can be truncated.
	 */
	if ((record_ptr[11] & 0x10) == 0) {
		unsigned int old_size = LE32(&record_ptr[28]);
		unsigned int new_size = LE32(&record[28]);

		if (new_size < old_size) {
			W_LE32(&record_ptr[28], new_size);

			if (write_record_buffer(fat, fd)) {
				W_LE32(&record_ptr[28], old_size);
				return FAT_BLOCK_WRITE_ERROR;
			}

			delete_extra_clusters(fat, fd);
		}
	}

	return 0;
}

int fat_eof(void *fat, int fd)
{
	/*
	 * This function returns zero even if parameters are not valid.
	 */
	if (this_fat->ready != FAT_READY || check_fd(fat, fd, 1) != 0)
		return 0;

	return (this_fat->fd[fd].eof != 0);
}

int fat_open(void *fat, int fd, const char *name, const char *mode)
{
	unsigned int cluster;
	int r;

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 0) != 0 || mode == NULL)
		return FAT_INVALID_PARAMETERS;

	init_fat_fd(fat, fd);

	if (name == NULL || name[0] == '\0')
		return FAT_INVALID_FILE_NAME;

	if (parse_mode(fat, fd, mode))
		return FAT_INVALID_PARAMETERS;

	if (fill_path_buffer(fat, name)) {
		if (name[0] != '/' || name[1] != '.' || name[2] != '\0')
			return FAT_INVALID_FILE_NAME;
		this_fat->fd[fd].mode = MODE_ROOT_DIR;
	}

	if ((this_fat->fd[fd].mode & MODE_MODIFY) != 0) {
		if (this_fat->type_32 && !this_fat->fs_info_modified) {
			write_fs_info(fat);
			this_fat->fs_info_modified = 1;
		}
	}

	/*
	 * Fill the record buffer if the requested name
	 * is not the root directory.
	 */
	if ((this_fat->fd[fd].mode & MODE_ROOT_DIR) == 0) {
		if ((r = fill_record_buffer(fat, fd)) != 0)
			return r;
	}

	/*
	 * Truncate the data and write the fs info if needed.
	 */
	if ((this_fat->fd[fd].mode & MODE_MODIFY) != 0) {
		unsigned char *record = get_record(fat, fd);
		int truncate_data = 0;

		if ((this_fat->fd[fd].mode & MODE_WRITE) != 0)
			truncate_data = 1;

		/*
		 * Do not modify system or read-only files.
		 */
		if ((record[11] & 0x05) != 0)
			return FAT_READ_ONLY_FILE;

		/*
		 * Directories are not truncated.
		 */
		if ((record[11] & 0x10) != 0)
			truncate_data = 0;

		/*
		 * For new files, do not update record values again.
		 */
		if ((this_fat->fd[fd].mode & 0x80000000) != 0)
			truncate_data = 0;

		/*
		 * Delete the existing file data.
		 */
		if (truncate_data) {
			record[11] = (unsigned char)(record[11] | 0x20);

			cluster = (LE16(&record[20]) << 16);
			cluster |= LE16(&record[26]);

			W_LE16(&record[20], 0);
			W_LE16(&record[26], 0);
			W_LE32(&record[28], 0);

			write_timestamps(record, 0, 1);

			if (write_record_buffer(fat, fd))
				return FAT_BLOCK_WRITE_ERROR;

			if (cluster != 0)
				delete_clusters(fat, cluster);
		}
	}

	this_fat->fd[fd].opened = 1;

	return 0;
}

int fat_read(void *fat, int fd, size_t *size, void *buf)
{
	size_t requested_size = 0;
	unsigned char *cluster_buffer, *buf_ptr = buf;
	unsigned int cluster_size, cluster, offset;
	unsigned int file_clusters, file_offset, file_size;
	unsigned int read_bytes_limit, read_bytes = 0;
	int is_dir = 0, r = 0;

	if (size != NULL) {
		requested_size = *size;
		*size = 0;
	}

	if (this_fat->ready != FAT_READY) {
		if (buf_ptr != NULL && requested_size != 0)
			memset(buf_ptr, 0, requested_size);
		return FAT_NOT_READY;
	}

	if (check_fd(fat, fd, 1) != 0) {
		if (buf_ptr != NULL && requested_size != 0)
			memset(buf_ptr, 0, requested_size);
		return FAT_INVALID_PARAMETERS;
	}

	if (requested_size == 0)
		return 0;

	cluster_size = this_fat->cluster_size;
	file_offset = this_fat->fd[fd].offset;

	if ((this_fat->fd[fd].mode & MODE_ROOT_DIR) != 0) {
		if (this_fat->type_32 == 0) {
			*size = requested_size;
			return read_fixed_root_dir(fat, fd, size, buf);
		}
		cluster = this_fat->fs_root_cluster;

		/*
		 * Read the whole cluster chain for the FAT32 root
		 * directory but the maximum size is 65536 * 32 bytes.
		 */
		file_size = 2097152;
		is_dir = 1;
	} else {
		const unsigned char *record = get_record(fat, fd);

		cluster = (LE16(&record[20]) << 16) | LE16(&record[26]);
		file_size = LE32(&record[28]);

		/*
		 * Read the whole cluster chain for directories but
		 * directory maximum size is 65536 * 32 bytes.
		 */
		if ((record[11] & 0x10) != 0) {
			file_size = 2097152;
			is_dir = 1;
		}
	}

	if (this_fat->fd[fd].eof != 0 || file_size <= file_offset) {
		if (buf_ptr != NULL)
			memset(buf_ptr, 0, requested_size);
		this_fat->fd[fd].eof = 1;
		return 0;
	}

	/*
	 * Calculate how many clusters there are.
	 */
	file_clusters = file_size / cluster_size;
	if ((file_size % cluster_size) != 0)
		file_clusters += 1;

	/*
	 * Find the first cluster to be read. Use the previous value if
	 * possible. This avoids going through the whole cluster chain.
	 */
	{
		unsigned int i = 0, read_index;

		read_index = this_fat->fd[fd].offset / cluster_size;

		if (this_fat->fd[fd].cluster_idx != 0) {
			if (this_fat->fd[fd].cluster_idx <= read_index) {
				i = this_fat->fd[fd].cluster_idx;
				cluster = this_fat->fd[fd].cluster_val;
			}
		}

		r = FAT_INCONSISTENT_STATE;

		for (/* void */; i < file_clusters; i++) {
			if (i == read_index) {
				r = 0;
				break;
			}
			cluster = get_table_value(fat, cluster);
			if (is_dir != 0 && cluster >= 0x0FFFFFF8) {
				r = 0;
				break;
			}
			if (cluster < 2 || cluster >= 0x0FFFFFF8)
				break;
		}

		if (r != 0)
			return r;
	}

	offset = (file_offset % cluster_size);
	cluster_buffer = this_fat->cluster_buffer;

	read_bytes_limit = file_size - file_offset;

	if (read_bytes_limit > requested_size)
		read_bytes_limit = (unsigned int)requested_size;

	if (is_dir != 0 && cluster >= 0x0FFFFFF8)
		read_bytes_limit = 0;

	/*
	 * Read the data. The destination buffer can be NULL.
	 */
	while (read_bytes < read_bytes_limit) {
		r = read_cluster(fat, cluster, 0);
		if (r != 0)
			break;

		/*
		 * Save known cluster index and value.
		 */
		{
			unsigned int idx = this_fat->fd[fd].offset;

			idx = (idx + read_bytes) / cluster_size;

			this_fat->fd[fd].cluster_idx = idx;
			this_fat->fd[fd].cluster_val = cluster;
		}

		if (buf_ptr != NULL) {
			while (offset < cluster_size) {
				*buf_ptr++ = cluster_buffer[offset++];
				if (!(++read_bytes < read_bytes_limit))
					break;
			}
		} else {
			while ((offset++) < cluster_size) {
				if (!(++read_bytes < read_bytes_limit))
					break;
			}
		}

		offset = 0;
		cluster = get_table_value(fat, cluster);

		if (cluster >= 0x0FFFFFF8)
			break;
	}

	if (is_dir != 0) {
		unsigned int dir_offset;

		if (cluster >= 0x0FFFFFF8) {
			dir_offset = this_fat->fd[fd].offset + read_bytes;
			if ((dir_offset % cluster_size) == 0)
				this_fat->fd[fd].eof = 1;
		}

		if (r == 0 && read_bytes == 0)
			this_fat->fd[fd].eof = 1;
	}

	if (read_bytes < requested_size) {
		size_t buf_end_size = requested_size - (size_t)read_bytes;
		if (buf_ptr != NULL)
			memset(buf_ptr, 0, buf_end_size);
	}

	this_fat->fd[fd].offset += read_bytes;

	if (this_fat->fd[fd].offset >= file_size)
		this_fat->fd[fd].eof = 1;

	*size = read_bytes;

	return r;
}

int fat_remove(void *fat, const char *name)
{
	unsigned char *record;
	unsigned int cluster;
	int fd, r;

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	/*
	 * Open file/dir using the special fd entry.
	 */
	fd = (int)this_fat->fd_special;

	init_fat_fd(fat, fd);
	this_fat->fd[fd].mode = MODE_READ;

	if (fill_path_buffer(fat, name))
		return FAT_INVALID_FILE_NAME;

	if ((r = fill_record_buffer(fat, fd)) != 0)
		return r;

	record = get_record(fat, fd);
	cluster = (LE16(&record[20]) << 16) | LE16(&record[26]);

	if ((record[11] & 0x0C) != 0)
		return FAT_READ_ONLY_RECORD;

	/*
	 * Delete directories only if they are empty.
	 */
	if ((record[11] & 0x10) != 0 && cluster != 0) {
		unsigned int cluster_size = this_fat->cluster_size;
		unsigned int iterate_limit = (65536 * 32) / cluster_size;
		unsigned int dir_cluster = cluster;
		unsigned int i = 64;

		while (iterate_limit) {
			int found_zero = 0;

			if ((r = read_cluster(fat, dir_cluster, 0)) != 0)
				return r;

			for (/* void */; i < cluster_size; i += 32) {
				unsigned int c = this_fat->cluster_buffer[i];

				if (c == 0) {
					found_zero = 1;
					break;
				}
				if (c != 0xE5)
					return FAT_DIRECTORY_NOT_EMPTY;
			}

			if (found_zero)
				break;

			dir_cluster = get_table_value(fat, dir_cluster);
			if (dir_cluster < 2 || dir_cluster >= 0x0FFFFFF8)
				break;

			iterate_limit -= 1;
			i = 0;
		}
		if (iterate_limit == 0)
			return FAT_INCONSISTENT_STATE;
	}

	/*
	 * Delete the record.
	 */
	record[0] = 0xE5;
	memset(&record[1], 0x20, 10);
	memset(&record[11], 0, 21);

	if ((r = write_record_buffer(fat, fd)) != 0)
		return r;

	delete_clusters(fat, cluster);

	return 0;
}

int fat_rename(void *fat, const char *old_name, const char *new_name)
{
	unsigned int path_entries;
	unsigned char fat_new_name[12];
	unsigned char *record;
	unsigned int i, is_dir = 0;
	int fd, r;

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	/*
	 * Open the old file/dir using the special fd entry.
	 */
	fd = (int)this_fat->fd_special;
	this_fat->fd[fd].opened = 0;
	this_fat->fd[fd].offset = 0;
	this_fat->fd[fd].mode = MODE_READ;
	this_fat->fd[fd].eof = 0;

	if (fill_path_buffer(fat, old_name))
		return FAT_INVALID_FILE_NAME;

	if ((path_entries = this_fat->path_entries) == 0)
		return FAT_INVALID_PARAMETERS;

	if (this_fat->path_buffer[path_entries - 1].is_dir)
		is_dir = 1;

	if ((r = fill_record_buffer(fat, fd)) != 0)
		return r;

	/*
	 * The new name must not exist.
	 */
	if (fill_path_buffer(fat, new_name))
		return FAT_INVALID_FILE_NAME;

	if (path_entries != this_fat->path_entries)
		return FAT_INVALID_PARAMETERS;

	if (this_fat->path_buffer[path_entries - 1].is_dir) {
		if (is_dir == 0)
			return FAT_INVALID_PARAMETERS;
	}

	r = fill_record_buffer(fat, fd);

	if (r != FAT_FILE_NOT_FOUND) {
		if (r == 0)
			return FAT_INVALID_PARAMETERS;
		return r;
	}

	/*
	 * Save the new name.
	 */
	{
		const char *src;

		src = &this_fat->path_buffer[path_entries - 1].name[0];
		memcpy(&fat_new_name[0], src, 12);
	}

	/*
	 * Both files must be in the same directory.
	 */
	if (path_entries > 1) {
		struct fat_name *path_buffer = this_fat->path_buffer;
		char buf[12];

		for (i = 0; i < path_entries - 1; i++) {
			if (i > 0) {
				if (fill_path_buffer(fat, new_name))
					return FAT_INVALID_FILE_NAME;
			}

			memcpy(&buf[0], &path_buffer[i].name[0], 12);

			if (fill_path_buffer(fat, old_name))
				return FAT_INVALID_FILE_NAME;

			if (fat_strncmp(&path_buffer[i].name[0], &buf[0], 11))
				return FAT_INVALID_PARAMETERS;
		}
	}

	/*
	 * Change the existing name (first 11 bytes of the record).
	 */
	if (fill_path_buffer(fat, old_name))
		return FAT_INVALID_FILE_NAME;

	if ((r = fill_record_buffer(fat, fd)) != 0)
		return r;

	record = get_record(fat, fd);
	memcpy(&record[0], &fat_new_name[0], 11);

	if ((r = write_record_buffer(fat, fd)) != 0)
		return r;

	return 0;
}

int fat_seek(void *fat, int fd, int offset, int whence)
{
	unsigned int maximum_offset = this_fat->maximum_file_size - 1;
	unsigned int extra_sub = 0;

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 1) != 0)
		return FAT_INVALID_PARAMETERS;

	if (offset < 0) {
		int val = INT_MAX;

		val += offset;
		if (val < 0) {
			val = (-val);
			offset += val;
			extra_sub = (unsigned int)val;
		}
	}

	/*
	 * SEEK_SET
	 */
	if (whence == 0) {
		if (offset < 0)
			return FAT_SEEK_ERROR;
		if ((unsigned int)offset > maximum_offset)
			return FAT_SEEK_ERROR;
		this_fat->fd[fd].offset = (unsigned int)offset;
		this_fat->fd[fd].eof = 0;
		return 0;
	}

	/*
	 * SEEK_CUR and SEEK_END
	 */
	if (whence == 1 || whence == 2) {
		unsigned int off = this_fat->fd[fd].offset;
		unsigned int val;

		if (whence == 2) {
			const unsigned char *record = get_record(fat, fd);
			off = LE32(&record[28]);
		}

		if (offset < 0) {
			val = (unsigned int)(-offset);
			if (off < val)
				return FAT_SEEK_ERROR;
			off -= val;
			while (extra_sub--) {
				if (off == 0)
					return FAT_SEEK_ERROR;
				off -= 1;
			}
		} else {
			if (this_fat->fd[fd].eof == 2)
				return FAT_SEEK_ERROR;
			val = (unsigned int)offset;
			if (!(off <= UINT_MAX - val))
				return FAT_SEEK_ERROR;
			off += val;
			if (off > maximum_offset)
				return FAT_SEEK_ERROR;
		}
		this_fat->fd[fd].offset = off;
		this_fat->fd[fd].eof = 0;
		return 0;
	}

	return FAT_SEEK_ERROR;
}

int fat_tell(void *fat, int fd, unsigned int *offset)
{
	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 1) != 0)
		return FAT_INVALID_PARAMETERS;

	*offset = this_fat->fd[fd].offset;

	return 0;
}

int fat_write(void *fat, int fd, size_t *size, const void *buf)
{
	struct fat_fd *fd_entry;
	size_t requested_size = 0;
	unsigned int cluster_size, record_cluster, offset_cluster = 0;
	unsigned int file_clusters, seek_bytes = 0, written_bytes = 0;
	unsigned int file_size, original_file_size;
	unsigned char *record;
	int r = 0;

	if (size != NULL) {
		requested_size = *size;
		*size = 0;
	}

	if (this_fat->ready != FAT_READY)
		return FAT_NOT_READY;

	if (check_fd(fat, fd, 1) != 0)
		return FAT_INVALID_PARAMETERS;

	cluster_size = this_fat->cluster_size;
	fd_entry = &this_fat->fd[fd];

	record = get_record(fat, fd);

	/*
	 * Directories are read-only.
	 */
	if ((record[11] & 0x10) != 0)
		return FAT_READ_ONLY_RECORD;

	if ((fd_entry->mode & MODE_MODIFY) == 0)
		return FAT_READ_ONLY_FILE;

	record_cluster = (LE16(&record[20]) << 16) | LE16(&record[26]);
	file_size = original_file_size = LE32(&record[28]);

	if ((fd_entry->mode & MODE_APPEND) != 0)
		fd_entry->offset = file_size;

	if (fd_entry->offset > file_size)
		seek_bytes = fd_entry->offset - file_size;

	/*
	 * Check the offset and requested size (overflow).
	 * Set the new file size;
	 */
	{
		unsigned int maximum_file_size = this_fat->maximum_file_size;
		unsigned int offset = fd_entry->offset;
		unsigned int new_file_size = 0;

		if (!(requested_size <= maximum_file_size))
			new_file_size = maximum_file_size;
		if (!((unsigned int)requested_size <= (UINT_MAX - offset)))
			new_file_size = maximum_file_size;
		if (!((requested_size + offset) <= maximum_file_size))
			new_file_size = maximum_file_size;

		if (new_file_size == 0)
			new_file_size = (unsigned int)requested_size + offset;

		if (file_size < new_file_size)
			file_size = new_file_size;
	}

	/*
	 * Allocate the first cluster.
	 */
	if (requested_size != 0 && record_cluster == 0) {
		const unsigned int end = 0x0FFFFFFF;
		unsigned int cluster_hi, cluster_lo;

		if ((r = allocate_cluster(fat, &record_cluster)) != 0)
			return r;
		if ((r = set_table_value(fat, record_cluster, end)) != 0)
			return r;
		if ((r = write_table(fat)) != 0)
			return r;

		cluster_hi = record_cluster >> 16;
		cluster_lo = record_cluster & 0xFFFF;

		W_LE16(&record[20], cluster_hi);
		W_LE16(&record[26], cluster_lo);

		if ((r = write_record_buffer(fat, fd)) != 0) {
			(void)set_table_value(fat, record_cluster, 0);
			(void)write_table(fat);
			W_LE16(&record[20], 0);
			W_LE16(&record[26], 0);
			return r;
		}
	}

	/*
	 * Calculate how many clusters are needed.
	 */
	file_clusters = file_size / cluster_size;
	if ((file_size % cluster_size) != 0)
		file_clusters += 1;

	/*
	 * Find first write cluster and allocate additional clusters.
	 */
	if (requested_size != 0) {
		unsigned int iterate_limit = this_fat->fs_clusters + 2;
		unsigned int cluster = record_cluster;
		unsigned int i = 0, next, write_index;

		write_index = (fd_entry->offset - seek_bytes) / cluster_size;

		/*
		 * Use the previous value if possible. This avoids
		 * going through the whole cluster chain.
		 */
		if (fd_entry->cluster_idx != 0) {
			if (fd_entry->cluster_idx <= write_index) {
				i = fd_entry->cluster_idx;
				cluster = fd_entry->cluster_val;
			}
		}

		for (/* void */; i < iterate_limit; i++) {
			if (i == write_index) {
				offset_cluster = cluster;
				if (file_size == original_file_size)
					break;
			}

			next = get_table_value(fat, cluster);

			if (next >= 0x0FFFFFF8) {
				if (i + 1 >= file_clusters)
					break;

				r = append_cluster(fat, cluster, 1);
				if (r != 0)
					break;

				next = get_table_value(fat, cluster);
			}

			if (next < 2) {
				r = FAT_INCONSISTENT_STATE;
				break;
			}
			cluster = next;
		}

		if (write_table(fat) != 0)
			return FAT_BLOCK_WRITE_ERROR;

		if (r > 0 && r != FAT_NOT_ENOUGH_SPACE) {
			delete_extra_clusters(fat, fd);
			return r;
		}
	}

	/*
	 * Write the data from the buffer. The buffer can be NULL.
	 */
	if (offset_cluster >= 2) {
		const unsigned char *buf_ptr = buf;
		unsigned int original_size = LE32(&record[28]);
		unsigned char *cluster_buffer = this_fat->cluster_buffer;
		unsigned int end, offset, written = 0;

		offset = (fd_entry->offset - seek_bytes) % cluster_size;

		if (requested_size > this_fat->maximum_file_size)
			end = (unsigned int)this_fat->maximum_file_size;
		else
			end = (unsigned int)requested_size;

		while (written < end) {
			int skip_io = 0;

			if (fd_entry->offset + written >= original_size) {
				if (offset == 0)
					skip_io = 1;
			}

			r = read_cluster(fat, offset_cluster, skip_io);
			if (r != 0)
				break;

			/*
			 * Save known cluster index and value.
			 */
			{
				unsigned int idx = this_fat->fd[fd].offset;

				idx = (idx + written_bytes) / cluster_size;

				this_fat->fd[fd].cluster_idx = idx;
				this_fat->fd[fd].cluster_val = offset_cluster;
			}

			if (seek_bytes != 0) {
				while (offset < cluster_size) {
					cluster_buffer[offset++] = 0;
					if (--seek_bytes == 0)
						break;
				}
			}

			if (buf_ptr != NULL) {
				while (offset < cluster_size) {
					cluster_buffer[offset++] = *buf_ptr++;
					if (!(++written < requested_size))
						break;
				}
			} else {
				while (offset < cluster_size) {
					cluster_buffer[offset++] = 0;
					if (!(++written < requested_size))
						break;
				}
			}

			if (fd_entry->offset + written >= original_size) {
				while (offset < cluster_size)
					cluster_buffer[offset++] = 0;
			}

			offset = 0;

			if ((r = write_cluster(fat)) != 0)
				break;

			written_bytes = written;

			offset_cluster = get_table_value(fat, offset_cluster);
			if (offset_cluster >= 0x0FFFFFF8)
				break;
		}
	}

	/*
	 * Adjust the file size according to what was actually written.
	 */
	if (written_bytes != 0) {
		file_size = original_file_size;

		if (file_size < fd_entry->offset + written_bytes)
			file_size = fd_entry->offset + written_bytes;

		record[11] = (unsigned char)(record[11] | 0x20);
		W_LE32(&record[28], file_size);
	}

	write_timestamps(record, 0, 1);

	/*
	 * Write the record buffer. Modify the output size
	 * if the record file size cannot be updated.
	 */
	if ((r = write_record_buffer(fat, fd)) != 0) {
		unsigned int new_off = fd_entry->offset + written_bytes;

		if (fd_entry->offset >= original_file_size)
			written_bytes = 0;
		else if (original_file_size < new_off)
			written_bytes = original_file_size - fd_entry->offset;

		W_LE32(&record[28], original_file_size);
	}

	/*
	 * There may be more clusters allocated than actually
	 * used if the requested size was not written.
	 */
	if (written_bytes != requested_size) {
		delete_extra_clusters(fat, fd);
		if (r == 0)
			r = FAT_NOT_ENOUGH_SPACE;
	}

	fd_entry->offset += written_bytes;

	/*
	 * Set the "eof" if offset is the maximum size.
	 */
	if (written_bytes != 0) {
		if (this_fat->fd[fd].offset >= this_fat->maximum_file_size)
			this_fat->fd[fd].eof = 1;
	}

	if (size != NULL)
		*size = written_bytes;

	return r;
}
