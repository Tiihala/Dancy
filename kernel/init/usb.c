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
 * init/usb.c
 *      Universal Serial Bus
 */

#include <init.h>

static phys_addr_t usb_get_base(struct pci_device *pci, int off, int early)
{
	phys_addr_t base = (phys_addr_t)pci_read(pci, off), base_high = 0;
	int io_space = (int)(base & 1u);
	int type = (int)((base >> 1) & 3u);

	if (io_space)
		return (phys_addr_t)(1);

	base &= 0xFFFFFFF0u;

	if (type == 2) {
		base_high = (phys_addr_t)pci_read(pci, off + 4);
		base |= ((base_high << 16) << 16);
#ifdef DANCY_32
		if (base_high)
			base = 0;
#endif
		if (base_high && early)
			base = 0;
	}

	return base;
}

static int usb_init_uhci(struct pci_device *pci, int early)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFFu;
	uint32_t base = pci_read(pci, 0x20);
	uint32_t i, val;

	int io_enabled = 0;
	int io_space = (int)(base & 1u);

	base &= 0xFFFFFFFCu;

	if ((cmd & 1u) != 0) {
		if (io_space == 0 || base > 0xFFFC)
			return (int)(__LINE__);
		io_enabled = 1;

	} else if ((cmd & 2u) != 0 && io_space == 0) {
		return (int)(__LINE__);
	}

	if (early) {
		b_log("Universal Host Controller Interface (UHCI)\n");

		b_log("\tI/O Base Address is %04X\n", base);

		val = pci_read(pci, 0x3C);
		b_log("\tInterrupt Line is %u and Interrupt PIN is %02X\n",
			(val & 0xFFu), ((val >> 8) & 0xFFu));

		val = pci_read(pci, 0x60) & 0xFFu;
		b_log("\tSerial Bus Release Number is %02X\n", val);

		val = pci_read(pci, 0xC0) & 0xFFFFu;
		b_log("\tLegacy support register value is %04X\n\n", val);

		return 0;
	}

	/*
	 * Disable the Legacy Support (LEGSUP).
	 */
	val = 0x8F00;
	pci_write(pci, 0xC0, val);

	if (!io_enabled)
		return 0;

	/*
	 * Clear the Run/Stop (RS) bit.
	 */
	{
		const uint32_t rs_bit = (1u);
		const uint32_t hc_halted_bit = (1u << 5);

		val = cpu_in16((uint16_t)(base + 0));

		if ((val & rs_bit) != 0) {
			val &= (~rs_bit);
			cpu_out16((uint16_t)(base + 0), (uint16_t)val);

			/*
			 * Wait until the HCHalted bit is set (USBSTS).
			 */
			for (i = 0; i < 10000; i++) {
				val = cpu_in16((uint16_t)(base + 2));

				if ((val & hc_halted_bit) != 0)
					break;

				delay(100000);
			}
		}
	}

	/*
	 * Reset the controller (HCRESET).
	 */
	{
		const uint32_t hc_reset_bit = (1u << 1);

		/*
		 * Set the HCRESET bit.
		 */
		val = 0u | hc_reset_bit;
		cpu_out16((uint16_t)(base + 0), (uint16_t)val);

		delay(100000);

		/*
		 * Wait until the HCRESET bit is cleared.
		 */
		for (i = 0; i < 10000; i++) {
			val = cpu_in16((uint16_t)(base + 0));

			if ((val & hc_reset_bit) == 0)
				break;

			delay(100000);
		}
	}

	/*
	 * Initialize USB Interrupt Enable Register (USBINTR).
	 */
	val = 0x0000;
	cpu_out16((uint16_t)(base + 4), (uint16_t)val);

	/*
	 * Initialize USB Status Register (USBSTS).
	 */
	val = 0x001F;
	cpu_out16((uint16_t)(base + 2), (uint16_t)val);

	/*
	 * Initialize USB Command Register (USBCMD).
	 */
	val = 0x0000;
	cpu_out16((uint16_t)(base + 0), (uint16_t)val);

	return 0;
}

