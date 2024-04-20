# Dancy Operating System

##############################################################################

DANCY_SYSCALL_OBJECTS_32= \
 ./o32/kernel/syscall/a32/trap.o \
 ./o32/kernel/syscall/arg.o \
 ./o32/kernel/syscall/file.o \
 ./o32/kernel/syscall/ioctl.o \
 ./o32/kernel/syscall/kill.o \
 ./o32/kernel/syscall/misc.o \
 ./o32/kernel/syscall/proc.o \
 ./o32/kernel/syscall/reboot.o \
 ./o32/kernel/syscall/sleep.o \
 ./o32/kernel/syscall/spawn.o \
 ./o32/kernel/syscall/syscall.o \

DANCY_SYSCALL_OBJECTS_64= \
 ./o64/kernel/syscall/a64/trap.o \
 ./o64/kernel/syscall/arg.o \
 ./o64/kernel/syscall/file.o \
 ./o64/kernel/syscall/ioctl.o \
 ./o64/kernel/syscall/kill.o \
 ./o64/kernel/syscall/misc.o \
 ./o64/kernel/syscall/proc.o \
 ./o64/kernel/syscall/reboot.o \
 ./o64/kernel/syscall/sleep.o \
 ./o64/kernel/syscall/spawn.o \
 ./o64/kernel/syscall/syscall.o \

##############################################################################

./o32/syscall.at: $(DANCY_SYSCALL_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_32)

./o64/syscall.at: $(DANCY_SYSCALL_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_64)
