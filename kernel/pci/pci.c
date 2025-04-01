/*
 * Copyright (c) 2021, 2022, 2025 Antti Tiihala
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
 * pci/pci.c
 *      Peripheral Component Interconnect
 */

#include <dancy.h>

static int pci_lock;

struct handler_entry {
	void (*func)(int irq, void *arg);
	void *arg;
	int lock;
};

static struct handler_entry handler_array[64];

static struct handler_entry *get_handler_entry(int *i)
{
	struct handler_entry *entry = NULL;
	void *lock_local = &pci_lock;

	spin_enter(&lock_local);

	{
		static int array_i;

		if ((*i = array_i) < 64)
			entry = &handler_array[array_i++];
	}

	spin_leave(&lock_local);

	return entry;
}

static void pci_msi_handler(int irq)
{
	struct handler_entry *entry;

	if (irq < 0xA0 || irq > 0xDF)
		return;

	entry = &handler_array[irq - 0xA0];

	task_switch_disable();

	if (spin_trylock(&entry->lock)) {
		if (entry->func)
			entry->func(irq, entry->arg);
		spin_unlock(&entry->lock);
	}

	apic_eoi();

	task_switch_enable();
}

static uint32_t pci_read32_locked(int b, int d, int f, int off)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000 | ((uint32_t)off & 0xFC);

	addr |= (((uint32_t)f & 0x07) << 8);
	addr |= (((uint32_t)d & 0x1F) << 11);
	addr |= (((uint32_t)b & 0xFF) << 16);

	cpu_out32(pci_config_addr, addr);
	return cpu_in32(pci_config_data);
}

static void pci_write32_locked(int b, int d, int f, int off, uint32_t val)
{
	const uint16_t pci_config_addr = 0x0CF8;
	const uint16_t pci_config_data = 0x0CFC;

	uint32_t addr = 0x80000000 | ((uint32_t)off & 0xFC);

	addr |= (((uint32_t)f & 0x07) << 8);
	addr |= (((uint32_t)d & 0x1F) << 11);
	addr |= (((uint32_t)b & 0xFF) << 16);

	cpu_out32(pci_config_addr, addr);
	cpu_out32(pci_config_data, val);
}

int pci_init(void)
{
	static int run_once;
	struct {
		struct _pci_driver *driver;
		int symbol;
	} *driver_array;
	int driver_count = 0;
	int i, j, r;

#ifdef DANCY_32
	const char *name = "__pci_driver_";
#else
	const char *name = "_pci_driver_";
#endif
	size_t name_length = strlen(name);

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	idt_msi_handler = pci_msi_handler;

	if ((driver_array = calloc(1024, sizeof(*driver_array))) == NULL)
		return DE_MEMORY;

	for (i = 0; i < kernel->symbol_count; i++) {
		if (!strncmp(kernel->symbol[i].name, &name[0], name_length)) {
			addr_t a = (addr_t)kernel->symbol[i].value;
			struct _pci_driver *d = (struct _pci_driver *)a;

			driver_array[driver_count].driver = d;
			driver_array[driver_count].symbol = i;

			if ((driver_count++) >= 1024)
				return free(driver_array), DE_OVERFLOW;
		}
	}

	for (i = 0; i < kernel->pci_device_count; i++) {
		struct pci_id *pci = &kernel->pci_device[i];

		for (j = 0; j < driver_count; j++) {
			struct _pci_driver *d = driver_array[j].driver;

			if (d->vendor_id >= 0) {
				if ((uint32_t)d->vendor_id != pci->vendor_id)
					continue;
			}

			if (d->device_id >= 0) {
				if ((uint32_t)d->device_id != pci->device_id)
					continue;
			}

			if (d->class_code_0 >= 0) {
				uint8_t c0 = (uint8_t)(pci->class_code >> 16);
				if ((uint8_t)d->class_code_0 != c0)
					continue;
			}

			if (d->class_code_1 >= 0) {
				uint8_t c1 = (uint8_t)(pci->class_code >> 8);
				if ((uint8_t)d->class_code_1 != c1)
					continue;
			}

			if (d->class_code_2 >= 0) {
				uint8_t c2 = (uint8_t)(pci->class_code >> 0);
				if ((uint8_t)d->class_code_2 != c2)
					continue;
			}

			if ((r = d->init(pci)) != 0) {
				int s = driver_array[j].symbol;
				kernel->print("PCI: %s (%s)\n",
					&kernel->symbol[s].name[name_length],
					strerror(r));
				task_sleep(10000);
				return free(driver_array), r;
			}
		}
	}

	return free(driver_array), 0;
}