static int usb_init_ohci(struct pci_device *pci, int early)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFFu;
	phys_addr_t base = usb_get_base(pci, 0x10, early);
	uint32_t hc_revision = 0;
	uint32_t i, val;

	int mem_enabled = 0, legacy_bit = 0;

	if ((cmd & 2u) != 0) {
		if (base < 0x20000)
			return (int)(__LINE__);
		mem_enabled = 1;

	} else if ((cmd & 1u) != 0 && base == 1) {
		return (int)(__LINE__);
	}

	if (mem_enabled) {
		hc_revision = cpu_read32((const uint32_t *)(base + 0));
		legacy_bit = (int)((hc_revision & 0x100u) >> 8);
	}

	if (early) {
		b_log("Open Host Controller Interface (OHCI)\n");

		b_log("\tMemory Base Address is %p\n", (const void *)base);

		val = pci_read(pci, 0x3C);
		b_log("\tInterrupt Line is %u and Interrupt PIN is %02X\n",
			(val & 0xFFu), ((val >> 8) & 0xFFu));

		b_log("\tHcRevision value is %08X\n", hc_revision);

		if (mem_enabled) {
			val = cpu_read32((const uint32_t *)(base + 0x04));
			b_log("\tHcControl value is %08X\n", val);
		}

		if (legacy_bit) {
			val = cpu_read32((const uint32_t *)(base + 0x100));
			b_log("\tHceControl value is %08X\n", val);
			val = cpu_read32((const uint32_t *)(base + 0x10C));
			b_log("\tHceStatus value is %08X\n", val);
		}

		b_log("\n");

		return 0;
	}

	if (!mem_enabled)
		return 0;

	pg_map_uncached((void *)base);

	/*
	 * Handle the InterruptRouting bit.
	 */
	for (i = 0; i < 20000; i++) {
		const uint32_t ir_bit = (1u << 8);
		const uint32_t oc_bit = (1u << 30);
		const uint32_t ocr_bit = (1u << 3);

		val = cpu_read32((const uint32_t *)(base + 0x04));
		if (i == 0 && (val & ir_bit) != 0) {
			/*
			 * Enable interrupt generation due to Ownership
			 * change (HcInterruptEnable).
			 */
			val = 0u | oc_bit;
			cpu_write32((uint32_t *)(base + 0x0C), val);

			/*
			 * Set OwnershipChangeRequest bit (HcCommandStatus).
			 */
			val = 0u | ocr_bit;
			cpu_write32((uint32_t *)(base + 0x08), val);

			delay(100000);
			val = cpu_read32((const uint32_t *)(base + 0x04));
		}

		/*
		 * Wait until the InterruptRouting bit is cleared.
		 */
		if ((val & ir_bit) == 0)
			break;

		delay(100000);
	}

	/*
	 * Disable all interrupts (HcInterruptDisable).
	 */
	val = 0xFFFFFFFF;
	cpu_write32((uint32_t *)(base + 0x14), val);

	/*
	 * Reset the controller (HcControl). The RemoteWakeupConnected bit
	 * is set by system firmware, and the value will be preserved.
	 */
	{
		const uint32_t rwc_bit = (1u << 9);

		val = 0u | rwc_bit;
		cpu_write32((uint32_t *)(base + 0x04), val);
	}

	/*
	 * Reset the controller (HcCommandStatus).
	 */
	{
		const uint32_t hcr_bit = (1u);

		val = 0u | hcr_bit;
		cpu_write32((uint32_t *)(base + 0x08), val);

		delay(100000);

		for (i = 0; i < 100; i++) {
			val = cpu_read32((const uint32_t *)(base + 0x08));

			if ((val & hcr_bit) == 0)
				break;

			delay(100000);
		}
	}

	/*
	 * Initialize HceControl Register.
	 */
	if (legacy_bit) {
		val = 0x00000100;
		cpu_write32((uint32_t *)(base + 0x100), val);
	}

	return 0;
}

