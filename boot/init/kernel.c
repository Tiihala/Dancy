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
 * init/kernel.c
 *      Kernel initialization
 */

#include <init.h>

struct kernel_table *kernel;

#ifdef DANCY_32
#define OBJECT_PREFIX "o32/"
#define SYMBOL_PREFIX "_"
#endif

#ifdef DANCY_64
#define OBJECT_PREFIX "o64/"
#define SYMBOL_PREFIX ""
#endif

struct kernel_object {
	int optional;
	struct ld_object *ld_obj;
	unsigned char *data;
	const char *name;
};

static struct kernel_object kernel_objects[] = {
	{ 0, NULL, NULL, OBJECT_PREFIX "kernel.at" },
	{ 1, NULL, NULL, OBJECT_PREFIX "acpica.at" }
};

static struct ld_object *ld_obj_array = NULL;

static const size_t stack_size = 0x2000;
static addr_t stack_array_ptr = 0;
static int stack_lock = 0;

static void (*kernel_start_func)(void);
static void (*kernel_start_ap_func)(void);

static int kernel_ap_count = 0;
static int kernel_ap_retry = 0;
static int kernel_ap_lock = 1;

static addr_t kernel_memory_ptr = 0;
static size_t kernel_memory_size = 0;

static addr_t k_base_addr = 0;
static addr_t k_heap_addr = 0;
static addr_t k_stack_array_addr = 0;
static size_t k_stack_array_size = 0;

static void kernel_error(const char *message)
{
	if (message)
		gui_print("%s\n", message);

	gui_refresh();
	delay(4000000000);
	panic("Error: could not start the kernel!");
}

static void kernel_init_memory(void)
{
	const struct b_mem_raw *map = memory_map;

	while (map->base_high == 0) {
		uint32_t type = map[0].type;
		uint32_t b1 = map[0].base_low;
		uint32_t b2 = map[1].base_low;

		if (type == B_MEM_KERNEL) {
			if (kernel_memory_ptr != 0 || b1 > b2)
				kernel_error("kernel_init_memory: error 1");
			kernel_memory_ptr = (addr_t)b1;
			kernel_memory_size = (size_t)(b2 - b1);
		}

		map += 1;
	}

	if (kernel_memory_ptr == 0)
		kernel_error("kernel_init_memory: error 2");
	if ((kernel_memory_ptr & 0x3FFFFFu) != 0)
		kernel_error("kernel_init_memory: error 3");

	if (kernel_memory_size == 0)
		kernel_error("kernel_init_memory: error 4");
	if ((kernel_memory_size & 0x3FFFFFu) != 0)
		kernel_error("kernel_init_memory: error 5");

	/*
	 * Calculate how much memory is needed for stacks (BSP + APs).
	 */
	k_stack_array_size = (size_t)(1 + smp_ap_count) * stack_size;

	if (k_stack_array_size > (kernel_memory_size / 4))
		kernel_error("kernel_init_memory: out of memory");

	kernel_memory_size -= k_stack_array_size;

	/*
	 * Set the memory layout variables.
	 */
	k_base_addr = kernel_memory_ptr;
	k_heap_addr = kernel_memory_ptr;
	k_stack_array_addr = kernel_memory_ptr + (addr_t)kernel_memory_size;

	/*
	 * Set the stack array pointer.
	 */
	stack_array_ptr = k_stack_array_addr;

	/*
	 * Fill the whole kernel memory block with int3 instructions.
	 */
	{
		void *destination = (void *)k_base_addr;
		const int int3_instruction = 0xCC;
		size_t size = kernel_memory_size + k_stack_array_size;

		memset(destination, int3_instruction, size);
	}
}

static void load_object(int i)
{
	const char *name = kernel_objects[i].name;
	unsigned char *data;
	size_t size;
	int r;

	r = db_read(name, &data, &size);

	/*
	 * The return value 1 means that the file cannot be found. This
	 * is not an error for optional objects.
	 */
	if (r == 1 && kernel_objects[i].optional != 0)
		return;

	if (r != 0) {
		char buf[64];

		snprintf(&buf[0], 64, "%s: %s", name, db_read_error(r));
		kernel_error(&buf[0]);
	}

	if (ld_validate(name, data, size))
		kernel_error(NULL);

	kernel_objects[i].data = data;
}

static void link_object(int i)
{
	const char *name = kernel_objects[i].name;
	unsigned char *obj = kernel_objects[i].data;
	struct ld_object ld_obj;

	if (obj == NULL)
		return;

	memset(&ld_obj, 0, sizeof(ld_obj));

	if ((kernel_memory_ptr & 0xFFF) != 0)
		kernel_error("link_object: error 1");
	if ((kernel_memory_size & 0xFFF) != 0)
		kernel_error("link_object: error 2");

	ld_obj.base_address = (void *)kernel_memory_ptr;
	ld_obj.reserved_size = kernel_memory_size;

	/*
	 * Link the kernel object.
	 */
	if (ld_link(name, obj, &ld_obj))
		kernel_error(NULL);

	if (kernel_memory_size < ld_obj.total_size)
		kernel_error("link_object: error 3");

	kernel_memory_ptr += (addr_t)ld_obj.total_size;
	kernel_memory_size -= ld_obj.total_size;

	if (kernel_memory_ptr > (addr_t)(0x10000000 - k_stack_array_size))
		kernel_error("link_object: unexpected memory layout");

	/*
	 * Kernel objects must not use all the memory. Theoretically,
	 * one 4 MiB memory block is enough for the kernel, but at least
	 * half of it must remain free after linking the kernel objects.
	 */
	if (kernel_memory_size < 0x200000)
		kernel_error("link_object: out of memory");

	ld_obj.base_address = NULL;
	ld_obj.reserved_size = 0;

	memcpy(&ld_obj_array[i], &ld_obj, sizeof(ld_obj));

	kernel_objects[i].ld_obj = &ld_obj_array[i];

	k_heap_addr = kernel_memory_ptr;
}

