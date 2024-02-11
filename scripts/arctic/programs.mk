# Dancy Operating System

##############################################################################

ARCTIC_PROGRAMS_CAT_OBJECTS_32= \
 ./o32/arctic/programs/cat/main.o \
 ./o32/arctic/programs/cat/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_DSH_OBJECTS_32= \
 ./o32/arctic/programs/dsh/line.o \
 ./o32/arctic/programs/dsh/main.o \
 ./o32/arctic/programs/dsh/operate.o \
 ./o32/arctic/programs/dsh/term.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_INIT_OBJECTS_32= \
 ./o32/arctic/programs/init/main.o \
 ./o32/arctic/programs/init/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32= \
 ./o32/arctic/programs/terminal/main.o \
 ./o32/arctic/programs/terminal/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_CAT_OBJECTS_64= \
 ./o64/arctic/programs/cat/main.o \
 ./o64/arctic/programs/cat/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_DSH_OBJECTS_64= \
 ./o64/arctic/programs/dsh/line.o \
 ./o64/arctic/programs/dsh/main.o \
 ./o64/arctic/programs/dsh/operate.o \
 ./o64/arctic/programs/dsh/term.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_INIT_OBJECTS_64= \
 ./o64/arctic/programs/init/main.o \
 ./o64/arctic/programs/init/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64= \
 ./o64/arctic/programs/terminal/main.o \
 ./o64/arctic/programs/terminal/operate.o \
 ./o64/arctic/libc.a \

##############################################################################

ARCTIC_PROGRAMS_CAT_HEADERS= \
 ./arctic/programs/cat/main.h \

ARCTIC_PROGRAMS_DSH_HEADERS= \
 ./arctic/programs/dsh/main.h \

ARCTIC_PROGRAMS_INIT_HEADERS= \
 ./arctic/programs/init/main.h \

ARCTIC_PROGRAMS_TERMINAL_HEADERS= \
 ./arctic/programs/terminal/main.h \

##############################################################################

./arctic/bin32/cat: $(ARCTIC_PROGRAMS_CAT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_CAT_OBJECTS_32)

./arctic/bin32/dsh: $(ARCTIC_PROGRAMS_DSH_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_DSH_OBJECTS_32)

./arctic/bin32/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)

./arctic/bin32/terminal: $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32)

./arctic/bin64/cat: $(ARCTIC_PROGRAMS_CAT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_CAT_OBJECTS_64)

./arctic/bin64/dsh: $(ARCTIC_PROGRAMS_DSH_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_DSH_OBJECTS_64)

./arctic/bin64/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)

./arctic/bin64/terminal: $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64)
