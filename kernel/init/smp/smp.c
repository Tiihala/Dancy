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
 * init/smp/smp.c
 *      Symmetric multiprocessing
 */

#include <init.h>

volatile uint32_t smp_ap_count = 0;

static void *get_trampoline_addr(void)
{
	const uint32_t top = 0x98000;
	const struct b_mem_raw *map = memory_map;
	uint32_t addr;

	for (addr = 0; map->base_high == 0; map++) {
		uint32_t type = map[0].type;
		uint32_t b1 = map[0].base_low;
		uint32_t b2 = map[1].base_low;

		if (b1 > top)
			break;

		if (type == B_MEM_NORMAL) {
			uint32_t a = ((b2 < top) ? b2 : top) & 0xFFFFF000u;

			if (a > b1 && (a - b1) >= 0x1000)
				addr = a - 0x1000;
		}
	}

	if (addr < 0x1000)
		panic("SMP: low memory for trampoline not found");

	return (void *)((phys_addr_t)addr);
}

static int ap_lock;

void smp_ap_entry(void)
{
	uint32_t id;

	/*
	 * Increment the AP counter.
	 */
	spin_lock(&ap_lock);
	smp_ap_count += 1;
	spin_unlock(&ap_lock);

	/*
	 * Read the APIC ID.
	 */
	id = apic_id();

	/*
	 * Continue the journey.
	 */
	init_ap(id);
}

void smp_init(void)
{
	const struct acpi_information *acpi = acpi_get_information();
	uint32_t ap_entry_addr = (uint32_t)((addr_t)(smp_ap_entry));
	unsigned char *addr, *ap_array;
	size_t ap_array_size;
	unsigned cpu_count, i, j;

	if (acpi == NULL || acpi->num_cpu_core < 2)
		return;

	cpu_count = (acpi->num_cpu_core < 256) ? acpi->num_cpu_core : 256u;

	addr = get_trampoline_addr();
	memcpy(addr, &smp_trampoline[0], 512);

	ap_array_size = (size_t)((cpu_count - 1) * 4096);
	ap_array = aligned_alloc(4096, ap_array_size);

	if (!ap_array)
		panic("SMP: out of memory");

	memset(ap_array, 0, ap_array_size);

	for (i = 0; i < cpu_count; i++) {
		unsigned ap_offset = i * 4096;
		uint32_t ap_status = 0;
		uint32_t icr_low, icr_high;
		struct acpi_apic apic;

		if (acpi_get_apic(i, &apic))
			panic("SMP: cannot access APIC");

		/*
		 * Skip the bootstrap processor.
		 */
		if (apic.id == apic_id())
			continue;

		if (apic.enabled == 0)
			continue;

		/*
		 * The trampoline contains an smp_ap_info structure.
		 *
		 * Offset 0x1F0
		 *
		 *     struct smp_ap_info {
		 *            uint32_t ap_stack;
		 *            uint32_t ap_paging;
		 *            uint32_t ap_status;
		 *     };
		 *
		 */
		{
			uint32_t ap_stack;
			phys_addr_t ap_paging;

			/*
			 * Use correct stack alignment (DANCY_32 or DANCY_64).
			 */
			ap_offset -= (unsigned)(32 + sizeof(addr_t));
			ap_stack = (uint32_t)((addr_t)(&ap_array[ap_offset]));

			/*
			 * The stack is unique for each AP and it contains the
			 * smp_ap_entry address. The trampoline code will take
			 * the address and replace it with NULL. It then jumps
			 * to the function and the stack frame is compatible
			 * with ABIs that are in use.
			 *
			 * If the smp_ap_entry function returns, it tries to
			 * reach NULL and a page fault will happen.
			 */
			cpu_write32(&ap_array[ap_offset], ap_entry_addr);

			/*
			 * Use the same paging structures.
			 */
			pg_get_current(&ap_paging);

			/*
			 * Write the smp_ap_info values.
			 */
			cpu_write32(&addr[0x1F0], ap_stack);
			cpu_write32(&addr[0x1F4], (uint32_t)ap_paging);
			cpu_write32(&addr[0x1F8], ap_status);
		}

		icr_high = apic.id << 24;

		/*
		 * Send INIT (Assert).
		 */
		icr_low = 0x00004500;
		apic_send(icr_low, icr_high);

		/*
		 * Wait for 10 milliseconds.
		 */
		delay(10000000);

		/*
		 * Send INIT (Deassert).
		 */
		icr_low = 0x00000500;
		apic_send(icr_low, icr_high);

		/*
		 * Wait for a very short moment.
		 */
		delay(100000);

		/*
		 * Send Startup IPI. The apic_send() function waits until the
		 * delivery status bit is 0 before sending the interrupt.
		 */
		icr_low = (uint32_t)(((addr_t)addr >> 12) & 0xFFu) | 0x4600u;
		apic_send(icr_low, icr_high);

		/*
		 * Wait for the AP to wake up (about 2 milliseconds).
		 */
		for (j = 0; j < 20; j++) {
			ap_status = cpu_read32(&addr[0x1F8]);

			if (ap_status == 1)
				break;

			delay(100000);
		}

		/*
		 * Send another Startup IPI.
		 */
		if ((ap_status = cpu_read32(&addr[0x1F8])) != 1) {
			apic_send(icr_low, icr_high);

			/*
			 * Wait for the AP to wake up (about half a second).
			 */
			for (j = 0; j < 5000; j++) {
				ap_status = cpu_read32(&addr[0x1F8]);

				if (ap_status == 1)
					break;

				delay(100000);
			}
		}

		/*
		 * If the AP did not wake up, clear the smp_ap_entry address.
		 */
		if (ap_status != 1) {
			cpu_write32(&ap_array[ap_offset], 0);
			continue;
		}

		/*
		 * Tell the AP that it may continue.
		 */
		ap_status = 2;
		cpu_write32(&addr[0x1F8], ap_status);

		/*
		 * The AP must acknowledge the message. It is unlikely that
		 * this final step fails because the AP was already running.
		 */
		for (j = 0; /* void */; j++) {
			ap_status = cpu_read32(&addr[0x1F8]);

			if (ap_status == 3)
				break;

			if (j == 5000)
				panic("SMP: acknowledge failure");

			delay(100000);
		}
	}
}