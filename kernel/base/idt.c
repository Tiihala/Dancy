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

struct idt_context {
	cpu_native_t eax;
	cpu_native_t ecx;
	cpu_native_t edx;
	cpu_native_t ebx;
	cpu_native_t ebp;
	cpu_native_t esi;
	cpu_native_t edi;

	cpu_native_t cr2;
	cpu_native_t cr3;
};

static void idt_print_context(const struct idt_context *context, char *out)
{
	const struct task *current = task_current();
	const size_t n = 1024;
	int add, off = 0;

	add = snprintf(&out[off], n,
		"eax=%08X ecx=%08X edx=%08X ebx=%08X\n",
		context->eax, context->ecx, context->edx, context->ebx);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"esp=%08X ebp=%08X esi=%08X edi=%08X\n",
		current->iret_frame[3], context->ebp,
		context->esi, context->edi);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"\ncr2=%08X cr3=%08X\n",
		context->cr2, context->cr3);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"eip=%08X  cs=%04X\n\neflags=%08X\n",
		current->iret_frame[0], current->iret_frame[1] & 0xFFFF,
		current->iret_frame[2]);
	off += (add > 0 ? add : 0);

	snprintf(&out[off], n, "\033[1A");
}

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

struct idt_context {
	cpu_native_t rax;
	cpu_native_t rcx;
	cpu_native_t rdx;
	cpu_native_t rbx;
	cpu_native_t rbp;
	cpu_native_t rsi;
	cpu_native_t rdi;

	cpu_native_t r8;
	cpu_native_t r9;
	cpu_native_t r10;
	cpu_native_t r11;
	cpu_native_t r12;
	cpu_native_t r13;
	cpu_native_t r14;
	cpu_native_t r15;

	cpu_native_t cr2;
	cpu_native_t cr3;
};

static void idt_print_context(const struct idt_context *context, char *out)
{
	const struct task *current = task_current();
	const size_t n = 1024;
	int add, off = 0;

	add = snprintf(&out[off], n,
		"rax=%016llX rcx=%016llX\nrdx=%016llX rbx=%016llX\n",
		context->rax, context->rcx, context->rdx, context->rbx);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"rsp=%016llX rbp=%016llX\nrsi=%016llX rdi=%016llX\n",
		current->iret_frame[3], context->rbp,
		context->rsi, context->rdi);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"\n r8=%016llX  r9=%016llX\nr10=%016llX r11=%016llX\n",
		context->r8, context->r9, context->r10, context->r11);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"r12=%016llX r13=%016llX\nr14=%016llX r15=%016llX\n",
		context->r12, context->r13, context->r14, context->r15);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"\ncr2=%016llX cr3=%016llX\n",
		context->cr2, context->cr3);
	off += (add > 0 ? add : 0);

	add = snprintf(&out[off], n,
		"rip=%016llX  cs=%04llX\n\nrflags=%016llX\n",
		current->iret_frame[0], current->iret_frame[1] & 0xFFFF,
		current->iret_frame[2]);
	off += (add > 0 ? add : 0);

	snprintf(&out[off], n, "\033[1A");
}

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
	spin_unlock(&idt_lock);

	/*
	 * Install the timer handler (IRQ 0).
	 */
	if (kernel->io_apic_enabled) {
		const int irq0_apic = 64;
		uint32_t a, *p;

		a = (uint32_t)kernel->apic_base_addr;
		p = (uint32_t *)((addr_t)&timer_apic_base[0]);

		if (cpu_read32(p))
			return DE_UNEXPECTED;
		cpu_write32(p, a);
#ifdef DANCY_64
		a = (uint32_t)(kernel->apic_base_addr >> 32);
		p = (uint32_t *)((addr_t)&timer_apic_base[4]);

		if (cpu_read32(p))
			return DE_UNEXPECTED;
		cpu_write32(p, a);
#endif
		idt_install_asm(irq0_apic, timer_asm_handler_apic);
	} else {
		const int irq0_pic = 32;

		idt_install_asm(irq0_pic, timer_asm_handler_pic);
	}

	idt_load(idt_ptr);

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

void idt_panic(int num, void *stack, struct idt_context *context)
{
	static int run_once;

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
	static char buffer[2048];
	const int entries = (int)(sizeof(names) / sizeof(names[0]));
	struct task *current = task_current();
	int add, off = 0;

	if (context == NULL) {
		cpu_native_t *p = stack;

		if (!spin_trylock(&current->iret_lock))
			cpu_halt(0);

		current->iret_frame[0] = p[0];
		current->iret_frame[1] = p[1];
		current->iret_frame[2] = p[2];

		current->iret_num = num;

		p[0] = (cpu_native_t)&idt_asm_panic[0];
		p[1] = (cpu_native_t)gdt_kernel_code;
		p[2] = (cpu_native_t)0x00000002;

#ifdef DANCY_32
		if ((current->iret_frame[1] & 3) == 0) {
			cpu_native_t iret_esp, iret_ss;

			iret_esp = (cpu_native_t)sizeof(cpu_native_t) * 3;
			iret_esp += (cpu_native_t)stack;
			iret_ss = (cpu_native_t)gdt_kernel_data;

			current->iret_frame[3] = iret_esp;
			current->iret_frame[4] = iret_ss;
		} else {
			current->iret_frame[3] = p[3];
			current->iret_frame[4] = p[4];
			/*
			 * The ESP and SS are not used anymore, but set
			 * them anyway for consistency.
			 */
			p[3] = (cpu_native_t)stack;
			p[4] = (cpu_native_t)gdt_kernel_data;
		}
#else
		current->iret_frame[3] = p[3];
		current->iret_frame[4] = p[4];
		p[3] = (cpu_native_t)stack;
		p[4] = (cpu_native_t)gdt_kernel_data;
#endif
		return;
	}

	if (!spin_trylock(&run_once))
		cpu_halt(0);

	num = current->iret_num;

	if (num < entries)
		add = snprintf(&buffer[0], 128, "%s\n\n", names[num]);
	else
		add = snprintf(&buffer[0], 128, "Interrupt %d\n\n", num);

	off += (add > 0 ? add : 0);
	idt_print_context(context, &buffer[off]);

	kernel->panic(&buffer[0]);
}

void idt_handler(int num, void *stack)
{
	idt_panic(num, stack, NULL);
}

void idt_install_asm(int num, const uint8_t asm_handler[])
{
	if (num < 0 || num > 255 || (addr_t)&asm_handler[0] >= 0x10000000)
		return;

	spin_lock(&idt_lock);
	idt_install_global(num, asm_handler);
	spin_unlock(&idt_lock);
}
