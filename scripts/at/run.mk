# Dancy Operating System

##############################################################################

DANCY_RUN_OBJECTS_32= \
 ./o32/run/run.o \

DANCY_RUN_OBJECTS_64= \
 ./o64/run/run.o \

##############################################################################

./o32/run.at: $(DANCY_RUN_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_RUN_OBJECTS_32)

./o64/run.at: $(DANCY_RUN_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_RUN_OBJECTS_64)
