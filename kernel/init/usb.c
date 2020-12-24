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

	} else if ((cmd & 2u) != 0) {
		if (io_space == 0)
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

static int usb_init_controllers(int early)
{
	struct pci_device *pci;
	int class_code, i, r;

	for (i = 0; i < (int)pci_device_count; i++) {
		pci = &pci_devices[i];
		class_code = (int)pci->class_code;

		switch (class_code) {
		case 0x0C0300:
			if ((r = usb_init_uhci(pci, early)) != 0)
				return r;
			break;
		default:
			break;
		}
	}

	return 0;
}

int usb_init_early(void)
{
	int r;

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
