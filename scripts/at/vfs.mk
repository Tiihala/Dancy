# Dancy Operating System

##############################################################################

DANCY_VFS_OBJECTS_32= \
 ./o32/vfs/vfs.o

DANCY_VFS_OBJECTS_64= \
 ./o64/vfs/vfs.o

##############################################################################

./o32/vfs.at: $(DANCY_VFS_OBJECTS_32)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_VFS_OBJECTS_32)

./o64/vfs.at: $(DANCY_VFS_OBJECTS_64)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_VFS_OBJECTS_64)
