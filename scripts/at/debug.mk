# Dancy Operating System

##############################################################################

DANCY_DEBUG_OBJECTS_32= \
 ./o32/debug/debug.o

DANCY_DEBUG_OBJECTS_64= \
 ./o64/debug/debug.o

##############################################################################

./o32/debug.at: $(DANCY_DEBUG_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_DEBUG_OBJECTS_32)

./o64/debug.at: $(DANCY_DEBUG_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_DEBUG_OBJECTS_64)
