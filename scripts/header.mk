# Dancy Operating System

DANCY_VERSION=-DDANCY_MAJOR=$(DANCY_MAJOR) -DDANCY_MINOR=$(DANCY_MINOR)

DANCY_SYSTEM_CPPFLAGS=-I. -I./include -I./arctic/include $(DANCY_VERSION)

DANCY_SYSTEM_CFLAGS=-ffreestanding -O2 -mno-80387 -mno-mmx -mno-sse -mno-sse2\
-mno-3dnow -mno-aes -mno-avx -fno-ident

DANCY_ARCTIC_CPPFLAGS=-I./arctic/include\
-D__DANCY_MAJOR=$(DANCY_MAJOR) -D__DANCY_MINOR=$(DANCY_MINOR)

DANCY_ARCTIC_CFLAGS=-ffreestanding -O2 -fno-ident

DANCY_WARNINGS=-Wall -Wconversion -Wextra -Wshadow -Wwrite-strings -pedantic\
-Wpointer-arith -Wcast-align -Wstrict-prototypes

DANCY_CPPFLAGS_32=$(DANCY_SYSTEM_CPPFLAGS) -DDANCY_32
DANCY_CPPFLAGS_64=$(DANCY_SYSTEM_CPPFLAGS) -DDANCY_64
DANCY_CFLAGS_32=-march=i686 $(DANCY_SYSTEM_CFLAGS) $(DANCY_WARNINGS)
DANCY_CFLAGS_64=-march=x86-64 $(DANCY_SYSTEM_CFLAGS) $(DANCY_WARNINGS)

ACPICA_CPPFLAGS_32=-I./include/acpica $(DANCY_SYSTEM_CPPFLAGS) -DDANCY_32
ACPICA_CPPFLAGS_64=-I./include/acpica $(DANCY_SYSTEM_CPPFLAGS) -DDANCY_64
ACPICA_CFLAGS_32=-march=i686 $(DANCY_SYSTEM_CFLAGS)
ACPICA_CFLAGS_64=-march=x86-64 $(DANCY_SYSTEM_CFLAGS)

ARCTIC_CPPFLAGS_32=$(DANCY_ARCTIC_CPPFLAGS) -D__DANCY_32
ARCTIC_CPPFLAGS_64=$(DANCY_ARCTIC_CPPFLAGS) -D__DANCY_64
ARCTIC_CFLAGS_32=-march=i686 $(DANCY_ARCTIC_CFLAGS) $(DANCY_WARNINGS)
ARCTIC_CFLAGS_64=-march=x86-64 $(DANCY_ARCTIC_CFLAGS) $(DANCY_WARNINGS)

DY_BLOB=$(DANCY_DY)-blob$(DANCY_EXE)
DY_CONF=$(DANCY_DY)-conf$(DANCY_EXE)
DY_GPT=$(DANCY_DY)-gpt$(DANCY_EXE)
DY_INIT=$(DANCY_DY)-init$(DANCY_EXE)
DY_ISO=$(DANCY_DY)-iso$(DANCY_EXE)
DY_LINK=$(DANCY_DY)-link$(DANCY_EXE)
DY_MBR=$(DANCY_DY)-mbr$(DANCY_EXE)
DY_MCOPY=$(DANCY_DY)-mcopy$(DANCY_EXE)
DY_PATCH=$(DANCY_DY)-patch$(DANCY_EXE)
DY_PATH=$(DANCY_DY)-path$(DANCY_EXE)
DY_PNG=$(DANCY_DY)-png$(DANCY_EXE)
DY_TTF=$(DANCY_DY)-ttf$(DANCY_EXE)
DY_UEFI=$(DANCY_DY)-uefi$(DANCY_EXE)
DY_VBR=$(DANCY_DY)-vbr$(DANCY_EXE)
DY_ZIP=$(DANCY_DY)-zip$(DANCY_EXE)

