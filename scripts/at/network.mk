# Dancy Operating System

##############################################################################

DANCY_NETWORK_OBJECTS_32= \
 ./o32/kernel/network/e1000.o \
 ./o32/kernel/network/net_node.o \

DANCY_NETWORK_OBJECTS_64= \
 ./o64/kernel/network/e1000.o \
 ./o64/kernel/network/net_node.o \

##############################################################################

./o32/network.at: $(DANCY_NETWORK_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_NETWORK_OBJECTS_32)

./o64/network.at: $(DANCY_NETWORK_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_NETWORK_OBJECTS_64)
