# Dancy Operating System

##############################################################################

./o32/_common/a32/dark.o: ./common/a32/dark.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./common/a32/dark.asm

./o32/_common/a32/string.o: ./common/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./common/a32/string.asm

./o32/_common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32.c

./o32/_common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32c.c

./o32/_common/epoch.o: ./common/epoch.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/epoch.c

./o32/_common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/fat.c

./o32/_common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/ttf.c

./o32/_common/utf8.o: ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/utf8.c

./o64/_common/a64/dark.o: ./common/a64/dark.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./common/a64/dark.asm

./o64/_common/a64/string.o: ./common/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./common/a64/string.asm

./o64/_common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32.c

./o64/_common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32c.c

./o64/_common/epoch.o: ./common/epoch.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/epoch.c

./o64/_common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/fat.c

./o64/_common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/ttf.c

./o64/_common/utf8.o: ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/utf8.c
