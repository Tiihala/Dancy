# Dancy Operating System

##############################################################################

ARCTIC_PROGRAMS_INIT_OBJECTS_32= \
 ./o32/arctic/programs/init/main.o \
 ./o32/arctic/programs/init/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_INIT_OBJECTS_64= \
 ./o64/arctic/programs/init/main.o \
 ./o64/arctic/programs/init/operate.o \
 ./o64/arctic/libc.a \

##############################################################################

ARCTIC_PROGRAMS_INIT_HEADERS= \
 ./arctic/programs/init/main.h \

##############################################################################

./arctic/bin32/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)

./arctic/bin64/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)
