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
 * base/idt.c
 *      Interrupt Descriptor Table
 */

#include <dancy.h>

static int idt_lock = 1;
static int idt_count = 0;

static struct {
	uint16_t pad1[3];
	uint16_t table_limit;
	uint32_t table_addr;
	uint32_t table_addr_high;
} idt_register;

static uint8_t *idt_global = NULL;
static void *idt_ptr = NULL;

#ifdef DANCY_32

static void idt_build(void)
{
	int i;

	/*
	 * Generate code for each interrupt handler
	 *
	 *   push 0                      ; push error code
	 *   push eax                    ; save register eax
	 *   mov eax, idt_asm_handler    ; eax = address of idt_asm_handler
	 *   push ecx                    ; save register ecx
	 *   mov ecx, <interrupt>        ; ecx = interrupt number
	 *   jmp eax                     ; jmp idt_asm_handler
	 *
	 * The first instruction "push 0" will be replaced with
	 * two "nop" instructions for exceptions that push the
	 * error code, e.g. General Protection Exception.
	 */
	for (i = 0; i < 256; i++) {
		uint32_t *p = (uint32_t *)((addr_t)&idt_asm_array[i][0]);

		if (i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 30)
			p[0] = 0xB8509090;
		else
			p[0] = 0xB850006A;

		p[1] = (uint32_t)((addr_t)&idt_asm_handler[0]);
		p[2] = (uint32_t)((i << 16) | 0x0000B951);
		p[3] = 0xE0FF0000;
	}

	/*
	 * Use a special interrupt handler for Vector 13. This will handle
	 * the cases if segment registers ES or DS contain a null selector.
	 */
	{
		uint32_t *p = (uint32_t *)((addr_t)&idt_asm_array[13][0]);

		p[1] = (uint32_t)((addr_t)&idt_asm_gp_handler[0]);
	}

	/*
	 * Install interrupt descriptors.
	 */
	for (i = 0; i < 256; i++) {
		addr_t addr = (addr_t)&idt_asm_array[i][0];
		uint32_t *p = (uint32_t *)((addr_t)&idt_global[i * 8]);

		p[0] = (uint32_t)((addr & 0x0000FFFF) | 0x00080000);
		p[1] = (uint32_t)((addr & 0xFFFF0000) | 0x00008E00);
	}
}

static void idt_install_global(int num, const uint8_t asm_handler[])
{
	addr_t addr = (addr_t)&asm_handler[0];
	uint32_t *p = (uint32_t *)((addr_t)&idt_global[num * 8]);

	cpu_write32(&p[1], 0);

	cpu_write32(&p[0], (uint32_t)((addr & 0x0000FFFF) | 0x00080000));
	cpu_write32(&p[1], (uint32_t)((addr & 0xFFFF0000) | 0x00008E00));
}

#endif

#ifdef DANCY_64

static void idt_build(void)
{
	int i;

	/*
	 * Generate code for each interrupt handler
	 *
	 *   push 0                      ; push error code
	 *   push rax                    ; save register rax
	 *   mov eax, idt_asm_handler    ; rax = address of idt_asm_handler
	 *   push rcx                    ; save register rcx
	 *   mov ecx, <interrupt>        ; rcx = interrupt number
	 *   jmp rax                     ; jmp idt_asm_handler
	 *
	 * The first instruction "push 0" will be replaced with
	 * two "nop" instructions for exceptions that push the
	 * error code, e.g. General Protection Exception.
	 */
	for (i = 0; i < 256; i++) {
		uint32_t *p = (uint32_t *)((addr_t)&idt_asm_array[i][0]);

		if (i == 8 || (i >= 10 && i <= 14) || i == 17 || i == 30)
			p[0] = 0xB8509090;
		else
			p[0] = 0xB850006A;

		p[1] = (uint32_t)((addr_t)&idt_asm_handler[0]);
		p[2] = (uint32_t)((i << 16) | 0x0000B951);
		p[3] = 0xE0FF0000;
	}

	/*
	 * Install interrupt descriptors.
	 */
	for (i = 0; i < 256; i++) {
		addr_t addr = (addr_t)&idt_asm_array[i][0];
		uint32_t *p = (uint32_t *)((addr_t)&idt_global[i * 16]);

		p[0] = (uint32_t)((addr & 0x0000FFFF) | 0x00080000);
		p[1] = (uint32_t)((addr & 0xFFFF0000) | 0x00008E00);
		p[2] = 0;
		p[3] = 0;
	}
}

static void idt_install_global(int num, const uint8_t asm_handler[])
{
	addr_t addr = (addr_t)&asm_handler[0];
	uint32_t *p = (uint32_t *)((addr_t)&idt_global[num * 16]);

	cpu_write32(&p[1], 0), p[2] = 0, p[3] = 0;

	cpu_write32(&p[0], (uint32_t)((addr & 0x0000FFFF) | 0x00080000));
	cpu_write32(&p[1], (uint32_t)((addr & 0xFFFF0000) | 0x00008E00));
}

#endif

int idt_init(void)
{
	static int run_once;
	size_t size = (size_t)(256 * 2) * sizeof(void *);

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	idt_global = aligned_alloc(size, size);
	if (!idt_global)
		return DE_MEMORY;

	idt_build();

	idt_register.table_limit = (uint16_t)(size - 1);
	idt_register.table_addr = (uint32_t)((addr_t)idt_global);

	idt_ptr = &idt_register.table_limit;
	idt_load(idt_ptr);

	spin_unlock(&idt_lock);

	return 0;
}

int idt_init_ap(void)
{
	spin_lock(&idt_lock);

	if (idt_count >= kernel->smp_ap_count + 1) {
		spin_unlock(&idt_lock);
		return DE_UNEXPECTED;
	}

	idt_count += 1;
	spin_unlock(&idt_lock);

	idt_load(idt_ptr);

	return 0;
}

static void idt_panic(int num, void *stack)
{
	static const char *names[] = {
		"#DE - Divide-by-Zero Exception",
		"#DB - Debug Exception",
		"NMI - Non-Maskable-Interrupt Exception",
		"#BP - Breakpoint Exception",

		"#OF - Overflow Exception",
		"#BR - Bound-Range Exception",
		"#UD - Invalid-Opcode Exception",
		"#NM - Device-Not-Available Exception",

		"#DF - Double-Fault Exception",
		"Coprocessor-Segment-Overrun Exception",
		"#TS - Invalid-TSS Exception",
		"#NP - Segment-Not-Present Exception",

		"#SS - Stack Exception",
		"#GP - General-Protection Exception",
		"#PF - Page-Fault Exception"
	};
	const int entries = (int)(sizeof(names) / sizeof(names[0]));
	addr_t ip = *((addr_t *)stack);
	char b[128];

	if (num < entries)
		snprintf(&b[0], 128, "%s at %p", names[num], (void *)ip);
	else
		snprintf(&b[0], 128, "Interrupt %d", num);

	kernel->panic(&b[0]);
}

void idt_handler(int num, void *stack)
{
	idt_panic(num, stack);
}

void idt_install_asm(int num, const uint8_t asm_handler[])
{
	if (num < 0 || num > 255 || (addr_t)&asm_handler[0] >= 0x10000000)
		return;

	spin_lock(&idt_lock);
	idt_install_global(num, asm_handler);
	spin_unlock(&idt_lock);
}
