# Dancy Operating System

##############################################################################

ARCTIC_APPS_HELLO_OBJECTS_32= \
 ./o32/arctic/apps/hello/main.o \
 ./o32/arctic/apps/hello/operate.o \
 ./o32/arctic/libc.a \

ARCTIC_APPS_HELLO_OBJECTS_64= \
 ./o64/arctic/apps/hello/main.o \
 ./o64/arctic/apps/hello/operate.o \
 ./o64/arctic/libc.a \

##############################################################################

ARCTIC_APPS_HELLO_HEADERS= \
 ./arctic/apps/hello/main.h \

##############################################################################

./arctic/bin32/hello: $(ARCTIC_APPS_HELLO_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_APPS_HELLO_OBJECTS_32)

./arctic/bin64/hello: $(ARCTIC_APPS_HELLO_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_APPS_HELLO_OBJECTS_64)
