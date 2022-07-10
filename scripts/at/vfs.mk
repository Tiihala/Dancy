# Dancy Operating System

##############################################################################

DANCY_VFS_OBJECTS_32= \
 ./o32/_common/fat.o \
 ./o32/vfs/default.o \
 ./o32/vfs/devfs.o \
 ./o32/vfs/fat_io.o \
 ./o32/vfs/path.o \
 ./o32/vfs/root.o \
 ./o32/vfs/vfs.o \

DANCY_VFS_OBJECTS_64= \
 ./o64/_common/fat.o \
 ./o64/vfs/default.o \
 ./o64/vfs/devfs.o \
 ./o64/vfs/fat_io.o \
 ./o64/vfs/path.o \
 ./o64/vfs/root.o \
 ./o64/vfs/vfs.o \

##############################################################################

./o32/vfs.at: $(DANCY_VFS_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_VFS_OBJECTS_32)

./o64/vfs.at: $(DANCY_VFS_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_VFS_OBJECTS_64)