static int usb_init_ehci(struct pci_device *pci, int early)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFFu;
	phys_addr_t base = usb_get_base(pci, 0x10, early), base_cap = 0;

	uint32_t cap_length = 0, hci_version = 0, eecp_offset = 0;
	uint32_t hcs_params = 0, hcc_params = 0;
	uint32_t i, val;

	int mem_enabled = 0;

	if ((cmd & 2u) != 0) {
		if (base < 0x20000)
			return (int)(__LINE__);
		mem_enabled = 1;

	} else if ((cmd & 1u) != 0 && base == 1) {
		return (int)(__LINE__);
	}

	if (mem_enabled) {
		val = cpu_read32((const uint32_t *)(base + 0x00));

		cap_length = (val & 0xFFu);
		base_cap = base + (phys_addr_t)cap_length;

		hci_version = ((val >> 16) & 0xFFFFu);
		hcs_params = cpu_read32((const uint32_t *)(base + 0x04));
		hcc_params = cpu_read32((const uint32_t *)(base + 0x08));

		eecp_offset = ((hcc_params >> 8) & 0xFFu);
	}

	/*
	 * Make sure that "EECP" points to the USBLEGSUP Register.
	 */
	for (i = 0; eecp_offset != 0; i++) {
		val = pci_read(pci, (int)(eecp_offset + 0x00));

		/*
		 * Check the Capability ID (value of 1).
		 */
		if ((val & 0xFFu) == 1)
			break;

		if (i == 128)
			return (int)(__LINE__);

		eecp_offset = ((val >> 8) & 0xFFu);
	}

	if (early) {
		b_log("Enhanced Host Controller Interface (EHCI)\n");

		b_log("\tMemory Base Address is %p\n", (const void *)base);

		val = pci_read(pci, 0x3C);
		b_log("\tInterrupt Line is %u and Interrupt PIN is %02X\n",
			(val & 0xFFu), ((val >> 8) & 0xFFu));

		val = pci_read(pci, 0x60) & 0xFFu;
		b_log("\tSerial Bus Release Number is %02X\n", val);

		b_log("\tCapability Register Length is %02X\n", cap_length);
		b_log("\tInterface Version Number is %04X\n", hci_version);

		b_log("\tStructural Parameters are %08X\n", hcs_params);
		b_log("\tCapability Parameters are %08X\n", hcc_params);

		if (mem_enabled) {
			val = cpu_read32((const uint32_t *)(base_cap + 0x00));
			b_log("\tCommand Register value is %08X\n", val);
			val = cpu_read32((const uint32_t *)(base_cap + 0x04));
			b_log("\tStatus Register value is %08X\n", val);
		}

		if (eecp_offset) {
			val = pci_read(pci, (int)(eecp_offset + 0x00));
			b_log("\tLegacy (EECP + 0) value is %08X\n", val);
			val = pci_read(pci, (int)(eecp_offset + 0x04));
			b_log("\tLegacy (EECP + 4) value is %08X\n", val);
		}

		b_log("\n");

		return 0;
	}

	if (!mem_enabled)
		return 0;

	pg_map_uncached((void *)base);

	/*
	 * Handle the HC BIOS Owned Semaphore (USBLEGSUP).
	 */
	if (eecp_offset) {
		const uint32_t smi_bit = (1u << 13);
		const uint32_t hc_bios_semaphore = (1u << 16);
		const uint32_t hc_os_semaphore = (1u << 24);

		int legsup_offset = (int)(eecp_offset + 0x00);
		int ctlsts_offset = (int)(eecp_offset + 0x04);

		/*
		 * Request ownership of the EHCI controller.
		 */
		val = pci_read(pci, legsup_offset);
		pci_write(pci, legsup_offset, (val | hc_os_semaphore));

		for (i = 0; (val & hc_bios_semaphore) != 0; i++) {
			/*
			 * If it seems that the BIOS does not release the
			 * controller, set the SMI on OS Ownership Enable
			 * bit (USBLEGCTLSTS) and try again.
			 */
			if (i == 10000) {
				/*
				 * Clear the SMI bit temporarily.
				 */
				val = pci_read(pci, ctlsts_offset);
				val &= (~smi_bit);
				pci_write(pci, ctlsts_offset, val);

				/*
				 * Cancel the current ownership request.
				 */
				val = pci_read(pci, legsup_offset);
				val &= (~hc_os_semaphore);
				pci_write(pci, legsup_offset, val);

				/*
				 * Set the SMI bit.
				 */
				val = pci_read(pci, ctlsts_offset);
				val |= smi_bit;
				pci_write(pci, ctlsts_offset, val);

				/*
				 * Set the OS Owned Semaphore again.
				 */
				val = pci_read(pci, legsup_offset);
				val |= hc_os_semaphore;
				pci_write(pci, legsup_offset, val);
			}

			/*
			 * Something went wrong. Take ownership from the BIOS.
			 */
			if (i == 30000) {
				val &= (~hc_bios_semaphore);
				pci_write(pci, legsup_offset, val);
				break;
			}

			delay(100000);

			val = pci_read(pci, legsup_offset);
		}

		/*
		 * Check that the above loop cleared the BIOS semaphore
		 * and set the OS semaphore.
		 */
		val = pci_read(pci, legsup_offset);

		if ((val & hc_bios_semaphore) != 0)
			return (int)(__LINE__);

		if ((val & hc_os_semaphore) == 0)
			return (int)(__LINE__);

		/*
		 * Initialize Legacy Support Control/Status (USBLEGCTLSTS).
		 */
		val = 0x00000000;
		pci_write(pci, ctlsts_offset, val);
	}

	/*
	 * Handle host controller operational registers.
	 */
	{
		const uint32_t rs_bit = (1u);
		const uint32_t hc_reset_bit = (1u << 1);
		const uint32_t hc_halted_bit = (1u << 12);

		uint32_t *usbcmd_addr = (uint32_t *)(base_cap + 0x00);
		uint32_t *usbsts_addr = (uint32_t *)(base_cap + 0x04);
		uint32_t *usbint_addr = (uint32_t *)(base_cap + 0x08);

		val = cpu_read32(usbcmd_addr);

		/*
		 * Check the Run/Stop (RS) bit and clear if needed.
		 */
		if ((val & rs_bit) != 0) {
			val &= (~rs_bit);
			cpu_write32(usbcmd_addr, val);

			/*
			 * Wait until the HCHalted bit is set (USBSTS).
			 */
			for (i = 0; i < 10000; i++) {
				val = cpu_read32(usbsts_addr);

				if ((val & hc_halted_bit) != 0)
					break;

				delay(100000);
			}
		}

		/*
		 * Set the HCRESET bit.
		 */
		val = cpu_read32(usbcmd_addr) | hc_reset_bit;
		cpu_write32(usbcmd_addr, val);

		delay(100000);

		/*
		 * Wait until the HCRESET bit is cleared.
		 */
		for (i = 0; i < 10000; i++) {
			val = cpu_read32(usbcmd_addr);

			if ((val & hc_reset_bit) == 0)
				break;

			delay(100000);
		}

		/*
		 * Initialize USB Status Register (USBSTS).
		 */
		val = 0x0000003F;
		cpu_write32(usbsts_addr, val);

		/*
		 * Initialize USB Interrupt Enable Register (USBINTR).
		 */
		val = 0x00000000;
		cpu_write32(usbint_addr, val);
	}

	return 0;
}

