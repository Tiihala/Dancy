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
 * init/cpu.c
 *      Test CPU features
 */

#include <dancy.h>

int cpu_test_features(void)
{
	uint32_t eax, ecx, edx, ebx;
	int ret = 0;

	/*
	 * The boot loader guarantees that cpuid is available and
	 * a floating point unit is "on-chip".
	 */
	cpu_id((eax = 0, &eax), &ecx, &edx, &ebx);
	log("CPU Features\n");
	log("  %.4s%.4s%.4s\n", (char *)&ebx, (char *)&edx, (char *)&ecx);

	if (eax >= 1)
		cpu_id((eax = 1, &eax), &ecx, &edx, &ebx);
	else
		eax = 0, ecx = 0, edx = 0, ebx = 0, ret = 1;

	if (edx & (1u << 4)) {
		log("  Time Stamp Counter (TSC)\n");
	} else {
		b_print("Error: Time Stamp Counter (TSC) is required.\n");
		ret = 1;
	}

	if (edx & (1u << 9))
		log("  APIC On-Chip\n");
	if (edx & (1u << 13))
		log("  Page Global Bit\n");

	log("\n");
	return ret;
}
