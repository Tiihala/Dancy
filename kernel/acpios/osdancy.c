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
 * osdancy.c
 *      Dancy Operating System Services Layer for ACPICA
 */

#include <dancy.h>
#include "acpi.h"
#include "accommon.h"

static int acpios_task(void *arg);

int acpios_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (!kernel->acpi_enabled)
		return 0;

	if (!task_create(acpios_task, NULL, task_detached))
		return DE_MEMORY;

	return 0;
}

ACPI_STATUS AcpiOsInitialize(void)
{
	return (AE_OK);
}

ACPI_STATUS AcpiOsTerminate(void)
{
	return (AE_OK);
}

ACPI_STATUS AcpiOsReadMemory(
	ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
	phys_addr_t addr = (phys_addr_t)Address;
	uint64_t val;

	if ((ACPI_PHYSICAL_ADDRESS)addr != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		val = pg_read_memory(addr, 1);
	else if (Width == 16)
		val = pg_read_memory(addr, 2);
	else if (Width == 32)
		val = pg_read_memory(addr, 4);
	else if (Width == 64)
		val = pg_read_memory(addr, 8);
	else
		return (AE_BAD_PARAMETER);

	if (Value)
		*Value = (UINT64)val;

	return (AE_OK);
}

ACPI_STATUS AcpiOsWriteMemory(
	ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
	phys_addr_t addr = (phys_addr_t)Address;
	uint64_t val = (uint64_t)Value;

	if ((ACPI_PHYSICAL_ADDRESS)addr != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		pg_write_memory(addr, val, 1);
	else if (Width == 16)
		pg_write_memory(addr, val, 2);
	else if (Width == 32)
		pg_write_memory(addr, val, 4);
	else if (Width == 64)
		pg_write_memory(addr, val, 8);
	else
		return (AE_BAD_PARAMETER);

	return (AE_OK);
}

ACPI_STATUS AcpiOsReadPort(
	ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width)
{
	uint16_t port = (uint16_t)Address;
	uint32_t val;

	if ((ACPI_IO_ADDRESS)port != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		val = cpu_in8(port);
	else if (Width == 16)
		val = cpu_in16(port);
	else if (Width == 32)
		val = cpu_in32(port);
	else
		return (AE_BAD_PARAMETER);

	if (Value)
		*Value = (UINT32)val;

	return (AE_OK);
}

ACPI_STATUS AcpiOsWritePort(
	ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	uint16_t port = (uint16_t)Address;

	if ((ACPI_IO_ADDRESS)port != Address)
		return (AE_BAD_PARAMETER);

	if (Width == 8)
		cpu_out8(port, (uint8_t)Value);
	else if (Width == 16)
		cpu_out16(port, (uint16_t)Value);
	else if (Width == 32)
		cpu_out32(port, (uint32_t)Value);
	else
		return (AE_BAD_PARAMETER);

	return (AE_OK);
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle)
{
	int *i;

	if (!OutHandle)
		return (AE_BAD_PARAMETER);

	if (!(i = malloc(sizeof(int))))
		return (AE_NO_MEMORY);

	*i = 0;
	*OutHandle = (ACPI_SPINLOCK)i;

	return (AE_OK);
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	free(Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	int r = cpu_ints(0);
	int *lock = (int *)Handle;

	spin_lock(lock);

	return (ACPI_CPU_FLAGS)r;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	int r = (int)Flags;
	int *lock = (int *)Handle;

	spin_unlock(lock);
	cpu_ints(r);
}

static struct {
	ACPI_OSD_HANDLER routine;
	void *native;
	int lock;
} acpios_irq[16];

static void acpios_irq_route(int irq, void *arg)
{
	acpios_irq[irq].routine(arg);
}

ACPI_STATUS AcpiOsInstallInterruptHandler(
	UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine,
	void *Context)
{
	int irq = (int)InterruptNumber;

	if (irq <= 0 || irq == 2 || irq > 15)
		return (AE_BAD_PARAMETER);

	if (!spin_trylock(&acpios_irq[irq].lock))
		return (AE_ALREADY_EXISTS);

	acpios_irq[irq].routine = ServiceRoutine;
	acpios_irq[irq].native = irq_install(irq, Context, acpios_irq_route);

	if (!acpios_irq[irq].native) {
		spin_unlock(&acpios_irq[irq].lock);
		return (AE_BAD_PARAMETER);
	}

	return (AE_OK);
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(
	UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine)
{
	int irq = (int)InterruptNumber;

	if (irq <= 0 || irq == 2 || irq > 15)
		return (AE_BAD_PARAMETER);

	(void)ServiceRoutine;

	if (!acpios_irq[irq].lock)
		return (AE_NOT_EXIST);

	irq_uninstall(acpios_irq[irq].native);
	acpios_irq[irq].native = NULL;

	spin_unlock(&acpios_irq[irq].lock);

	return (AE_OK);
}

void *AcpiOsAllocate(ACPI_SIZE Size)
{
	size_t size = (size_t)Size;

	if ((ACPI_SIZE)size != Size)
		return NULL;

	return malloc(size);
}

void AcpiOsFree(void *Memory)
{
	free(Memory);
}

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Size)
{
	phys_addr_t addr = (phys_addr_t)Where;
	size_t size = (size_t)Size;

	if ((ACPI_PHYSICAL_ADDRESS)addr != Where || addr < 0x1000)
		return NULL;

	if ((ACPI_SIZE)size != Size)
		return NULL;

	return pg_map_kernel(addr, size, pg_normal);
}

void AcpiOsUnmapMemory(void *LogicalAddress, ACPI_SIZE Size)
{
	(void)LogicalAddress;
	(void)Size;
}

ACPI_STATUS AcpiOsPredefinedOverride(
	const ACPI_PREDEFINED_NAMES *InitVal, ACPI_STRING *NewVal)
{
	if (!InitVal || !NewVal)
		return (AE_BAD_PARAMETER);

	*NewVal = NULL;

	return (AE_OK);
}

ACPI_STATUS AcpiOsTableOverride(
	ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable)
{
	if (!ExistingTable || !NewTable)
		return (AE_BAD_PARAMETER);

	*NewTable = NULL;

	return (AE_OK);
}

ACPI_STATUS AcpiOsPhysicalTableOverride(
	ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress,
	UINT32 *NewTableLength)
{
	if (!ExistingTable || !NewAddress || !NewTableLength)
		return (AE_BAD_PARAMETER);

	*NewAddress = 0;
	*NewTableLength = 0;

	return (AE_OK);
}

ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void)
{
	return (ACPI_PHYSICAL_ADDRESS)kernel->acpi->rsdp_addr;
}

ACPI_THREAD_ID AcpiOsGetThreadId(void)
{
	return (ACPI_THREAD_ID)task_current()->id;
}

static struct {
	ACPI_OSD_EXEC_CALLBACK Function;
	void *Context;
} acpios_task_array[2][32];

static int acpios_task_count[2];
static int acpios_task_lock[2];

static uint32_t acpios_events;

static int acpios_task(void *arg)
{
	void *lock_local;
	int count, i;

	while (!arg) {
		int execute_function = 0;
		ACPI_OSD_EXEC_CALLBACK Function;
		void *Context;

		/*
		 * All OSL_NOTIFY_HANDLER functions are executed first.
		 */
		for (i = 0; i < 2 && execute_function == 0; i++) {
			lock_local = &acpios_task_lock[i];
			spin_enter(&lock_local);

			count = acpios_task_count[i];

			if (count > 0) {
				void *dst = &acpios_task_array[i][0];
				void *src = &acpios_task_array[i][1];
				size_t size = sizeof(acpios_task_array[0][0]);

				size *= (size_t)count;
				Function = acpios_task_array[i][0].Function;
				Context = acpios_task_array[i][0].Context;

				/*
				 * The last entry is always unused.
				 */
				memmove(dst, src, size);

				acpios_task_count[i] = (count - 1);
				execute_function = 1;
			}

			spin_leave(&lock_local);
		}

		if (execute_function) {
			task_switch_disable();
			Function(Context);
			cpu_sub32(&acpios_events, 1);
			task_switch_enable();
			continue;
		}

		task_yield();
	}

	return 0;
}

ACPI_STATUS AcpiOsExecute(
	ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function,
	void *Context)
{
	ACPI_STATUS status = (AE_ERROR);
	void *lock_local;
	int i, type;

	if (Type == OSL_NOTIFY_HANDLER)
		type = 0;
	else if (Type == OSL_GPE_HANDLER)
		type = 1;
	else
		return status;

	lock_local = &acpios_task_lock[type];
	spin_enter(&lock_local);

	i = acpios_task_count[type];

	/*
	 * The last entry is always unused.
	 */
	if (i <= 30) {
		acpios_task_array[type][i].Function = Function;
		acpios_task_array[type][i].Context = Context;

		cpu_add32(&acpios_events, 1);
		acpios_task_count[type] = (i + 1);
		status = (AE_OK);
	}

	spin_leave(&lock_local);

	return status;
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	uint64_t current = timer_read();

	while ((timer_read() - current) < (uint64_t)Milliseconds)
		task_yield();
}

void AcpiOsStall(UINT32 Microseconds)
{
	uint32_t q = (uint32_t)(Microseconds / 1000);
	uint32_t r = (uint32_t)((Microseconds % 1000) * 1000);

	while (q--)
		delay(1000000);
	if (r)
		delay(r);
}

void AcpiOsWaitEventsComplete(void)
{
	while (cpu_read32(&acpios_events))
		task_yield();
}
