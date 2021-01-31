/*
 * Copyright (c) 2020, 2021 Antti Tiihala
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
 * dancy/lib.h
 *      Header of Dancy Operating System
 */

#ifndef DANCY_LIB_H
#define DANCY_LIB_H

#include <dancy/types.h>

/*
 * Declarations of bitarray.c
 */
struct bitarray {
	unsigned char *data;
	size_t size;
	unsigned state[2];
	size_t written;
	int (*callback)(struct bitarray *b);
};

void bitarray_init(struct bitarray *b, unsigned char *data, size_t size);
void bitarray_callback(struct bitarray *b, int (*func)(struct bitarray *b));
void bitarray_clear(struct bitarray *b);
long bitarray_aligned_fetch(struct bitarray *b, unsigned bits, void **data);
long bitarray_fetch(struct bitarray *b, unsigned bits);
int bitarray_shove(struct bitarray *b, unsigned bits, unsigned val);
int bitarray_written(struct bitarray *b, size_t *written);

/*
 * Declarations of cpu.asm
 */
void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b);
void cpu_halt(uint32_t counter);
int cpu_ints(int enable);

void cpu_rdtsc(uint32_t *a, uint32_t *d);
void cpu_rdtsc_delay(uint32_t a, uint32_t d);
void cpu_rdtsc_diff(uint32_t *a, uint32_t *d);

void cpu_rdmsr(uint32_t msr, uint32_t *a, uint32_t *d);
void cpu_wrmsr(uint32_t msr, uint32_t a, uint32_t d);

uint8_t cpu_in8(uint16_t port);
uint16_t cpu_in16(uint16_t port);
uint32_t cpu_in32(uint16_t port);

void cpu_out8(uint16_t port, uint8_t value);
void cpu_out16(uint16_t port, uint16_t value);
void cpu_out32(uint16_t port, uint32_t value);

uint8_t cpu_read8(const void *address);
uint16_t cpu_read16(const void *address);
uint32_t cpu_read32(const void *address);

cpu_native_t cpu_read_cr0(void);
cpu_native_t cpu_read_cr2(void);
cpu_native_t cpu_read_cr3(void);
cpu_native_t cpu_read_cr4(void);

void cpu_write8(void *address, uint8_t value);
void cpu_write16(void *address, uint16_t value);
void cpu_write32(void *address, uint32_t value);

void cpu_write_cr0(cpu_native_t value);
void cpu_write_cr2(cpu_native_t value);
void cpu_write_cr3(cpu_native_t value);
void cpu_write_cr4(cpu_native_t value);

/*
 * Declarations of crc32.c and crc32c.c
 */
unsigned long crc32(const void *obj, size_t len);
unsigned long crc32c(const void *obj, size_t len);

/*
 * Declarations of fat.c
 */
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

/*
 * Declarations of huffman.c
 */
struct huffman {
	unsigned lengths[16];
	unsigned *symbols;
	unsigned completed;
};

int huffman_init(struct huffman *h, unsigned *symbols, int n);
int huffman_fetch(struct huffman *h, struct bitarray *b);
int huffman_table(struct huffman *h, unsigned *table, int n);

/*
 * Declarations of inflate.c
 */
int inflate(struct bitarray *b, unsigned char *out, size_t *size);

/*
 * Declarations of spin.asm
 */
void spin_enter(void **lock_local);
void spin_leave(void **lock_local);

void spin_lock(int *lock);
int spin_trylock(int *lock);
void spin_unlock(int *lock);

/*
 * Declarations of ttf.c
 */
int ttf_create(void **instance);
int ttf_delete(void *ttf);

int ttf_get_kerning(void *ttf, const unsigned int code_points[2], int *value);
int ttf_open(void *ttf, size_t size, const void *ttf_file);
int ttf_set_bitmap(void *ttf, size_t size, void *bitmap);
int ttf_set_shades(void *ttf, unsigned int number);
int ttf_render(void *ttf, unsigned int code_point, unsigned int *width);

#endif
