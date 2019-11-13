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
 * init/ld.c
 *      Dynamic linker and loader
 */

#include <init.h>

#if defined (DANCY_32)
static const unsigned obj_magic = 0x014C;
#elif defined (DANCY_64)
static const unsigned obj_magic = 0x8664;
#endif

static int ld_error(const char *name, const char *msg)
{
	b_print("Error: %s (%s)\n", name, msg);
	return 1;
}

static int ld_reloc_access(unsigned reloc_type)
{
	/*
	 * The return value tells how many bytes must be accessible from
	 * the reloc offset in the relocatable section. Accessible does
	 * not necessarily mean that the relocation modifies all bytes.
	 */
	int bytes_to_access = 0;

#if defined (DANCY_32)
	if (reloc_type < 0x8000) {
		switch (reloc_type) {
		case 0x06: bytes_to_access = 0x04; break;
		case 0x07: bytes_to_access = 0x04; break;
		case 0x14: bytes_to_access = 0x04; break;
		default: break;
		}
	}
#elif defined (DANCY_64)
	if (reloc_type < 0x8000) {
		switch (reloc_type) {
		case 0x01: bytes_to_access = 0x08; break;
		case 0x02: bytes_to_access = 0x04; break;
		case 0x03: bytes_to_access = 0x04; break;
		case 0x04: bytes_to_access = 0x04; break;
		case 0x05: bytes_to_access = 0x05; break;
		case 0x06: bytes_to_access = 0x06; break;
		case 0x07: bytes_to_access = 0x07; break;
		case 0x08: bytes_to_access = 0x08; break;
		case 0x09: bytes_to_access = 0x09; break;
		default: break;
		}
	}
#endif
	return bytes_to_access;
}

int ld_validate(const char *name, unsigned char *obj, size_t size)
{
	if (size < 212)
		return ld_error(name, "unsupported format");
	if (LE32(&obj[16]) != size)
		return ld_error(name, "size error");

	{
		unsigned long crc_value = LE32(&obj[24]);
		int err;

		W_LE32(&obj[24], 0);
		err = (crc32c(obj, size) != crc_value);
		W_LE32(&obj[24], crc_value);

		if (err)
			return ld_error(name, "CRC-32C");
	}

	obj += 32;

	if (LE16(&obj[0]) != obj_magic)
		return ld_error(name, "magic number");
	if (LE16(&obj[2]) != 4)
		return ld_error(name, "number of sections");
	if (obj[16] || obj[17])
		return ld_error(name, "optional header");

	if (memcmp(&obj[20], ".text", 6))
		return ld_error(name, ".text section");
	if (memcmp(&obj[60], ".rdata", 7))
		return ld_error(name, ".rdata section");
	if (memcmp(&obj[100], ".data", 6))
		return ld_error(name, ".data section");
	if (memcmp(&obj[140], ".bss", 5))
		return ld_error(name, ".bss section");

	if ((LE32(&obj[20+36]) & 0xFF0FFFFF) != 0x60000020)
		return ld_error(name, ".text flags");
	if ((LE32(&obj[60+36]) & 0xFF0FFFFF) != 0x40000040)
		return ld_error(name, ".rdata flags");
	if ((LE32(&obj[100+36]) & 0xFF0FFFFF) != 0xC0000040)
		return ld_error(name, ".data flags");
	if ((LE32(&obj[140+36]) & 0xFF0FFFFF) != 0xC0000080)
		return ld_error(name, ".bss flags");

	return 0;
}
