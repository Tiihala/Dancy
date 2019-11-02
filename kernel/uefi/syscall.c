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
 * uefi/syscall.c
 *      Boot loader system calls (UEFI)
 */

#include <uefi.h>

static unsigned long not_implemented(const char *name)
{
	u_print("%s\n", name);
	while (gSystemTable->BootServices->Stall(1000000) == EFI_SUCCESS) { }
	return 0;
}

unsigned long b_output_string(const char *arg1, unsigned int arg2)
{
	size_t size = (arg2 != 0) ? arg2 : (unsigned long)strlen(arg1);

	u_print("%.*s", size, arg1);
	return (unsigned long)size;
}

unsigned long b_output_string_hl(const char *arg1, unsigned int arg2)
{
	return not_implemented("b_output_string_hl");
}

unsigned long b_output_control(unsigned int arg1, unsigned int arg2)
{
	return not_implemented("b_output_control");
}

unsigned long b_get_keycode(void)
{
	return not_implemented("b_get_keycode");
}

unsigned long b_get_byte_com1(void)
{
	return not_implemented("b_get_byte_com1");
}

unsigned long b_put_byte_com1(unsigned char arg1)
{
	return not_implemented("b_put_byte_com1");
}

unsigned long b_get_byte_com2(void)
{
	return not_implemented("b_get_byte_com2");
}

unsigned long b_put_byte_com2(unsigned char arg1)
{
	return not_implemented("b_put_byte_com2");
}

unsigned long b_get_parameter(unsigned int arg1)
{
	return not_implemented("b_get_parameter");
}

unsigned long b_get_structure(void *arg1, unsigned int arg2)
{
	return not_implemented("b_get_structure");
}

unsigned long b_set_read_buffer(void *addr, unsigned int size)
{
	return block_set_read_buffer(addr, size);
}

unsigned long b_read_blocks(unsigned int lba, unsigned int blocks)
{
	return block_read_blocks(lba, blocks);
}

unsigned long b_set_write_buffer(void *addr, unsigned int size)
{
	return block_set_write_buffer(addr, size);
}

unsigned long b_write_blocks(unsigned int lba, unsigned int blocks)
{
	return block_write_blocks(lba, blocks);
}

unsigned long b_pause(void)
{
	return not_implemented("b_pause");
}

unsigned long b_exit(void)
{
	return not_implemented("b_exit");
}

unsigned long b_get_time(void *arg1)
{
	return not_implemented("b_get_time");
}