static int usb_init_xhci(struct pci_device *pci, int early)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFFu;
	phys_addr_t base = usb_get_base(pci, 0x10, early), base_cap = 0;
	phys_addr_t xecp_addr = 0;

	uint32_t cap_length = 0, hci_version = 0;
	uint32_t hcs_params[3] = { 0, 0, 0 };
	uint32_t hcc_params[2] = { 0, 0 };
	uint32_t i, val;

	int mem_enabled = 0;

	if ((cmd & 2u) != 0) {
		if (base < 0x20000)
			return (int)(__LINE__);
		mem_enabled = 1;

	} else if ((cmd & 1u) != 0 && base == 1) {
		return (int)(__LINE__);
	}

	if (mem_enabled) {
		val = cpu_read32((const uint32_t *)(base + 0x00));

		cap_length = (val & 0xFFu);
		base_cap = base + (phys_addr_t)cap_length;
		hci_version = ((val >> 16) & 0xFFFFu);

		hcs_params[0] = cpu_read32((const uint32_t *)(base + 0x04));
		hcs_params[1] = cpu_read32((const uint32_t *)(base + 0x08));
		hcs_params[2] = cpu_read32((const uint32_t *)(base + 0x0C));

		hcc_params[0] = cpu_read32((const uint32_t *)(base + 0x10));
		hcc_params[1] = cpu_read32((const uint32_t *)(base + 0x1C));

		xecp_addr = base + (((hcc_params[0] >> 16) & 0xFFFFu) << 2);
	}

	/*
	 * Make sure that "xECP" points to the USBLEGSUP Register.
	 */
	for (i = 0; xecp_addr != 0; i++) {
		val = cpu_read32((const uint32_t *)(xecp_addr + 0x00));

		/*
		 * Check the Capability ID (value of 1).
		 */
		if ((val & 0xFFu) == 1)
			break;

		if (i == 128)
			return (int)(__LINE__);

		/*
		 * Get the next xHCI Extended Capability Pointer.
		 */
		{
			uint32_t add = ((val >> 8) & 0xFFu) << 2;

			if (add != 0)
				xecp_addr += (phys_addr_t)add;
			else
				xecp_addr = 0;
		}
	}

	if (early) {
		b_log("Extensible Host Controller Interface (xHCI)\n");

		b_log("\tMemory Base Address is %p\n", (const void *)base);

		val = pci_read(pci, 0x3C);
		b_log("\tInterrupt Line is %u and Interrupt PIN is %02X\n",
			(val & 0xFFu), ((val >> 8) & 0xFFu));

		val = pci_read(pci, 0x60) & 0xFFu;
		b_log("\tSerial Bus Release Number is %02X\n", val);

		b_log("\tCapability Register Length is %02X\n", cap_length);
		b_log("\tInterface Version Number is %04X\n", hci_version);

		b_log("\tStructural Parameters 1 are %08X\n", hcs_params[0]);
		b_log("\tStructural Parameters 2 are %08X\n", hcs_params[1]);
		b_log("\tStructural Parameters 3 are %08X\n", hcs_params[2]);

		b_log("\tCapability Parameters 1 are %08X\n", hcc_params[0]);
		b_log("\tCapability Parameters 2 are %08X\n", hcc_params[1]);

		if (mem_enabled) {
			val = cpu_read32((const uint32_t *)(base_cap + 0));
			b_log("\tCommand Register value is %08X\n", val);
			val = cpu_read32((const uint32_t *)(base_cap + 4));
			b_log("\tStatus Register value is %08X\n", val);
		}

		if (xecp_addr) {
			val = cpu_read32((const uint32_t *)(xecp_addr + 0));
			b_log("\tLegacy (xECP + 0) value is %08X\n", val);
			val = cpu_read32((const uint32_t *)(xecp_addr + 4));
			b_log("\tLegacy (xECP + 4) value is %08X\n", val);
		}

		b_log("\n");

		return 0;
	}

	if (!mem_enabled)
		return 0;

	pg_map_uncached((void *)base);

	/*
	 * Handle the HC BIOS Owned Semaphore (USBLEGSUP).
	 */
	if (xecp_addr) {
		const uint32_t smi_bit = (1u << 13);
		const uint32_t hc_bios_semaphore = (1u << 16);
		const uint32_t hc_os_semaphore = (1u << 24);

		uint32_t *legsup_addr = (uint32_t *)(xecp_addr + 0x00);
		uint32_t *ctlsts_addr = (uint32_t *)(xecp_addr + 0x04);

		/*
		 * Request ownership of the EHCI controller.
		 */
		val = cpu_read32(legsup_addr);
		cpu_write32(legsup_addr, (val | hc_os_semaphore));

		for (i = 0; (val & hc_bios_semaphore) != 0; i++) {
			/*
			 * If it seems that the BIOS does not release the
			 * controller, set the SMI on OS Ownership Enable
			 * bit (USBLEGCTLSTS) and try again.
			 */
			if (i == 10000) {
				/*
				 * Clear the SMI bit temporarily.
				 */
				val = cpu_read32(ctlsts_addr);
				val &= (~smi_bit);
				cpu_write32(ctlsts_addr, val);

				/*
				 * Cancel the current ownership request.
				 */
				val = cpu_read32(legsup_addr);
				val &= (~hc_os_semaphore);
				cpu_write32(legsup_addr, val);

				/*
				 * Set the SMI bit.
				 */
				val = cpu_read32(ctlsts_addr);
				val |= smi_bit;
				cpu_write32(ctlsts_addr, val);

				/*
				 * Set the OS Owned Semaphore again.
				 */
				val = cpu_read32(legsup_addr);
				val |= hc_os_semaphore;
				cpu_write32(legsup_addr, val);
			}

			/*
			 * Something went wrong. Take ownership from the BIOS.
			 */
			if (i == 30000) {
				val &= (~hc_bios_semaphore);
				cpu_write32(legsup_addr, val);
				break;
			}

			delay(100000);

			val = cpu_read32(legsup_addr);
		}

		/*
		 * Check that the above loop cleared the BIOS semaphore
		 * and set the OS semaphore.
		 */
		val = cpu_read32(legsup_addr);

		if ((val & hc_bios_semaphore) != 0)
			return (int)(__LINE__);

		if ((val & hc_os_semaphore) == 0)
			return (int)(__LINE__);

		/*
		 * Initialize Legacy Support Control/Status (USBLEGCTLSTS).
		 */
		val = 0x00000000;
		cpu_write32(ctlsts_addr, val);
	}

	/*
	 * Handle host controller operational registers.
	 */
	{
		const uint32_t rs_bit = (1u);
		const uint32_t hc_reset_bit = (1u << 1);
		const uint32_t hc_halted_bit = (1u);

		uint32_t *usbcmd_addr = (uint32_t *)(base_cap + 0x00);
		uint32_t *usbsts_addr = (uint32_t *)(base_cap + 0x04);

		val = cpu_read32(usbcmd_addr);

		/*
		 * Check the Run/Stop (RS) bit and clear if needed.
		 */
		if ((val & rs_bit) != 0) {
			val &= (~rs_bit);
			cpu_write32(usbcmd_addr, val);

			/*
			 * Wait until the HCHalted bit is set (USBSTS).
			 */
			for (i = 0; i < 10000; i++) {
				val = cpu_read32(usbsts_addr);

				if ((val & hc_halted_bit) != 0)
					break;

				delay(100000);
			}
		}

		/*
		 * Set the HCRESET bit.
		 */
		val = cpu_read32(usbcmd_addr) | hc_reset_bit;
		cpu_write32(usbcmd_addr, val);

		delay(100000);

		/*
		 * Wait until the HCRESET bit is cleared.
		 */
		for (i = 0; i < 10000; i++) {
			val = cpu_read32(usbcmd_addr);

			if ((val & hc_reset_bit) == 0)
				break;

			delay(100000);
		}
	}

	return 0;
}

