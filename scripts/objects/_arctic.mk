# Dancy Operating System

##############################################################################

./o32/_arctic/hello/main.o: \
     ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/hello/main.c

./o32/_arctic/libc/a32/memcmp.o: \
     ./arctic/libc/a32/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcmp.asm

./o32/_arctic/libc/a32/memcpy.o: \
     ./arctic/libc/a32/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcpy.asm

./o32/_arctic/libc/a32/memmove.o: \
     ./arctic/libc/a32/memmove.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memmove.asm

./o32/_arctic/libc/a32/memset.o: \
     ./arctic/libc/a32/memset.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memset.asm

./o32/_arctic/libc/a32/spin.o: \
     ./arctic/libc/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/spin.asm

./o32/_arctic/libc/a32/start.o: \
     ./arctic/libc/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/start.asm

./o32/_arctic/libc/a32/strlen.o: \
     ./arctic/libc/a32/strlen.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/strlen.asm

./o32/_arctic/libc/a32/syscall0.o: \
     ./arctic/libc/a32/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall0.asm

./o32/_arctic/libc/a32/syscall1.o: \
     ./arctic/libc/a32/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall1.asm

./o32/_arctic/libc/a32/syscall2.o: \
     ./arctic/libc/a32/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall2.asm

./o32/_arctic/libc/a32/syscall3.o: \
     ./arctic/libc/a32/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall3.asm

./o32/_arctic/libc/a32/syscall4.o: \
     ./arctic/libc/a32/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall4.asm

./o32/_arctic/libc/a32/syscall5.o: \
     ./arctic/libc/a32/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall5.asm

./o32/_arctic/libc/ctype/isalnum.o: \
     ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalnum.c

./o32/_arctic/libc/ctype/isalpha.o: \
     ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalpha.c

./o32/_arctic/libc/ctype/isblank.o: \
     ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isblank.c

./o32/_arctic/libc/ctype/iscntrl.o: \
     ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/iscntrl.c

./o32/_arctic/libc/ctype/isdigit.o: \
     ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isdigit.c

./o32/_arctic/libc/ctype/isgraph.o: \
     ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isgraph.c

./o32/_arctic/libc/ctype/islower.o: \
     ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/islower.c

./o32/_arctic/libc/ctype/isprint.o: \
     ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isprint.c

./o32/_arctic/libc/ctype/ispunct.o: \
     ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/ispunct.c

./o32/_arctic/libc/ctype/isspace.o: \
     ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isspace.c

./o32/_arctic/libc/ctype/isupper.o: \
     ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isupper.c

./o32/_arctic/libc/ctype/isxdigit.o: \
     ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isxdigit.c

./o32/_arctic/libc/ctype/tolower.o: \
     ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/tolower.c

./o32/_arctic/libc/ctype/toupper.o: \
     ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/toupper.c

./o32/_arctic/libc/dirent/dir.o: \
     ./arctic/libc/dirent/dir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/dir.c

./o32/_arctic/libc/dirent/dirfd.o: \
     ./arctic/libc/dirent/dirfd.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/dirfd.c

./o32/_arctic/libc/dirent/rewind.o: \
     ./arctic/libc/dirent/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/rewind.c

./o32/_arctic/libc/errno/errno.o: \
     ./arctic/libc/errno/errno.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/errno/errno.c

./o32/_arctic/libc/fcntl/fcntl.o: \
     ./arctic/libc/fcntl/fcntl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/fcntl/fcntl.c

./o32/_arctic/libc/fcntl/open.o: \
     ./arctic/libc/fcntl/open.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/fcntl/open.c

./o32/_arctic/libc/misc/__main.o: \
     ./arctic/libc/misc/__main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/__main.c

./o32/_arctic/libc/spawn/actions.o: \
     ./arctic/libc/spawn/actions.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/actions.c

./o32/_arctic/libc/spawn/addclose.o: \
     ./arctic/libc/spawn/addclose.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/addclose.c

./o32/_arctic/libc/spawn/adddup2.o: \
     ./arctic/libc/spawn/adddup2.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/adddup2.c

./o32/_arctic/libc/spawn/addopen.o: \
     ./arctic/libc/spawn/addopen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/addopen.c

./o32/_arctic/libc/spawn/attr.o: \
     ./arctic/libc/spawn/attr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/attr.c

./o32/_arctic/libc/spawn/flags.o: \
     ./arctic/libc/spawn/flags.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/flags.c

