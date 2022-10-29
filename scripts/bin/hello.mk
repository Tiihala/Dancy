# Dancy Operating System

##############################################################################

ARCTIC_HELLO_OBJECTS_32= \
 ./arctic/o32/hello/main.o \
 ./arctic/o32/libc.a \

ARCTIC_HELLO_OBJECTS_64= \
 ./arctic/o64/hello/main.o \
 ./arctic/o64/libc.a \

##############################################################################

./arctic/bin32/hello: $(ARCTIC_HELLO_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_HELLO_OBJECTS_32)

./arctic/bin64/hello: $(ARCTIC_HELLO_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_HELLO_OBJECTS_64)
