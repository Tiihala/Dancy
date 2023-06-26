# Dancy Operating System

##############################################################################

DANCY_SYSCALL_OBJECTS_32= \
 ./o32/syscall/a32/trap.o \
 ./o32/syscall/arg.o \
 ./o32/syscall/file.o \
 ./o32/syscall/kill.o \
 ./o32/syscall/sleep.o \
 ./o32/syscall/spawn.o \
 ./o32/syscall/syscall.o \

DANCY_SYSCALL_OBJECTS_64= \
 ./o64/syscall/a64/trap.o \
 ./o64/syscall/arg.o \
 ./o64/syscall/file.o \
 ./o64/syscall/kill.o \
 ./o64/syscall/sleep.o \
 ./o64/syscall/spawn.o \
 ./o64/syscall/syscall.o \

##############################################################################

./o32/syscall.at: $(DANCY_SYSCALL_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_32)

./o64/syscall.at: $(DANCY_SYSCALL_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_64)