DANCY_TARGET_TOOLS= \
 ./bin/dy-blob$(DANCY_EXE) \
 ./bin/dy-conf$(DANCY_EXE) \
 ./bin/dy-gpt$(DANCY_EXE) \
 ./bin/dy-init$(DANCY_EXE) \
 ./bin/dy-iso$(DANCY_EXE) \
 ./bin/dy-link$(DANCY_EXE) \
 ./bin/dy-mbr$(DANCY_EXE) \
 ./bin/dy-mcopy$(DANCY_EXE) \
 ./bin/dy-patch$(DANCY_EXE) \
 ./bin/dy-path$(DANCY_EXE) \
 ./bin/dy-png$(DANCY_EXE) \
 ./bin/dy-ttf$(DANCY_EXE) \
 ./bin/dy-uefi$(DANCY_EXE) \
 ./bin/dy-vbr$(DANCY_EXE) \
 ./bin/dy-zip$(DANCY_EXE) \

DANCY_TARGET_SYSTEM= \
 ./efi/boot/BOOTX64.EFI \
 ./system/CONFIG.AT \
 ./system/DB_000.AT \
 ./system/DB_100.AT \
 ./system/DB_200.AT \
 ./system/DB_300.AT \
 ./system/DB_400.AT \
 ./system/IN_IA16.AT \
 ./system/IN_IA32.AT \
 ./system/IN_X64.AT \
 ./LOADER.512 \
 ./LOADER.AT \

DANCY_TARGET_IMAGES= \
 ./release/dancy.iso \
 ./release/fdd160.img \
 ./release/fdd720a.img \
 ./release/fdd720b.img \
 ./release/fdd1440.img \
 ./release/usbtiny.img \

DANCY_TARGET_RELEASE= \
 ./release/dancy.zip \

DANCY_HEADERS= \
 ./arctic/include/__dancy/core.h \
 ./arctic/include/__dancy/keys.h \
 ./arctic/include/__dancy/mman.h \
 ./arctic/include/__dancy/mode.h \
 ./arctic/include/__dancy/pollfd.h \
 ./arctic/include/__dancy/sched.h \
 ./arctic/include/__dancy/seek.h \
 ./arctic/include/__dancy/signum.h \
 ./arctic/include/__dancy/spawn.h \
 ./arctic/include/__dancy/spin.h \
 ./arctic/include/__dancy/ssize.h \
 ./arctic/include/__dancy/stat.h \
 ./arctic/include/__dancy/syscall.h \
 ./arctic/include/__dancy/timedef.h \
 ./arctic/include/__dancy/timespec.h \
 ./arctic/include/__dancy/tm.h \
 ./arctic/include/sys/mman.h \
 ./arctic/include/sys/stat.h \
 ./arctic/include/sys/types.h \
 ./arctic/include/sys/wait.h \
 ./arctic/include/assert.h \
 ./arctic/include/ctype.h \
 ./arctic/include/dirent.h \
 ./arctic/include/errno.h \
 ./arctic/include/fcntl.h \
 ./arctic/include/signal.h \
 ./arctic/include/spawn.h \
 ./arctic/include/stdio.h \
 ./arctic/include/stdlib.h \
 ./arctic/include/string.h \
 ./arctic/include/threads.h \
 ./arctic/include/time.h \
 ./arctic/include/unistd.h \
 ./include/boot/init.h \
 ./include/boot/loader.h \
 ./include/boot/uefi.h \
 ./include/common/blob.h \
 ./include/common/keys.h \
 ./include/common/lib.h \
 ./include/common/limits.h \
 ./include/common/types.h \
 ./include/kernel/base.h \
 ./include/kernel/debug.h \
 ./include/kernel/error.h \
 ./include/kernel/misc.h \
 ./include/kernel/pci.h \
 ./include/kernel/ps2.h \
 ./include/kernel/run.h \
 ./include/kernel/sched.h \
 ./include/kernel/syscall.h \
 ./include/kernel/table.h \
 ./include/kernel/task.h \
 ./include/kernel/vfs.h \
 ./include/dancy.h \

DANCY_DEPS=$(DANCY_EXT) $(DANCY_HEADERS) $(DANCY_TARGET_TOOLS)