static void prepare_stack(void **stack)
{
	unsigned char *s;

	spin_lock(&stack_lock);

	s = (unsigned char *)stack_array_ptr;
	stack_array_ptr += (addr_t)stack_size;

	if (stack_array_ptr > 0x10000000)
		kernel_error("prepare_stack: unexpected memory layout");

	spin_unlock(&stack_lock);

	memset(s, 0, stack_size);
	s += (stack_size - sizeof(void *) - 32);
	*stack = s;
}

void kernel_init(void)
{
	int object_count = sizeof(kernel_objects) / sizeof(kernel_objects[0]);
	void *stack;
	int i;

	/*
	 * Initialize the dynamic linker.
	 */
	if (ld_init(8192))
		kernel_error(NULL);

	/*
	 * Allocate the ld_obj array.
	 */
	ld_obj_array = calloc((size_t)object_count, sizeof(struct ld_object));
	if (!ld_obj_array)
		kernel_error("kernel_init: out of memory");

	/*
	 * Load the kernel objects.
	 */
	for (i = 0; i < object_count; i++)
		load_object(i);

	/*
	 * Disable the memory manager.
	 */
	memory_disable_manager();

	/*
	 * Initialize kernel memory.
	 */
	kernel_init_memory();

	/*
	 * Link the first kernel object (must be kernel_at).
	 */
	link_object(0);

	/*
	 * Find the __dancy_kernel_table symbol.
	 */
	{
		struct global_symbol *sym;
		addr_t addr;

		if (ld_find("__dancy_kernel_table", &sym))
			kernel_error("kernel.at: missing kernel table");

		if ((addr = sym->value) == 0)
			kernel_error("kernel.at: kernel table is null");

		kernel = (struct kernel_table *)addr;
		kernel->table_size = sizeof(struct kernel_table);
	}

	/*
	 * Find the start symbol (bootstrap processor).
	 */
	{
		struct global_symbol *sym;
		addr_t addr;

		if (ld_find(SYMBOL_PREFIX "kernel_start", &sym))
			kernel_error("kernel.at: missing kernel_start");

		if ((addr = sym->value) == 0)
			kernel_error("kernel.at: kernel_start is null");

		kernel_start_func = (void (*)(void))addr;
	}

	/*
	 * Find the start_ap symbol (application processors).
	 */
	{
		struct global_symbol *sym;
		addr_t addr;

		if (ld_find(SYMBOL_PREFIX "kernel_start_ap", &sym))
			kernel_error("kernel.at: missing kernel_start_ap");

		if ((addr = sym->value) == 0)
			kernel_error("kernel.at: kernel_start_ap is null");

		kernel_start_ap_func = (void (*)(void))addr;
	}

	/*
	 * Link other kernel objects.
	 */
	for (i = 1; i < object_count; i++)
		link_object(i);

	/*
	 * Write the memory layout variables.
	 */
	kernel->base_addr = k_base_addr;
	kernel->heap_addr = k_heap_addr;
	kernel->stack_array_addr = k_stack_array_addr;
	kernel->stack_array_size = k_stack_array_size;

	/*
	 * Initialize the kernel table.
	 */
	table_init();

	/*
	 * Release the lock.
	 */
	spin_unlock(&kernel_ap_lock);

	/*
	 * Send an interrupt to every application processor so
	 * that they wake up from the halted state.
	 */
	while (cpu_read32((uint32_t *)&kernel_ap_count) != smp_ap_count) {
		for (i = 0; i < (int)smp_ap_count; i++) {
			uint32_t ap_id = smp_ap_id[i];
			uint32_t icr_low, icr_high;

			icr_low = 0x000040EF;
			icr_high = ap_id << 24;

			apic_send(icr_low, icr_high);
			delay(200000);
		}

		if (kernel_ap_retry == 0) {
			delay(2000000);
			kernel_ap_retry = 1;
			continue;
		}

		b_print("kernel_init: synchronization failure (%d/%d)\n",
			kernel_ap_count, (int)smp_ap_count);

		cpu_halt(2000);
	}

	gui_refresh();

	/*
	 * Prepare the stack and jump to the kernel.
	 */
	prepare_stack(&stack);
	kernel_jump(kernel_start_func, stack);
}

void kernel_init_ap(uint32_t id)
{
	void *stack;

	/*
	 * Stop execution until an interrupt occurs.
	 */
	cpu_halt(1);

	/*
	 * Acquire the lock, increment the counter, and release the lock.
	 */
	spin_lock(&kernel_ap_lock);

	if (smp_ap_id[kernel_ap_count] != id)
		b_print("kernel_init_ap: unexpected execution order\n");

	kernel_ap_count += 1;
	if (kernel_ap_count > (int)smp_ap_count)
		kernel_error("kernel_init_ap: unexpected behavior");

	spin_unlock(&kernel_ap_lock);

	/*
	 * Prepare the stack and jump to the kernel (AP).
	 */
	prepare_stack(&stack);
	kernel_jump(kernel_start_ap_func, stack);
}
