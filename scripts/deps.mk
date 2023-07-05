# Dancy Operating System

##############################################################################

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
 ./arctic/include/__dancy/termios.h \
 ./arctic/include/__dancy/timedef.h \
 ./arctic/include/__dancy/timespec.h \
 ./arctic/include/__dancy/tm.h \
 ./arctic/include/assert.h \
 ./arctic/include/ctype.h \
 ./arctic/include/dirent.h \
 ./arctic/include/errno.h \
 ./arctic/include/fcntl.h \
 ./arctic/include/poll.h \
 ./arctic/include/signal.h \
 ./arctic/include/spawn.h \
 ./arctic/include/stdio.h \
 ./arctic/include/stdlib.h \
 ./arctic/include/string.h \
 ./arctic/include/sys/mman.h \
 ./arctic/include/sys/stat.h \
 ./arctic/include/sys/types.h \
 ./arctic/include/sys/wait.h \
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
 ./include/dancy.h \
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

##############################################################################

DANCY_DEPS=$(DANCY_EXT) $(DANCY_HEADERS) $(DANCY_TARGET_TOOLS)
