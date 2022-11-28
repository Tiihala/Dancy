# Dancy Operating System

##############################################################################

ARCTIC_HELLO_OBJECTS_32= \
 ./o32/_arctic/hello/main.o \
 ./o32/_arctic/libc.a \

ARCTIC_HELLO_OBJECTS_64= \
 ./o64/_arctic/hello/main.o \
 ./o64/_arctic/libc.a \

##############################################################################

./arctic/bin32/hello: $(ARCTIC_HELLO_OBJECTS_32)
	$(DY_LINK) -o$@ $(ARCTIC_HELLO_OBJECTS_32)

./arctic/bin64/hello: $(ARCTIC_HELLO_OBJECTS_64)
	$(DY_LINK) -o$@ $(ARCTIC_HELLO_OBJECTS_64)
