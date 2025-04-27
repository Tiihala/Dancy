/*
 * Copyright (c) 2025 Antti Tiihala
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
 * usb/ehci.c
 *      Enhanced Host Controller Interface (EHCI)
 */

#include <dancy.h>

struct ehci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint32_t irq_count;
	event_t irq_event;

	uint32_t cap_length;
	uint32_t hci_version;
	uint32_t hcs_params;
	uint32_t hcc_params;

	uint8_t *base_cap;

	uint32_t *usb_cmd;
	uint32_t *usb_sts;

	struct dancy_usb_controller *hci;
};

static void ehci_information(struct ehci *ehci)
{
	printk("[EHCI] Base Address %08llX, Size %08X\n",
		(unsigned long long)((addr_t)ehci->base),
		(unsigned int)((addr_t)ehci->size));

	printk("[EHCI] Interface Version %u.%u, "
		"Command Register %08X, Status Register %08X\n",
		(unsigned int)(ehci->hci_version >> 8) & 0xFF,
		(unsigned int)(ehci->hci_version >> 0) & 0xFF,
		(unsigned int)cpu_read32(ehci->usb_cmd),
		(unsigned int)cpu_read32(ehci->usb_sts));
}

static int ehci_irq_task(void *arg)
{
	struct ehci *ehci = arg;

	task_set_cmdline(task_current(), NULL, "[ehci]");
	ehci_information(ehci);

	while (ehci) {
		/*
		 * Wait for the interrupt request.
		 */
		event_wait(ehci->irq_event, 2000);
	}

	return 0;
}

static void ehci_irq_func(int irq, void *arg)
{
	struct ehci *ehci = arg;

	cpu_add32(&ehci->irq_count, 1);
	event_signal(ehci->irq_event);

	(void)irq;
}

static phys_addr_t get_base(struct pci_id *pci, int offset)
{
	phys_addr_t base = (phys_addr_t)pci_read(pci, offset), base_high = 0;
	int io_space = (int)(base & 1);
	int type = (int)((base >> 1) & 3);

	if (io_space)
		return (phys_addr_t)(1);

	base &= 0xFFFFFFF0u;

	if (type == 2) {
		base_high = (phys_addr_t)pci_read(pci, offset + 4);
		base |= ((base_high << 16) << 16);
#ifdef DANCY_32
		if (base_high)
			base = 0;
#endif
	}

	return base;
}

static size_t get_size(struct pci_id *pci, int offset)
{
	uint32_t val, saved;

	saved = pci_read(pci, offset);

	pci_write(pci, offset, 0xFFFFFFFFu);
	val = pci_read(pci, offset);

	pci_write(pci, offset, saved);

	return (size_t)((~(val & 0xFFFFFFF0u)) + 1u);
}

static int ehci_init(struct ehci *ehci)
{
	uint8_t *base = ehci->base;
	uint32_t val;

	if (!(ehci->irq_event = event_create(0)))
		return DE_MEMORY;

	val = cpu_read32(base + 0x00);
	ehci->cap_length = (val & 0xFF);
	ehci->hci_version = ((val >> 16) & 0xFFFF);

	ehci->hcs_params = cpu_read32(base + 0x04);
	ehci->hcc_params = cpu_read32(base + 0x08);

	ehci->base_cap = (base + ehci->cap_length);

	ehci->usb_cmd = (uint32_t *)((void *)(ehci->base_cap + 0x00));
	ehci->usb_sts = (uint32_t *)((void *)(ehci->base_cap + 0x04));

	/*
	 * The boot procesures are responsible of resetting the controller
	 * and taking the ownership from the firmware. HCHalted must be 1.
	 */
	if ((cpu_read32(ehci->usb_sts) & (1u << 12)) == 0) {
		kernel->print("\033[91m[WARNING]\033[m "
			"EHCI HCHalted is 0\n");
		return DE_UNSUPPORTED;
	}

	/*
	 * Install the IRQ handler.
	 */
	if (pci_install_handler(ehci->pci, ehci, ehci_irq_func) == NULL) {
		kernel->print("\033[91m[ERROR]\033[m EHCI IRQ Handling\n");
		return DE_UNSUPPORTED;
	}

	return 0;
}

static int usb_ehci_init(struct pci_id *pci)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFF;
	phys_addr_t addr = get_base(pci, 0x10);
	int r = DE_UNSUPPORTED;

	if ((cmd & 2) != 0 && addr > 1 && addr < SIZE_MAX) {
		size_t size = get_size(pci, 0x10);
		void *base = pg_map_kernel(addr, size, pg_uncached);

		if (base != NULL) {
			const int type = task_detached;
			struct dancy_usb_controller *hci;
			struct ehci *ehci;

			if ((hci = malloc(sizeof(*hci))) == NULL)
				return DE_MEMORY;

			if ((ehci = malloc(sizeof(*ehci))) == NULL)
				return DE_MEMORY;

			memset(ehci, 0, sizeof(*ehci));
			ehci->pci = pci;
			ehci->base = base;
			ehci->size = size;
			ehci->hci = hci;

			memset(hci, 0, sizeof(*hci));
			hci->type = DANCY_USB_CONTROLLER_EHCI;
			hci->pci = pci;
			hci->hci = ehci;

			r = usb_register_controller(hci);

			pci_write(pci, 0x04, cmd | 4);

			if (!r)
				r = ehci_init(ehci);

			if (!r && !task_create(ehci_irq_task, ehci, type))
				r = DE_MEMORY;
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(usb_ehci_init, -1, -1, 0x0C, 0x03, 0x20);
