/*
 * Copyright (c) 2019, 2020, 2021 Antti Tiihala
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
 * init/cpu.c
 *      Test CPU features and initialize control registers
 */

#include <boot/init.h>

static int cpu_osfxr_support;

int cpu_test_features(void)
{
	uint32_t eax, ecx, edx, ebx;
	int ret = 0;

	/*
	 * The boot loader guarantees that cpuid is available and
	 * a floating point unit is "on-chip".
	 */
	cpu_id((eax = 0, &eax), &ecx, &edx, &ebx);
	b_log("CPU Features\n");
	b_log("\t%.4s%.4s%.4s\n", (char *)&ebx, (char *)&edx, (char *)&ecx);

	if (eax >= 1)
		cpu_id((eax = 1, &eax), &ecx, &edx, &ebx);
	else
		eax = 0, ecx = 0, edx = 0, ebx = 0, ret = 1;

	if ((edx & (1u << 4)) != 0) {
		b_log("\tTime Stamp Counter (TSC)\n");
	} else {
		b_print("Error: Time Stamp Counter (TSC) is required.\n");
		ret = 1;
	}

	if ((edx & (1u << 9)) != 0)
		b_log("\tAPIC On-Chip\n");

	if ((edx & (1u << 13)) != 0)
		b_log("\tPage Global Bit\n");

	if ((edx & (1u << 24)) != 0) {
		b_log("\tFXSAVE and FXRSTOR Instructions\n");
		cpu_osfxr_support = 1;
	}

	b_log("\n");
	return ret;
}

void cpu_init_control_registers(void)
{
	cpu_native_t cr0 = cpu_read_cr0();
	cpu_native_t cr4 = cpu_read_cr4();

	/*
	 * Initialize CR0 Register.
	 */
	cr0 = (cpu_native_t)((cr0 & 0xFFFFFFFF9FFAFFF3ull) | 0x22u);
	cpu_write_cr0(cr0);

	/*
	 * Initialize CR4 Register.
	 */
	cr4 = (cpu_native_t)(cr4 & 0xFFFFFFFFFFFBF830ull);
	if (cpu_osfxr_support)
		cr4 |= (1u << 9);
	cpu_write_cr4(cr4);

#ifdef DANCY_64
	/*
	 * Initialize EFER Register.
	 */
	{
		uint32_t eax, edx;

		cpu_rdmsr(0xC0000080u, &eax, &edx);
		eax = (eax & 0xFFFF8701u) | 0x01u;
		cpu_wrmsr(0xC0000080u, eax, edx);
	}
#endif
}