./o32/_arctic/libc/spawn/pgroup.o: \
     ./arctic/libc/spawn/pgroup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/pgroup.c

./o32/_arctic/libc/spawn/policy.o: \
     ./arctic/libc/spawn/policy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/policy.c

./o32/_arctic/libc/spawn/sched.o: \
     ./arctic/libc/spawn/sched.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sched.c

./o32/_arctic/libc/spawn/sigdef.o: \
     ./arctic/libc/spawn/sigdef.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sigdef.c

./o32/_arctic/libc/spawn/sigmask.o: \
     ./arctic/libc/spawn/sigmask.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sigmask.c

./o32/_arctic/libc/spawn/spawn.o: \
     ./arctic/libc/spawn/spawn.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/spawn.c

./o32/_arctic/libc/spawn/spawnp.o: \
     ./arctic/libc/spawn/spawnp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/spawnp.c

./o32/_arctic/libc/stdio/_io.o: \
     ./arctic/libc/stdio/_io.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/_io.c

./o32/_arctic/libc/stdio/clearerr.o: \
     ./arctic/libc/stdio/clearerr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/clearerr.c

./o32/_arctic/libc/stdio/fclose.o: \
     ./arctic/libc/stdio/fclose.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fclose.c

./o32/_arctic/libc/stdio/feof.o: \
     ./arctic/libc/stdio/feof.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/feof.c

./o32/_arctic/libc/stdio/ferror.o: \
     ./arctic/libc/stdio/ferror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/ferror.c

./o32/_arctic/libc/stdio/fflush.o: \
     ./arctic/libc/stdio/fflush.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fflush.c

./o32/_arctic/libc/stdio/fgetc.o: \
     ./arctic/libc/stdio/fgetc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fgetc.c

./o32/_arctic/libc/stdio/fputc.o: \
     ./arctic/libc/stdio/fputc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fputc.c

./o32/_arctic/libc/stdio/fread.o: \
     ./arctic/libc/stdio/fread.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fread.c

./o32/_arctic/libc/stdio/fwrite.o: \
     ./arctic/libc/stdio/fwrite.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fwrite.c

./o32/_arctic/libc/stdio/rename.o: \
     ./arctic/libc/stdio/rename.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/rename.c

./o32/_arctic/libc/stdio/snprintf.o: \
     ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/snprintf.c

./o32/_arctic/libc/stdlib/alloc.o: \
     ./arctic/libc/stdlib/alloc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/alloc.c

./o32/_arctic/libc/stdlib/qsort.o: \
     ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/qsort.c

./o32/_arctic/libc/string/strcat.o: \
     ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcat.c

./o32/_arctic/libc/string/strcmp.o: \
     ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcmp.c

./o32/_arctic/libc/string/strcpy.o: \
     ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcpy.c

./o32/_arctic/libc/string/strerror.o: \
     ./arctic/libc/string/strerror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strerror.c

./o32/_arctic/libc/string/strncmp.o: \
     ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncmp.c

./o32/_arctic/libc/string/strncpy.o: \
     ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncpy.c

./o32/_arctic/libc/sys/fstat.o: \
     ./arctic/libc/sys/fstat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/fstat.c

./o32/_arctic/libc/sys/lstat.o: \
     ./arctic/libc/sys/lstat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/lstat.c

./o32/_arctic/libc/sys/mkdir.o: \
     ./arctic/libc/sys/mkdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mkdir.c

./o32/_arctic/libc/sys/mmap.o: \
     ./arctic/libc/sys/mmap.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mmap.c

./o32/_arctic/libc/sys/mprotect.o: \
     ./arctic/libc/sys/mprotect.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mprotect.c

./o32/_arctic/libc/sys/msync.o: \
     ./arctic/libc/sys/msync.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/msync.c

./o32/_arctic/libc/sys/munmap.o: \
     ./arctic/libc/sys/munmap.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/munmap.c

./o32/_arctic/libc/sys/stat.o: \
     ./arctic/libc/sys/stat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/stat.c

./o32/_arctic/libc/sys/wait.o: \
     ./arctic/libc/sys/wait.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/wait.c

./o32/_arctic/libc/sys/waitpid.o: \
     ./arctic/libc/sys/waitpid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/waitpid.c

./o32/_arctic/libc/threads/mtx.o: \
     ./arctic/libc/threads/mtx.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/threads/mtx.c

./o32/_arctic/libc/time/gettime.o: \
     ./arctic/libc/time/gettime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/gettime.c

