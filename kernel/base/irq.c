/*
 * Copyright (c) 2021 Antti Tiihala
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
 * base/irq.c
 *      Interrupt Request
 */

#include <dancy.h>

static int irq_lock;
static mtx_t irq_mtx;

static int pic_lock;

struct irq_entry {
	struct irq_entry *next;
	void (*func)(int irq, void *arg);
	void *arg;
	int lock;
};

static struct irq_entry *irq_array[16];

static void irq_empty_func(int irq, void *arg)
{
	(void)irq;
	(void)arg;
}

static int irq_apic(int irq)
{
	int offset = (int)((unsigned int)irq & 15u);
	struct irq_entry *entry = irq_array[offset];

	task_switch_disable();

	while (entry) {
		if (spin_trylock(&entry->lock)) {
			entry->func(irq, entry->arg);
			spin_unlock(&entry->lock);
		}
		entry = entry->next;
	}

	apic_eoi();

	task_switch_enable();

	return 0;
}

static int irq_pic(int irq)
{
	int offset = (int)((unsigned int)irq & 15u);
	struct irq_entry *entry = irq_array[offset];

	/*
	 * Detect spurious interrupt requests.
	 */
	if (irq == 7 || irq == 15) {
		unsigned int isr;

		spin_lock(&pic_lock);

		if (irq == 7) {
			cpu_out8(0x20, 0x0B);
			isr = (unsigned int)cpu_in8(0x20) & 0x80u;
			cpu_out8(0x20, 0x0A);

		} else {
			cpu_out8(0xA0, 0x0B);
			isr = (unsigned int)cpu_in8(0xA0) & 0x80u;
			cpu_out8(0xA0, 0x0A);
		}

		spin_unlock(&pic_lock);

		if (!isr) {
			if (irq == 15)
				cpu_out8(0x20, 0x20);
			return 0;
		}
	}

	task_switch_disable();

	while (entry) {
		if (spin_trylock(&entry->lock)) {
			entry->func(irq, entry->arg);
			spin_unlock(&entry->lock);
		}
		entry = entry->next;
	}

	if (irq >= 8)
		cpu_out8(0xA0, 0x20);
	cpu_out8(0x20, 0x20);

	task_switch_enable();

	return 0;
}

static int irq_nop_apic(int irq)
{
	return (irq < 0) ? 0 : 1;
}

static int irq_nop_pic(int irq)
{
	/*
	 * Spurious IRQ 7 (PIC 1).
	 */
	if (irq == 7)
		return 0;

	/*
	 * Spurious IRQ 15 (PIC 2).
	 */
	if (irq == 15) {
		cpu_out8(0x20, 0x20);
		return 0;
	}

	return (irq < 0) ? 0 : 1;
}

int (*irq_handler_apic)(int irq) = irq_nop_apic;
int (*irq_handler_pic)(int irq) = irq_nop_pic;

int irq_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&irq_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if (kernel->io_apic_enabled)
		irq_handler_apic = irq_apic;
	else
		irq_handler_pic = irq_pic;

	return 0;
}

void irq_disable(int irq)
{
	void *lock_local;

	if (kernel->io_apic_enabled) {
		ioapic_disable(irq);
		return;
	}

	lock_local = &irq_lock;
	spin_enter(&lock_local);

	if (irq >= 0 && irq != 2 && irq <= 7) {
		unsigned int val = (unsigned int)cpu_in8(0x21);

		val |= (unsigned int)(1 << irq);
		cpu_out8(0x21, (uint8_t)val);

	} else if (irq >= 8 && irq <= 15) {
		unsigned int val = (unsigned int)cpu_in8(0xA1);

		val |= (unsigned int)(1 << (irq - 8));
		cpu_out8(0xA1, (uint8_t)val);
	}

	spin_leave(&lock_local);
}

void irq_enable(int irq)
{
	void *lock_local;

	if (kernel->io_apic_enabled) {
		ioapic_enable(irq);
		return;
	}

	lock_local = &irq_lock;
	spin_enter(&lock_local);

	if (irq >= 0 && irq != 2 && irq <= 7) {
		unsigned int val = (unsigned int)cpu_in8(0x21);

		val &= ~((unsigned int)(1 << irq));
		cpu_out8(0x21, (uint8_t)val);

	} else if (irq >= 8 && irq <= 15) {
		unsigned int val = (unsigned int)cpu_in8(0xA1);

		val &= ~((unsigned int)(1 << (irq - 8)));
		cpu_out8(0xA1, (uint8_t)val);
	}

	spin_leave(&lock_local);
}

void *irq_install(int irq, void *arg, void (*func)(int irq, void *arg))
{
	struct irq_entry *entry, *last = NULL;
	int irq_sync = 0;

	if (irq <= 0 || irq == 2 || irq > 15)
		return NULL;

	if (mtx_lock(&irq_mtx) != thrd_success)
		return NULL;

	entry = irq_array[irq];

	while (entry) {
		last = entry;
		if (entry->func == irq_empty_func) {
			entry->arg = arg;
			spin_lock(&irq_sync);
			entry->func = func;
			break;
		}
		entry = entry->next;
	}

	if (!entry) {
		entry = malloc(sizeof(*entry));

		if (entry) {
			memset(entry, 0, sizeof(*entry));

			entry->arg = arg;
			entry->func = func;
			spin_lock(&irq_sync);

			if (last)
				last->next = entry;
			else
				irq_array[irq] = entry;
		}
	}

	mtx_unlock(&irq_mtx);

	return entry;
}

void irq_uninstall(void *irq)
{
	int i;

	if (!irq)
		return;

	if (mtx_lock(&irq_mtx) != thrd_success)
		return;

	for (i = 0; i < 16; i++) {
		struct irq_entry *entry = irq_array[i];

		while (entry) {
			if (entry == (struct irq_entry *)irq) {
				entry->func = irq_empty_func;
				spin_lock(&entry->lock);
				spin_unlock(&entry->lock);
				break;
			}
			entry = entry->next;
		}
	}

	mtx_unlock(&irq_mtx);
}
