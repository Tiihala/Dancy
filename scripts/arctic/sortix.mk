# Dancy Operating System

##############################################################################

ARCTIC_APPS_EDITOR_OBJECTS_32= \
 ./o32/arctic/apps/editor/__dancy.o \
 ./o32/arctic/apps/editor/command.o \
 ./o32/arctic/apps/editor/cursor.o \
 ./o32/arctic/apps/editor/display.o \
 ./o32/arctic/apps/editor/editor.o \
 ./o32/arctic/apps/editor/highlight.o \
 ./o32/arctic/apps/editor/input.o \
 ./o32/arctic/apps/editor/modal.o \
 ./o32/arctic/apps/editor/multibyte.o \
 ./o32/arctic/apps/editor/terminal.o \
 ./o32/arctic/libc.a \

ARCTIC_APPS_EDITOR_OBJECTS_64= \
 ./o64/arctic/apps/editor/__dancy.o \
 ./o64/arctic/apps/editor/command.o \
 ./o64/arctic/apps/editor/cursor.o \
 ./o64/arctic/apps/editor/display.o \
 ./o64/arctic/apps/editor/editor.o \
 ./o64/arctic/apps/editor/highlight.o \
 ./o64/arctic/apps/editor/input.o \
 ./o64/arctic/apps/editor/modal.o \
 ./o64/arctic/apps/editor/multibyte.o \
 ./o64/arctic/apps/editor/terminal.o \
 ./o64/arctic/libc.a \

##############################################################################

./arctic/bin32/editor: $(ARCTIC_APPS_EDITOR_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_APPS_EDITOR_OBJECTS_32)

./arctic/bin64/editor: $(ARCTIC_APPS_EDITOR_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_APPS_EDITOR_OBJECTS_64)

##############################################################################

SORTIX_DEPS=$(DANCY_EXT) $(DANCY_HEADERS)
SORTIX_TEMP=./external/sortix/Makefile

##############################################################################

./o32/arctic/apps/editor/command.o: \
    ./external/sortix/editor/command.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/command.c

./o32/arctic/apps/editor/cursor.o: \
    ./external/sortix/editor/cursor.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/cursor.c

./o32/arctic/apps/editor/display.o: \
    ./external/sortix/editor/display.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/display.c

./o32/arctic/apps/editor/editor.o: \
    ./external/sortix/editor/editor.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/editor.c

./o32/arctic/apps/editor/highlight.o: \
    ./external/sortix/editor/highlight.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/highlight.c

./o32/arctic/apps/editor/input.o: \
    ./external/sortix/editor/input.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/input.c

./o32/arctic/apps/editor/modal.o: \
    ./external/sortix/editor/modal.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/modal.c

./o32/arctic/apps/editor/multibyte.o: \
    ./external/sortix/editor/multibyte.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/multibyte.c

./o32/arctic/apps/editor/terminal.o: \
    ./external/sortix/editor/terminal.c $(SORTIX_DEPS)
	$(ARCTIC_E_O32)$@ ./external/sortix/editor/terminal.c

./o64/arctic/apps/editor/command.o: \
    ./external/sortix/editor/command.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/command.c

./o64/arctic/apps/editor/cursor.o: \
    ./external/sortix/editor/cursor.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/cursor.c

./o64/arctic/apps/editor/display.o: \
    ./external/sortix/editor/display.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/display.c

./o64/arctic/apps/editor/editor.o: \
    ./external/sortix/editor/editor.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/editor.c

./o64/arctic/apps/editor/highlight.o: \
    ./external/sortix/editor/highlight.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/highlight.c

./o64/arctic/apps/editor/input.o: \
    ./external/sortix/editor/input.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/input.c

./o64/arctic/apps/editor/modal.o: \
    ./external/sortix/editor/modal.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/modal.c

./o64/arctic/apps/editor/multibyte.o: \
    ./external/sortix/editor/multibyte.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/multibyte.c

./o64/arctic/apps/editor/terminal.o: \
    ./external/sortix/editor/terminal.c $(SORTIX_DEPS)
	$(ARCTIC_E_O64)$@ ./external/sortix/editor/terminal.c

##############################################################################

./external/sortix/editor/command.c: $(SORTIX_TEMP)

./external/sortix/editor/cursor.c: $(SORTIX_TEMP)

./external/sortix/editor/display.c: $(SORTIX_TEMP)

./external/sortix/editor/editor.c: $(SORTIX_TEMP)

./external/sortix/editor/highlight.c: $(SORTIX_TEMP)

./external/sortix/editor/input.c: $(SORTIX_TEMP)

./external/sortix/editor/modal.c: $(SORTIX_TEMP)

./external/sortix/editor/multibyte.c: $(SORTIX_TEMP)

./external/sortix/editor/terminal.c: $(SORTIX_TEMP)

##############################################################################

$(SORTIX_TEMP): ./bin/dy-patch$(DANCY_EXE)
	$(SORTIX_SOURCE)