./o32/_arctic/libc/time/settime.o: \
     ./arctic/libc/time/settime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/settime.c

./o32/_arctic/libc/time/sleep.o: \
     ./arctic/libc/time/sleep.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/sleep.c

./o32/_arctic/libc/time/time.o: \
     ./arctic/libc/time/time.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/time.c

./o32/_arctic/libc/unistd/_exit.o: \
     ./arctic/libc/unistd/_exit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/_exit.c

./o32/_arctic/libc/unistd/chdir.o: \
     ./arctic/libc/unistd/chdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/chdir.c

./o32/_arctic/libc/unistd/close.o: \
     ./arctic/libc/unistd/close.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/close.c

./o32/_arctic/libc/unistd/dup.o: \
     ./arctic/libc/unistd/dup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/dup.c

./o32/_arctic/libc/unistd/dup2.o: \
     ./arctic/libc/unistd/dup2.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/dup2.c

./o32/_arctic/libc/unistd/execve.o: \
     ./arctic/libc/unistd/execve.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/execve.c

./o32/_arctic/libc/unistd/getcwd.o: \
     ./arctic/libc/unistd/getcwd.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getcwd.c

./o32/_arctic/libc/unistd/getpid.o: \
     ./arctic/libc/unistd/getpid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getpid.c

./o32/_arctic/libc/unistd/getppid.o: \
     ./arctic/libc/unistd/getppid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getppid.c

./o32/_arctic/libc/unistd/lseek.o: \
     ./arctic/libc/unistd/lseek.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/lseek.c

./o32/_arctic/libc/unistd/pipe.o: \
     ./arctic/libc/unistd/pipe.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/pipe.c

./o32/_arctic/libc/unistd/read.o: \
     ./arctic/libc/unistd/read.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/read.c

./o32/_arctic/libc/unistd/rmdir.o: \
     ./arctic/libc/unistd/rmdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/rmdir.c

./o32/_arctic/libc/unistd/unlink.o: \
     ./arctic/libc/unistd/unlink.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/unlink.c

./o32/_arctic/libc/unistd/write.o: \
     ./arctic/libc/unistd/write.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/write.c

./o32/_arctic/libc/start.o: \
     ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/start.c

./o64/_arctic/hello/main.o: \
     ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/hello/main.c

./o64/_arctic/libc/a64/memcmp.o: \
     ./arctic/libc/a64/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcmp.asm

./o64/_arctic/libc/a64/memcpy.o: \
     ./arctic/libc/a64/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcpy.asm

./o64/_arctic/libc/a64/memmove.o: \
     ./arctic/libc/a64/memmove.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memmove.asm

./o64/_arctic/libc/a64/memset.o: \
     ./arctic/libc/a64/memset.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memset.asm

./o64/_arctic/libc/a64/spin.o: \
     ./arctic/libc/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/spin.asm

./o64/_arctic/libc/a64/start.o: \
     ./arctic/libc/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/start.asm

./o64/_arctic/libc/a64/strlen.o: \
     ./arctic/libc/a64/strlen.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/strlen.asm

./o64/_arctic/libc/a64/syscall0.o: \
     ./arctic/libc/a64/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall0.asm

./o64/_arctic/libc/a64/syscall1.o: \
     ./arctic/libc/a64/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall1.asm

./o64/_arctic/libc/a64/syscall2.o: \
     ./arctic/libc/a64/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall2.asm

./o64/_arctic/libc/a64/syscall3.o: \
     ./arctic/libc/a64/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall3.asm

./o64/_arctic/libc/a64/syscall4.o: \
     ./arctic/libc/a64/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall4.asm

./o64/_arctic/libc/a64/syscall5.o: \
     ./arctic/libc/a64/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall5.asm

./o64/_arctic/libc/ctype/isalnum.o: \
     ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalnum.c

./o64/_arctic/libc/ctype/isalpha.o: \
     ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalpha.c

./o64/_arctic/libc/ctype/isblank.o: \
     ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isblank.c

./o64/_arctic/libc/ctype/iscntrl.o: \
     ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/iscntrl.c

./o64/_arctic/libc/ctype/isdigit.o: \
     ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isdigit.c

./o64/_arctic/libc/ctype/isgraph.o: \
     ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isgraph.c

./o64/_arctic/libc/ctype/islower.o: \
     ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/islower.c

./o64/_arctic/libc/ctype/isprint.o: \
     ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isprint.c

