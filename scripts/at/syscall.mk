# Dancy Operating System

##############################################################################

DANCY_SYSCALL_OBJECTS_32= \
 ./o32/syscall/syscall.o \

DANCY_SYSCALL_OBJECTS_64= \
 ./o64/syscall/syscall.o \

##############################################################################

./o32/syscall.at: $(DANCY_SYSCALL_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_32)

./o64/syscall.at: $(DANCY_SYSCALL_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_SYSCALL_OBJECTS_64)
