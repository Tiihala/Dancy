# Dancy Operating System

##############################################################################

./arctic/o32/hello/main.o: ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/hello/main.c

./arctic/o64/hello/main.o: ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/hello/main.c