./o64/_arctic/libc/ctype/ispunct.o: \
     ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/ispunct.c

./o64/_arctic/libc/ctype/isspace.o: \
     ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isspace.c

./o64/_arctic/libc/ctype/isupper.o: \
     ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isupper.c

./o64/_arctic/libc/ctype/isxdigit.o: \
     ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isxdigit.c

./o64/_arctic/libc/ctype/tolower.o: \
     ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/tolower.c

./o64/_arctic/libc/ctype/toupper.o: \
     ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/toupper.c

./o64/_arctic/libc/dirent/dir.o: \
     ./arctic/libc/dirent/dir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/dir.c

./o64/_arctic/libc/dirent/dirfd.o: \
     ./arctic/libc/dirent/dirfd.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/dirfd.c

./o64/_arctic/libc/dirent/rewind.o: \
     ./arctic/libc/dirent/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/rewind.c

./o64/_arctic/libc/errno/errno.o: \
     ./arctic/libc/errno/errno.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/errno/errno.c

./o64/_arctic/libc/fcntl/fcntl.o: \
     ./arctic/libc/fcntl/fcntl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/fcntl/fcntl.c

./o64/_arctic/libc/fcntl/open.o: \
     ./arctic/libc/fcntl/open.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/fcntl/open.c

./o64/_arctic/libc/misc/__main.o: \
     ./arctic/libc/misc/__main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/__main.c

./o64/_arctic/libc/spawn/actions.o: \
     ./arctic/libc/spawn/actions.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/actions.c

./o64/_arctic/libc/spawn/addclose.o: \
     ./arctic/libc/spawn/addclose.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/addclose.c

./o64/_arctic/libc/spawn/adddup2.o: \
     ./arctic/libc/spawn/adddup2.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/adddup2.c

./o64/_arctic/libc/spawn/addopen.o: \
     ./arctic/libc/spawn/addopen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/addopen.c

./o64/_arctic/libc/spawn/attr.o: \
     ./arctic/libc/spawn/attr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/attr.c

./o64/_arctic/libc/spawn/flags.o: \
     ./arctic/libc/spawn/flags.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/flags.c

./o64/_arctic/libc/spawn/pgroup.o: \
     ./arctic/libc/spawn/pgroup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/pgroup.c

./o64/_arctic/libc/spawn/policy.o: \
     ./arctic/libc/spawn/policy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/policy.c

./o64/_arctic/libc/spawn/sched.o: \
     ./arctic/libc/spawn/sched.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sched.c

./o64/_arctic/libc/spawn/sigdef.o: \
     ./arctic/libc/spawn/sigdef.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sigdef.c

./o64/_arctic/libc/spawn/sigmask.o: \
     ./arctic/libc/spawn/sigmask.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sigmask.c

./o64/_arctic/libc/spawn/spawn.o: \
     ./arctic/libc/spawn/spawn.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/spawn.c

./o64/_arctic/libc/spawn/spawnp.o: \
     ./arctic/libc/spawn/spawnp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/spawnp.c

./o64/_arctic/libc/stdio/_io.o: \
     ./arctic/libc/stdio/_io.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/_io.c

./o64/_arctic/libc/stdio/clearerr.o: \
     ./arctic/libc/stdio/clearerr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/clearerr.c

./o64/_arctic/libc/stdio/fclose.o: \
     ./arctic/libc/stdio/fclose.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fclose.c

./o64/_arctic/libc/stdio/feof.o: \
     ./arctic/libc/stdio/feof.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/feof.c

./o64/_arctic/libc/stdio/ferror.o: \
     ./arctic/libc/stdio/ferror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/ferror.c

./o64/_arctic/libc/stdio/fflush.o: \
     ./arctic/libc/stdio/fflush.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fflush.c

./o64/_arctic/libc/stdio/fgetc.o: \
     ./arctic/libc/stdio/fgetc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fgetc.c

./o64/_arctic/libc/stdio/fputc.o: \
     ./arctic/libc/stdio/fputc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fputc.c

./o64/_arctic/libc/stdio/fread.o: \
     ./arctic/libc/stdio/fread.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fread.c

./o64/_arctic/libc/stdio/fwrite.o: \
     ./arctic/libc/stdio/fwrite.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fwrite.c

./o64/_arctic/libc/stdio/rename.o: \
     ./arctic/libc/stdio/rename.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/rename.c

