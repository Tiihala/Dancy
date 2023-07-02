# Dancy Operating System

##############################################################################

DANCY_VFS_OBJECTS_32= \
 ./o32/common/fat.o \
 ./o32/kernel/vfs/default.o \
 ./o32/kernel/vfs/devfs.o \
 ./o32/kernel/vfs/fat_io.o \
 ./o32/kernel/vfs/path.o \
 ./o32/kernel/vfs/pipe.o \
 ./o32/kernel/vfs/root.o \
 ./o32/kernel/vfs/vfs.o \

DANCY_VFS_OBJECTS_64= \
 ./o64/common/fat.o \
 ./o64/kernel/vfs/default.o \
 ./o64/kernel/vfs/devfs.o \
 ./o64/kernel/vfs/fat_io.o \
 ./o64/kernel/vfs/path.o \
 ./o64/kernel/vfs/pipe.o \
 ./o64/kernel/vfs/root.o \
 ./o64/kernel/vfs/vfs.o \

##############################################################################

./o32/vfs.at: $(DANCY_VFS_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_VFS_OBJECTS_32)

./o64/vfs.at: $(DANCY_VFS_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_VFS_OBJECTS_64)
