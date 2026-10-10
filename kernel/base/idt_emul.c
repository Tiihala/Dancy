/*
 * Copyright (c) 2026 Antti Tiihala
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
 * base/idt_emul.c
 *      Emulate and patch user-space instructions
 */

#include <dancy.h>

static int get_byte(const uint8_t *c, int *i)
{
	if (pg_check_user_read(&c[*i], 1))
		return -1;

	return (int)(c[(*i)++]);
}

#if DANCY_32
static int emul_patch(uint8_t *c)
{
	addr_t tls_e = (addr_t)task_current()->tls._e;
	int i, state = 0;
	int b = get_byte(c, &state);

	/*
	 * Patch the 'mov eax, [gs:0x00000000]' instruction
	 * and the versions for other general-purpose registers.
	 */
	if (b == 0x65) {
		b = get_byte(c, &state);

		if (b == 0xA1) {
			for (i = 0; i < 4; i++) {
				if (get_byte(c, &state) != 0x00)
					return -1;
			}

			if (tls_e <= 0x10000000 || tls_e >= 0x20000000)
				return -1;

			c[0] = 0x90;
			memcpy(&c[2], &tls_e, 4);

			return 0;
		}

		if (b == 0x8B) {
			int mod_rm = -1;

			b = get_byte(c, &state);

			if (b == 0x05 || b == 0x0D || b == 0x15 || b == 0x1D)
				mod_rm = b;
			if (b == 0x25 || b == 0x2D || b == 0x35 || b == 0x3D)
				mod_rm = b;

			if (mod_rm < 0)
				return -1;

			for (i = 0; i < 4; i++) {
				if (get_byte(c, &state) != 0x00)
					return -1;
			}

			if (tls_e <= 0x10000000 || tls_e >= 0x20000000)
				return -1;

			c[0] = 0x90;
			memcpy(&c[3], &tls_e, 4);

			return 0;
		}

		return -1;
	}

	return -1;
}
#endif

#if DANCY_64
static int emul_patch(uint8_t *c)
{
	addr_t tls_e = (addr_t)task_current()->tls._e;
	int i, state = 0;
	int b = get_byte(c, &state);

	/*
	 * Patch the 'mov rax, [fs:0x00000000]' instruction
	 * and the versions for other general-purpose registers.
	 */
	if (b == 0x64) {
		int mod_rm = -1;

		if (get_byte(c, &state) != 0x48)
			return -1;
		if (get_byte(c, &state) != 0x8B)
			return -1;

		b = get_byte(c, &state);

		if (b == 0x04 || b == 0x0C || b == 0x14 || b == 0x1C)
			mod_rm = b;
		if (b == 0x24 || b == 0x2C || b == 0x34 || b == 0x3C)
			mod_rm = b;

		if (mod_rm < 0)
			return -1;

		b = get_byte(c, &state);

		if (b == 0x25) {
			for (i = 0; i < 4; i++) {
				if (get_byte(c, &state) != 0x00)
					return -1;
			}

			if (tls_e <= 0x10000000 || tls_e >= 0x20000000)
				return -1;

			c[0] = 0x90;
			memcpy(&c[5], &tls_e, 4);

			return 0;
		}

		return -1;
	}

	return -1;
}
#endif

int idt_emul_patch(void *stack)
{
	cpu_native_t *ip = stack;

	if (!emul_patch((void *)(*ip)))
		return 0;

	return DE_UNSUPPORTED;
}
