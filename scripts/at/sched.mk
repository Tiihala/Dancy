# Dancy Operating System

##############################################################################

DANCY_SCHED_OBJECTS_32= \
 ./o32/sched/sched.o

DANCY_SCHED_OBJECTS_64= \
 ./o64/sched/sched.o

##############################################################################

./o32/sched.at: $(DANCY_SCHED_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_SCHED_OBJECTS_32)

./o64/sched.at: $(DANCY_SCHED_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_SCHED_OBJECTS_64)
