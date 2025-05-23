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

static int acpios_failure(ACPI_STATUS status, const char *name)
{
	kernel->print("ACPICA Initialization Failure\n\n");
	kernel->print("\t%s: %08X\n\n", name, (unsigned int)status);

	cpu_halt(4000);
	kernel->print("\033c");

	return 0;
}

int acpios_init(void)
{
	static int run_once;
	ACPI_STATUS status;

	kernel->acpica_available = 1;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (!kernel->acpi_enabled)
		return 0;

	/*
	 * Initialize the ACPICA Subsystem.
	 */
	status = AcpiInitializeSubsystem();

	if (ACPI_FAILURE(status))
		return acpios_failure(status, "AcpiInitializeSubsystem");

	/*
	 * Initialize the ACPICA Table Manager.
	 */
	status = AcpiInitializeTables(NULL, 32, FALSE);

	if (ACPI_FAILURE(status))
		return acpios_failure(status, "AcpiInitializeTables");

	/*
	 * Create the ACPI namespace.
	 */
	status = AcpiLoadTables();

	if (ACPI_FAILURE(status))
		return acpios_failure(status, "AcpiLoadTables");

	/*
	 * Create an ACPICA-specific task.
	 */
	if (!task_create(acpios_task, NULL, task_detached | task_uniproc))
		return DE_MEMORY;

	/*
	 * Initialize the ACPI hardware.
	 */
	status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);

	if (ACPI_FAILURE(status))
		return acpios_failure(status, "AcpiEnableSubsystem");

	/*
	 * Complete the ACPICA initialization.
	 */
	status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);

	if (ACPI_FAILURE(status)) {
		acpios_failure(status, "AcpiInitializeObjects");
		return DE_UNEXPECTED;
	}

	if (!spin_trylock(&kernel->acpica_enabled))
		return DE_UNEXPECTED;

	return 0;
}

static uint32_t acpios_shutdown_status;

int acpios_shutdown(void)
{
	static int run_once;

	if (!kernel->acpica_enabled)
		return DE_UNINITIALIZED;

	if (!spin_trylock(&run_once))
		cpu_halt(0);

	/*
	 * The caller of acpios_shutdown is responsible for making
	 * sure that it is safe to shutdown the computer. Also,
	 * only the bootstrap processor should be active.
	 */
	if (apic_id() != kernel->apic_bsp_id)
		cpu_halt(0);

	cpu_write32(&acpios_shutdown_status, 1);

	while (cpu_read32(&acpios_shutdown_status))
		task_yield();

	return DE_UNEXPECTED;
}

int acpios_reset(void)
{
	static int run_once;

	if (!kernel->acpica_enabled)
		return DE_UNINITIALIZED;

	if (!spin_trylock(&run_once))
		cpu_halt(0);

	/*
	 * The caller of acpios_reset is responsible for making
	 * sure that it is safe to reset the computer. Also,
	 * only the bootstrap processor should be active.
	 */
	if (apic_id() != kernel->apic_bsp_id)
		cpu_halt(0);

	cpu_write32(&acpios_shutdown_status, 3);

	while (cpu_read32(&acpios_shutdown_status))
		task_yield();

	return DE_UNEXPECTED;
}

static void acpios_shutdown_prepare(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return;

	/*
	 * Prepare to enter a sleep state (S5).
	 */
	AcpiEnterSleepStatePrep(5);

	cpu_write32(&acpios_shutdown_status, 2);
}

static void acpios_shutdown_finish(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return;

	/*
	 * Disable interrupts before AcpiEnterSleepState(5).
	 */
	r = cpu_ints(0);

	/*
	 * Enter a sleep state (S5). This function should not return.
	 */
	AcpiEnterSleepState(5);

	cpu_ints(r);
	cpu_write32(&acpios_shutdown_status, 0);
}