uint32_t pci_read(struct pci_id *pci, int offset)
{
	uint32_t ret = 0;

	if (offset < 0 || (offset & 3) != 0)
		return ret;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)offset;

		if (offset <= 0x0FFC) {
			pg_enter_kernel();
			ret = cpu_read32((const uint32_t *)a);
			pg_leave_kernel();
		}

	} else {
		void *lock_local = &pci_lock;
		int b = pci->bus, d = pci->device, f = pci->func;

		if (offset <= 0xFC) {
			spin_enter(&lock_local);
			ret = pci_read32_locked(b, d, f, offset);
			spin_leave(&lock_local);
		}
	}

	return ret;
}

void pci_write(struct pci_id *pci, int offset, uint32_t value)
{
	if (offset < 0 || (offset & 3) != 0)
		return;

	if (pci->ecam) {
		phys_addr_t a = pci->ecam + (phys_addr_t)offset;

		if (offset <= 0x0FFC) {
			pg_enter_kernel();
			cpu_write32((uint32_t *)a, value);
			pg_leave_kernel();
		}

	} else {
		void *lock_local = &pci_lock;
		int b = pci->bus, d = pci->device, f = pci->func;

		if (offset <= 0xFC) {
			spin_enter(&lock_local);
			pci_write32_locked(b, d, f, offset, value);
			spin_leave(&lock_local);
		}
	}
}

void *pci_install_handler(struct pci_id *pci,
	void *arg, void (*func)(int irq, void *arg))
{
	void *r = NULL;
	uint32_t val;

	val = pci_read(pci, 0x04);
	val |= (1u << 10);
	pci_write(pci, 0x04, val);

	if (kernel->io_apic_enabled) {
		uint32_t msi_addr = 0xFEE00000;
		uint32_t dst_id = kernel->apic_bsp_id & 0xFF;
		struct handler_entry *entry;

		if (dst_id == 0xFF)
			return NULL;

		msi_addr |= (dst_id << 12);

		if (pci->cap_msi) {
			int i, c = pci->cap_msi;

			if ((entry = get_handler_entry(&i)) == NULL)
				return NULL;

			if (!spin_trylock(&entry->lock))
				return NULL;

			entry->func = func;
			entry->arg = arg;

			spin_unlock(&entry->lock);

			/*
			 * MSI Enable = 0
			 */
			val = pci_read(pci, c + 0) & 0xFF8EFFFF;
			pci_write(pci, c + 0, val);

			if ((val & 0x00800000) != 0) {
				pci_write(pci, c + 4, msi_addr);
				pci_write(pci, c + 8, 0);
				pci_write(pci, c + 12, (uint32_t)(0xA0 + i));
			} else {
				pci_write(pci, c + 4, msi_addr);
				pci_write(pci, c + 8, (uint32_t)(0xA0 + i));
			}

			/*
			 * MSI Enable = 1
			 */
			pci_write(pci, c + 0, val | 0x00010000);

			return (void *)entry;
		}
	}

	/*
	 * ISA interrupt requests.
	 */
	{
		int irq = (int)(pci_read(pci, 0x3C) & 0xFF);

		if (irq == 0 || irq == 2 || irq > 15)
			return r;

		if ((r = irq_install(irq, arg, func)) != NULL) {
			irq_enable(irq);

			val = pci_read(pci, 0x04);
			val &= (~(1u << 10));
			pci_write(pci, 0x04, val);
		}
	}

	return r;
}