./o64/_arctic/libc/stdio/snprintf.o: \
     ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/snprintf.c

./o64/_arctic/libc/stdlib/alloc.o: \
     ./arctic/libc/stdlib/alloc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/alloc.c

./o64/_arctic/libc/stdlib/qsort.o: \
     ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/qsort.c

./o64/_arctic/libc/string/strcat.o: \
     ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcat.c

./o64/_arctic/libc/string/strcmp.o: \
     ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcmp.c

./o64/_arctic/libc/string/strcpy.o: \
     ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcpy.c

./o64/_arctic/libc/string/strerror.o: \
     ./arctic/libc/string/strerror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strerror.c

./o64/_arctic/libc/string/strncmp.o: \
     ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncmp.c

./o64/_arctic/libc/string/strncpy.o: \
     ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncpy.c

./o64/_arctic/libc/sys/fstat.o: \
     ./arctic/libc/sys/fstat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/fstat.c

./o64/_arctic/libc/sys/lstat.o: \
     ./arctic/libc/sys/lstat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/lstat.c

./o64/_arctic/libc/sys/mkdir.o: \
     ./arctic/libc/sys/mkdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mkdir.c

./o64/_arctic/libc/sys/mmap.o: \
     ./arctic/libc/sys/mmap.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mmap.c

./o64/_arctic/libc/sys/mprotect.o: \
     ./arctic/libc/sys/mprotect.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mprotect.c

./o64/_arctic/libc/sys/msync.o: \
     ./arctic/libc/sys/msync.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/msync.c

./o64/_arctic/libc/sys/munmap.o: \
     ./arctic/libc/sys/munmap.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/munmap.c

./o64/_arctic/libc/sys/stat.o: \
     ./arctic/libc/sys/stat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/stat.c

./o64/_arctic/libc/sys/wait.o: \
     ./arctic/libc/sys/wait.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/wait.c

./o64/_arctic/libc/sys/waitpid.o: \
     ./arctic/libc/sys/waitpid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/waitpid.c

./o64/_arctic/libc/threads/mtx.o: \
     ./arctic/libc/threads/mtx.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/threads/mtx.c

./o64/_arctic/libc/time/gettime.o: \
     ./arctic/libc/time/gettime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/gettime.c

./o64/_arctic/libc/time/settime.o: \
     ./arctic/libc/time/settime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/settime.c

./o64/_arctic/libc/time/sleep.o: \
     ./arctic/libc/time/sleep.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/sleep.c

./o64/_arctic/libc/time/time.o: \
     ./arctic/libc/time/time.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/time.c

./o64/_arctic/libc/unistd/_exit.o: \
     ./arctic/libc/unistd/_exit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/_exit.c

./o64/_arctic/libc/unistd/chdir.o: \
     ./arctic/libc/unistd/chdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/chdir.c

./o64/_arctic/libc/unistd/close.o: \
     ./arctic/libc/unistd/close.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/close.c

./o64/_arctic/libc/unistd/dup.o: \
     ./arctic/libc/unistd/dup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/dup.c

./o64/_arctic/libc/unistd/dup2.o: \
     ./arctic/libc/unistd/dup2.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/dup2.c

./o64/_arctic/libc/unistd/execve.o: \
     ./arctic/libc/unistd/execve.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/execve.c

./o64/_arctic/libc/unistd/getcwd.o: \
     ./arctic/libc/unistd/getcwd.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getcwd.c

./o64/_arctic/libc/unistd/getpid.o: \
     ./arctic/libc/unistd/getpid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getpid.c

./o64/_arctic/libc/unistd/getppid.o: \
     ./arctic/libc/unistd/getppid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getppid.c

./o64/_arctic/libc/unistd/lseek.o: \
     ./arctic/libc/unistd/lseek.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/lseek.c

./o64/_arctic/libc/unistd/pipe.o: \
     ./arctic/libc/unistd/pipe.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/pipe.c

./o64/_arctic/libc/unistd/read.o: \
     ./arctic/libc/unistd/read.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/read.c

./o64/_arctic/libc/unistd/rmdir.o: \
     ./arctic/libc/unistd/rmdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/rmdir.c

./o64/_arctic/libc/unistd/unlink.o: \
     ./arctic/libc/unistd/unlink.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/unlink.c

./o64/_arctic/libc/unistd/write.o: \
     ./arctic/libc/unistd/write.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/write.c

./o64/_arctic/libc/start.o: \
     ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/start.c