static void acpios_reset_finish(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return;

	/*
	 * Perform a system reset.
	 */
	AcpiReset();

	cpu_write32(&acpios_shutdown_status, 0);
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
	pg_enter_kernel();
	acpios_irq[irq].routine(arg);
	pg_leave_kernel();
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

	irq_enable(irq);

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

	task_set_cmdline(task_current(), NULL, "[acpios]");

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

		if (cpu_read32(&acpios_shutdown_status) == 1) {
			acpios_shutdown_prepare();
			continue;
		}

		if (cpu_read32(&acpios_shutdown_status) == 2) {
			acpios_shutdown_finish();
			continue;
		}

		if (cpu_read32(&acpios_shutdown_status) == 3) {
			acpios_reset_finish();
			continue;
		}

		task_sleep(30);
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
	task_sleep((uint64_t)Milliseconds);
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

ACPI_STATUS AcpiOsReadPciConfiguration(
	ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 *Value, UINT32 Width)
{
	int offset = (int)Reg;
	struct pci_id *id = NULL;
	uint64_t val = 0;
	int i;

	if (!Value || offset < 0 || offset > 0x0FFF)
		return (AE_BAD_PARAMETER);

	for (i = 0; i < kernel->pci_device_count; i++) {
		if (kernel->pci_device[i].group != (int)PciId->Segment)
			continue;
		if (kernel->pci_device[i].bus != (int)PciId->Bus)
			continue;
		if (kernel->pci_device[i].device != (int)PciId->Device)
			continue;
		if (kernel->pci_device[i].func != (int)PciId->Function)
			continue;

		id = &kernel->pci_device[i];
		break;
	}

	if (Width == 8) {
		int r = offset % 4;

		if (id)
			val = pci_read(id, (offset & 0x0FFC)) >> (8 * r);
		else if (offset < 4)
			val = 0xFF;

		val &= 0xFF;
		*Value = val;
		return (AE_OK);
	}

	if (Width == 16) {
		int r = offset % 4;

		if ((offset & 1) != 0)
			return (AE_BAD_PARAMETER);

		if (id)
			val = pci_read(id, (offset & 0x0FFC)) >> (8 * r);
		else if (offset < 4)
			val = 0xFFFF;

		val &= 0xFFFF;
		*Value = val;
		return (AE_OK);
	}

	if (Width == 32) {
		if ((offset & 3) != 0)
			return (AE_BAD_PARAMETER);

		if (id)
			val = pci_read(id, offset);
		else if (offset < 4)
			val = 0xFFFFFFFF;

		*Value = val;
		return (AE_OK);
	}

	if (Width == 64) {
		uint64_t val_lo, val_hi;

		if ((offset & 3) != 0)
			return (AE_BAD_PARAMETER);

		if (id) {
			val_lo = pci_read(id, offset);
			val_hi = pci_read(id, offset + 4);
			val = ((val_hi << 16) << 16) | val_lo;
		} else if (offset < 4) {
			val = 0xFFFFFFFF;
		}

		*Value = val;
		return (AE_OK);
	}

	return (AE_BAD_PARAMETER);
}

ACPI_STATUS AcpiOsWritePciConfiguration(
	ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 Value, UINT32 Width)
{
	int offset = (int)Reg;
	struct pci_id *id = NULL;
	uint32_t val = 0;
	int i;

	if (offset < 0 || offset > 0x0FFF)
		return (AE_BAD_PARAMETER);

	for (i = 0; i < kernel->pci_device_count; i++) {
		if (kernel->pci_device[i].group != (int)PciId->Segment)
			continue;
		if (kernel->pci_device[i].bus != (int)PciId->Bus)
			continue;
		if (kernel->pci_device[i].device != (int)PciId->Device)
			continue;
		if (kernel->pci_device[i].func != (int)PciId->Function)
			continue;

		id = &kernel->pci_device[i];
		break;
	}

	if (Width == 8) {
		int r = offset % 4;

		if (id) {
			val = pci_read(id, (offset & 0x0FFC));

			val &= ~((uint32_t)(0xFFu << (8 * r)));
			val |= ((uint32_t)Value & 0xFF) << (8 * r);

			pci_write(id, (offset & 0x0FFC), val);
		}

		return (AE_OK);
	}

	if (Width == 16) {
		int r = offset % 4;

		if ((offset & 1) != 0)
			return (AE_BAD_PARAMETER);

		if (id) {
			val = pci_read(id, (offset & 0x0FFC));

			val &= ~((uint32_t)(0xFFFFu << (8 * r)));
			val |= ((uint32_t)Value & 0xFFFF) << (8 * r);

			pci_write(id, (offset & 0x0FFC), val);
		}

		return (AE_OK);
	}

	if (Width == 32) {
		if ((offset & 3) != 0)
			return (AE_BAD_PARAMETER);

		if (id) {
			val = (uint32_t)Value;
			pci_write(id, offset, val);
		}

		return (AE_OK);
	}

	if (Width == 64) {
		if ((offset & 3) != 0)
			return (AE_BAD_PARAMETER);

		if (id) {
			val = (uint32_t)Value;
			pci_write(id, offset, val);

			val = (uint32_t)((Value >> 16) >> 16);
			pci_write(id, offset + 4, val);
		}

		return (AE_OK);
	}

	return (AE_BAD_PARAMETER);
}

static size_t acpios_log_size;
static char acpios_log[256];

static int acpios_log_lock;

void AcpiOsPrintf(const char *Format, ...)
{
	va_list Args;
	va_start(Args, Format);
	AcpiOsVprintf(Format, Args);
	va_end(Args);
}

void AcpiOsVprintf(const char *Format, va_list Args)
{
	static char buffer[128];
	void *lock_local = &acpios_log_lock;
	char *ptr = &buffer[0];
	int r;

	spin_enter(&lock_local);
	r = vsnprintf(buffer, sizeof(buffer), Format, Args);

	while (r > 0 && r < (int)sizeof(buffer)) {
		/*
		 * The log is always terminated with '\0'.
		 */
		if ((size_t)(acpios_log_size + 1) >= sizeof(acpios_log))
			break;

		acpios_log[acpios_log_size++] = *ptr++;
		r -= 1;

		if (acpios_log[acpios_log_size - 1] == '\n') {
			char *log_ptr = &acpios_log[0];

			log_ptr[--acpios_log_size] = '\0';

			if (!strncmp(log_ptr, "ACPI: ", 6))
				log_ptr += 6;

			printk("[ACPICA] %s\n", log_ptr);

			memset(acpios_log, 0, acpios_log_size);
			acpios_log_size = 0;
		}
	}

	spin_leave(&lock_local);
}

UINT64 AcpiOsGetTimer(void)
{
	uint64_t msec = timer_read();

	return (msec * (uint64_t)(ACPI_100NSEC_PER_MSEC));
}

ACPI_STATUS AcpiOsEnterSleep(
	UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue)
{
	(void)SleepState;
	(void)RegaValue;
	(void)RegbValue;

	return (AE_OK);
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void *Info)
{
	(void)Function;
	(void)Info;

	return (AE_OK);
}

struct acpios_semaphore {
	uint32_t max_units;
	uint32_t units;
	int lock;
	int yield;
};

ACPI_STATUS AcpiOsCreateSemaphore(
	UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle)
{
	struct acpios_semaphore *s;

	if (!OutHandle || MaxUnits < InitialUnits)
		return (AE_BAD_PARAMETER);

	if ((s = malloc(sizeof(*s))) == NULL)
		return (AE_NO_MEMORY);

	memset(s, 0, sizeof(*s));
	cpu_add32(&s->max_units, (uint32_t)MaxUnits);
	cpu_add32(&s->units, (uint32_t)InitialUnits);

	*OutHandle = (ACPI_SEMAPHORE)s;
	return (AE_OK);
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	free((void *)Handle);
	return (AE_OK);
}

ACPI_STATUS AcpiOsWaitSemaphore(
	ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	struct acpios_semaphore *s = (struct acpios_semaphore *)Handle;
	uint64_t current = timer_read();
	void *lock_local;

	if (!s)
		return (AE_BAD_PARAMETER);

	lock_local = &s->lock;

	for (;;) {
		spin_enter(&lock_local);

		if (s->units >= (uint32_t)Units) {
			s->units -= (uint32_t)Units;

			spin_leave(&lock_local);
			break;
		}

		s->yield = 1;
		spin_leave(&lock_local);

		if (Timeout != 0xFFFF) {
			if ((timer_read() - current) >= (uint64_t)Timeout)
				return (AE_TIME);
		}

		task_yield();
	}

	return (AE_OK);
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	struct acpios_semaphore *s = (struct acpios_semaphore *)Handle;
	ACPI_STATUS status = (AE_OK);
	void *lock_local;
	int yield;

	if (!s)
		return (AE_BAD_PARAMETER);

	lock_local = &s->lock;
	spin_enter(&lock_local);

	if ((uint64_t)s->units + (uint64_t)Units <= (uint64_t)s->max_units)
		s->units += (uint32_t)Units;
	else
		status = (AE_LIMIT);

	yield = s->yield;
	s->yield = 0;

	spin_leave(&lock_local);

	if (yield)
		task_yield();

	return status;
}
