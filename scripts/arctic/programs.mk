# Dancy Operating System

##############################################################################

ARCTIC_PROGRAMS_CAT_OBJECTS_32= \
 ./o32/arctic/programs/cat/main.o \
 ./o32/arctic/programs/cat/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_DSH_OBJECTS_32= \
 ./o32/arctic/programs/dsh/builtin.o \
 ./o32/arctic/programs/dsh/execute.o \
 ./o32/arctic/programs/dsh/line.o \
 ./o32/arctic/programs/dsh/main.o \
 ./o32/arctic/programs/dsh/operate.o \
 ./o32/arctic/programs/dsh/parser.o \
 ./o32/arctic/programs/dsh/term.o \
 ./o32/arctic/programs/dsh/token.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_FALSE_OBJECTS_32= \
 ./o32/arctic/programs/false/main.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_32= \
 ./o32/arctic/programs/hexdump/main.o \
 ./o32/arctic/programs/hexdump/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_INIT_OBJECTS_32= \
 ./o32/arctic/programs/init/main.o \
 ./o32/arctic/programs/init/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_LS_OBJECTS_32= \
 ./o32/arctic/programs/ls/main.o \
 ./o32/arctic/programs/ls/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_POWEROFF_OBJECTS_32= \
 ./o32/arctic/programs/poweroff/main.o \
 ./o32/arctic/programs/poweroff/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_REBOOT_OBJECTS_32= \
 ./o32/arctic/programs/reboot/main.o \
 ./o32/arctic/programs/reboot/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_SLEEP_OBJECTS_32= \
 ./o32/arctic/programs/sleep/main.o \
 ./o32/arctic/programs/sleep/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32= \
 ./o32/arctic/programs/terminal/main.o \
 ./o32/arctic/programs/terminal/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_TRUE_OBJECTS_32= \
 ./o32/arctic/programs/true/main.o \
 ./o32/arctic/libc.a \

ARCTIC_PROGRAMS_CAT_OBJECTS_64= \
 ./o64/arctic/programs/cat/main.o \
 ./o64/arctic/programs/cat/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_DSH_OBJECTS_64= \
 ./o64/arctic/programs/dsh/builtin.o \
 ./o64/arctic/programs/dsh/execute.o \
 ./o64/arctic/programs/dsh/line.o \
 ./o64/arctic/programs/dsh/main.o \
 ./o64/arctic/programs/dsh/operate.o \
 ./o64/arctic/programs/dsh/parser.o \
 ./o64/arctic/programs/dsh/term.o \
 ./o64/arctic/programs/dsh/token.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_FALSE_OBJECTS_64= \
 ./o64/arctic/programs/false/main.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_64= \
 ./o64/arctic/programs/hexdump/main.o \
 ./o64/arctic/programs/hexdump/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_INIT_OBJECTS_64= \
 ./o64/arctic/programs/init/main.o \
 ./o64/arctic/programs/init/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_LS_OBJECTS_64= \
 ./o64/arctic/programs/ls/main.o \
 ./o64/arctic/programs/ls/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_POWEROFF_OBJECTS_64= \
 ./o64/arctic/programs/poweroff/main.o \
 ./o64/arctic/programs/poweroff/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_REBOOT_OBJECTS_64= \
 ./o64/arctic/programs/reboot/main.o \
 ./o64/arctic/programs/reboot/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_SLEEP_OBJECTS_64= \
 ./o64/arctic/programs/sleep/main.o \
 ./o64/arctic/programs/sleep/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64= \
 ./o64/arctic/programs/terminal/main.o \
 ./o64/arctic/programs/terminal/operate.o \
 ./o64/arctic/libc.a \

ARCTIC_PROGRAMS_TRUE_OBJECTS_64= \
 ./o64/arctic/programs/true/main.o \
 ./o64/arctic/libc.a \

##############################################################################

ARCTIC_PROGRAMS_CAT_HEADERS= \
 ./arctic/programs/cat/main.h \

ARCTIC_PROGRAMS_DSH_HEADERS= \
 ./arctic/programs/dsh/main.h \

ARCTIC_PROGRAMS_FALSE_HEADERS= \

ARCTIC_PROGRAMS_HEXDUMP_HEADERS= \
 ./arctic/programs/hexdump/main.h \

ARCTIC_PROGRAMS_INIT_HEADERS= \
 ./arctic/programs/init/main.h \

ARCTIC_PROGRAMS_LS_HEADERS= \
 ./arctic/programs/ls/main.h \

ARCTIC_PROGRAMS_POWEROFF_HEADERS= \
 ./arctic/programs/poweroff/main.h \

ARCTIC_PROGRAMS_REBOOT_HEADERS= \
 ./arctic/programs/reboot/main.h \

ARCTIC_PROGRAMS_SLEEP_HEADERS= \
 ./arctic/programs/sleep/main.h \

ARCTIC_PROGRAMS_TERMINAL_HEADERS= \
 ./arctic/programs/terminal/main.h \

ARCTIC_PROGRAMS_TRUE_HEADERS= \

##############################################################################

./arctic/bin32/cat: $(ARCTIC_PROGRAMS_CAT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_CAT_OBJECTS_32)

./arctic/bin32/dsh: $(ARCTIC_PROGRAMS_DSH_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_DSH_OBJECTS_32)

./arctic/bin32/false: $(ARCTIC_PROGRAMS_FALSE_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_FALSE_OBJECTS_32)

./arctic/bin32/hexdump: $(ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_32)

./arctic/bin32/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_32)

./arctic/bin32/ls: $(ARCTIC_PROGRAMS_LS_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_LS_OBJECTS_32)

./arctic/bin32/poweroff: $(ARCTIC_PROGRAMS_POWEROFF_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_POWEROFF_OBJECTS_32)

./arctic/bin32/reboot: $(ARCTIC_PROGRAMS_REBOOT_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_REBOOT_OBJECTS_32)

./arctic/bin32/sleep: $(ARCTIC_PROGRAMS_SLEEP_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_SLEEP_OBJECTS_32)

./arctic/bin32/terminal: $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_32)

./arctic/bin32/true: $(ARCTIC_PROGRAMS_TRUE_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TRUE_OBJECTS_32)

./arctic/bin64/cat: $(ARCTIC_PROGRAMS_CAT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_CAT_OBJECTS_64)

./arctic/bin64/dsh: $(ARCTIC_PROGRAMS_DSH_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_DSH_OBJECTS_64)

./arctic/bin64/false: $(ARCTIC_PROGRAMS_FALSE_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_FALSE_OBJECTS_64)

./arctic/bin64/hexdump: $(ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_HEXDUMP_OBJECTS_64)

./arctic/bin64/init: $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_INIT_OBJECTS_64)

./arctic/bin64/ls: $(ARCTIC_PROGRAMS_LS_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_LS_OBJECTS_64)

./arctic/bin64/poweroff: $(ARCTIC_PROGRAMS_POWEROFF_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_POWEROFF_OBJECTS_64)

./arctic/bin64/reboot: $(ARCTIC_PROGRAMS_REBOOT_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_REBOOT_OBJECTS_64)

./arctic/bin64/sleep: $(ARCTIC_PROGRAMS_SLEEP_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_SLEEP_OBJECTS_64)

./arctic/bin64/terminal: $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TERMINAL_OBJECTS_64)

./arctic/bin64/true: $(ARCTIC_PROGRAMS_TRUE_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_PROGRAMS_TRUE_OBJECTS_64)