static int usb_init_controllers(int early)
{
	struct pci_device *pci;
	int class_code, i, r;

	for (i = 0; i < (int)pci_device_count; i++) {
		pci = &pci_devices[i];
		class_code = (int)pci->class_code;

		/*
		 * Extensible Host Controller Interface (xHCI).
		 */
		if (class_code == 0x0C0330) {
			if ((r = usb_init_xhci(pci, early)) != 0)
				return r;
		}
	}

	for (i = 0; i < (int)pci_device_count; i++) {
		pci = &pci_devices[i];
		class_code = (int)pci->class_code;

		/*
		 * Enhanced Host Controller Interface (EHCI).
		 */
		if (class_code == 0x0C0320) {
			if ((r = usb_init_ehci(pci, early)) != 0)
				return r;
		}
	}

	for (i = 0; i < (int)pci_device_count; i++) {
		pci = &pci_devices[i];
		class_code = (int)pci->class_code;

		/*
		 * Universal Host Controller Interface (UHCI).
		 */
		if (class_code == 0x0C0300) {
			if ((r = usb_init_uhci(pci, early)) != 0)
				return r;
		}

		/*
		 * Open Host Controller Interface (OHCI).
		 */
		if (class_code == 0x0C0310) {
			if ((r = usb_init_ohci(pci, early)) != 0)
				return r;
		}
	}

	return 0;
}

int usb_init_early(void)
{
	int r;

	/*
	 * Allow early I/O only if using the BIOS boot loader. This
	 * is for writing the init log file. The "usb_init" function
	 * does the actual hardware configurations.
	 */
	if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
		return 0;

	if ((r = usb_init_controllers(1)) != 0) {
		b_print("Error: usb_init_early failure (%d)\n", r);
		return 1;
	}

	return 0;
}

void usb_init(void)
{
	int r;

	if ((r = usb_init_controllers(0)) != 0) {
		char err[32];

		snprintf(&err[0], 32, "Error: usb_init failure (%d)", r);
		panic(&err[0]);
	}
}
