# Dancy Operating System

##############################################################################

ARCTIC_LIBC_OBJECTS_32= \
 ./o32/arctic/libc/a32/memcmp.o \
 ./o32/arctic/libc/a32/memcpy.o \
 ./o32/arctic/libc/a32/memmove.o \
 ./o32/arctic/libc/a32/memset.o \
 ./o32/arctic/libc/a32/pointer.o \
 ./o32/arctic/libc/a32/spin.o \
 ./o32/arctic/libc/a32/start.o \
 ./o32/arctic/libc/a32/strlen.o \
 ./o32/arctic/libc/a32/syscall0.o \
 ./o32/arctic/libc/a32/syscall1.o \
 ./o32/arctic/libc/a32/syscall2.o \
 ./o32/arctic/libc/a32/syscall3.o \
 ./o32/arctic/libc/a32/syscall4.o \
 ./o32/arctic/libc/a32/syscall5.o \
 ./o32/arctic/libc/ctype/isalnum.o \
 ./o32/arctic/libc/ctype/isalpha.o \
 ./o32/arctic/libc/ctype/isblank.o \
 ./o32/arctic/libc/ctype/iscntrl.o \
 ./o32/arctic/libc/ctype/isdigit.o \
 ./o32/arctic/libc/ctype/isgraph.o \
 ./o32/arctic/libc/ctype/islower.o \
 ./o32/arctic/libc/ctype/isprint.o \
 ./o32/arctic/libc/ctype/ispunct.o \
 ./o32/arctic/libc/ctype/isspace.o \
 ./o32/arctic/libc/ctype/isupper.o \
 ./o32/arctic/libc/ctype/isxdigit.o \
 ./o32/arctic/libc/ctype/tolower.o \
 ./o32/arctic/libc/ctype/toupper.o \
 ./o32/arctic/libc/dirent/dir.o \
 ./o32/arctic/libc/dirent/dirfd.o \
 ./o32/arctic/libc/dirent/rewind.o \
 ./o32/arctic/libc/errno/errno.o \
 ./o32/arctic/libc/fcntl/fcntl.o \
 ./o32/arctic/libc/fcntl/open.o \
 ./o32/arctic/libc/keymap/_keymap.o \
 ./o32/arctic/libc/keymap/_write.o \
 ./o32/arctic/libc/misc/__main.o \
 ./o32/arctic/libc/misc/ull.o \
 ./o32/arctic/libc/poll/poll.o \
 ./o32/arctic/libc/pty/openpty.o \
 ./o32/arctic/libc/signal/kill.o \
 ./o32/arctic/libc/signal/raise.o \
 ./o32/arctic/libc/spawn/actions.o \
 ./o32/arctic/libc/spawn/addclose.o \
 ./o32/arctic/libc/spawn/adddup2.o \
 ./o32/arctic/libc/spawn/addopen.o \
 ./o32/arctic/libc/spawn/attr.o \
 ./o32/arctic/libc/spawn/flags.o \
 ./o32/arctic/libc/spawn/pgroup.o \
 ./o32/arctic/libc/spawn/policy.o \
 ./o32/arctic/libc/spawn/sched.o \
 ./o32/arctic/libc/spawn/sigdef.o \
 ./o32/arctic/libc/spawn/sigmask.o \
 ./o32/arctic/libc/spawn/spawn.o \
 ./o32/arctic/libc/spawn/spawnp.o \
 ./o32/arctic/libc/start.o \
 ./o32/arctic/libc/stdio/_io.o \
 ./o32/arctic/libc/stdio/clearerr.o \
 ./o32/arctic/libc/stdio/fclose.o \
 ./o32/arctic/libc/stdio/feof.o \
 ./o32/arctic/libc/stdio/ferror.o \
 ./o32/arctic/libc/stdio/fflush.o \
 ./o32/arctic/libc/stdio/fgetc.o \
 ./o32/arctic/libc/stdio/fgetpos.o \
 ./o32/arctic/libc/stdio/fgets.o \
 ./o32/arctic/libc/stdio/fileno.o \
 ./o32/arctic/libc/stdio/fopen.o \
 ./o32/arctic/libc/stdio/fprintf.o \
 ./o32/arctic/libc/stdio/fputc.o \
 ./o32/arctic/libc/stdio/fputs.o \
 ./o32/arctic/libc/stdio/fread.o \
 ./o32/arctic/libc/stdio/freopen.o \
 ./o32/arctic/libc/stdio/fseek.o \
 ./o32/arctic/libc/stdio/fseeko.o \
 ./o32/arctic/libc/stdio/fsetpos.o \
 ./o32/arctic/libc/stdio/ftell.o \
 ./o32/arctic/libc/stdio/ftello.o \
 ./o32/arctic/libc/stdio/fwrite.o \
 ./o32/arctic/libc/stdio/getc.o \
 ./o32/arctic/libc/stdio/getchar.o \
 ./o32/arctic/libc/stdio/perror.o \
 ./o32/arctic/libc/stdio/printf.o \
 ./o32/arctic/libc/stdio/putc.o \
 ./o32/arctic/libc/stdio/putchar.o \
 ./o32/arctic/libc/stdio/puts.o \
 ./o32/arctic/libc/stdio/remove.o \
 ./o32/arctic/libc/stdio/rename.o \
 ./o32/arctic/libc/stdio/rewind.o \
 ./o32/arctic/libc/stdio/setbuf.o \
 ./o32/arctic/libc/stdio/setvbuf.o \
 ./o32/arctic/libc/stdio/snprintf.o \
 ./o32/arctic/libc/stdio/sprintf.o \
 ./o32/arctic/libc/stdio/ungetc.o \
 ./o32/arctic/libc/stdio/vfprintf.o \
 ./o32/arctic/libc/stdio/vsprintf.o \
 ./o32/arctic/libc/stdlib/abort.o \
 ./o32/arctic/libc/stdlib/abs.o \
 ./o32/arctic/libc/stdlib/alloc.o \
 ./o32/arctic/libc/stdlib/atexit.o \
 ./o32/arctic/libc/stdlib/atoi.o \
 ./o32/arctic/libc/stdlib/atol.o \
 ./o32/arctic/libc/stdlib/atoll.o \
 ./o32/arctic/libc/stdlib/exit.o \
 ./o32/arctic/libc/stdlib/getenv.o \
 ./o32/arctic/libc/stdlib/labs.o \
 ./o32/arctic/libc/stdlib/llabs.o \
 ./o32/arctic/libc/stdlib/qsort.o \
 ./o32/arctic/libc/stdlib/realpath.o \
 ./o32/arctic/libc/stdlib/strtol.o \
 ./o32/arctic/libc/stdlib/strtoll.o \
 ./o32/arctic/libc/stdlib/strtoul.o \
 ./o32/arctic/libc/stdlib/strtoull.o \
 ./o32/arctic/libc/string/casecmp.o \
 ./o32/arctic/libc/string/memchr.o \
 ./o32/arctic/libc/string/strcat.o \
 ./o32/arctic/libc/string/strchr.o \
 ./o32/arctic/libc/string/strcmp.o \
 ./o32/arctic/libc/string/strcpy.o \
 ./o32/arctic/libc/string/strcspn.o \
 ./o32/arctic/libc/string/strdup.o \
 ./o32/arctic/libc/string/strerror.o \
 ./o32/arctic/libc/string/strncmp.o \
 ./o32/arctic/libc/string/strncpy.o \
 ./o32/arctic/libc/string/strndup.o \
 ./o32/arctic/libc/string/strpbrk.o \
 ./o32/arctic/libc/string/strrchr.o \
 ./o32/arctic/libc/string/strspn.o \
 ./o32/arctic/libc/string/strstr.o \
 ./o32/arctic/libc/string/strtok.o \
 ./o32/arctic/libc/sys/chmod.o \
 ./o32/arctic/libc/sys/fstat.o \
 ./o32/arctic/libc/sys/ioctl.o \
 ./o32/arctic/libc/sys/lstat.o \
 ./o32/arctic/libc/sys/mkdir.o \
 ./o32/arctic/libc/sys/mmap.o \
 ./o32/arctic/libc/sys/mprotect.o \
 ./o32/arctic/libc/sys/msync.o \
 ./o32/arctic/libc/sys/munmap.o \
 ./o32/arctic/libc/sys/stat.o \
 ./o32/arctic/libc/sys/umask.o \
 ./o32/arctic/libc/sys/wait.o \
 ./o32/arctic/libc/sys/waitpid.o \
 ./o32/arctic/libc/termios/getattr.o \
 ./o32/arctic/libc/termios/setattr.o \
 ./o32/arctic/libc/threads/mtx.o \
 ./o32/arctic/libc/time/gettime.o \
 ./o32/arctic/libc/time/settime.o \
 ./o32/arctic/libc/time/sleep.o \
 ./o32/arctic/libc/time/time.o \
 ./o32/arctic/libc/unistd/_exit.o \
 ./o32/arctic/libc/unistd/chdir.o \
 ./o32/arctic/libc/unistd/close.o \
 ./o32/arctic/libc/unistd/dup.o \
 ./o32/arctic/libc/unistd/dup2.o \
 ./o32/arctic/libc/unistd/execve.o \
 ./o32/arctic/libc/unistd/getcwd.o \
 ./o32/arctic/libc/unistd/getpgid.o \
 ./o32/arctic/libc/unistd/getpgrp.o \
 ./o32/arctic/libc/unistd/getpid.o \
 ./o32/arctic/libc/unistd/getppid.o \
 ./o32/arctic/libc/unistd/getsid.o \
 ./o32/arctic/libc/unistd/isatty.o \
 ./o32/arctic/libc/unistd/lseek.o \
 ./o32/arctic/libc/unistd/pipe.o \
 ./o32/arctic/libc/unistd/read.o \
 ./o32/arctic/libc/unistd/rmdir.o \
 ./o32/arctic/libc/unistd/unlink.o \
 ./o32/arctic/libc/unistd/write.o \
 ./o32/common/a32/dark.o \

ARCTIC_LIBC_OBJECTS_64= \
 ./o64/arctic/libc/a64/memcmp.o \
 ./o64/arctic/libc/a64/memcpy.o \
 ./o64/arctic/libc/a64/memmove.o \
 ./o64/arctic/libc/a64/memset.o \
 ./o64/arctic/libc/a64/pointer.o \
 ./o64/arctic/libc/a64/spin.o \
 ./o64/arctic/libc/a64/start.o \
 ./o64/arctic/libc/a64/strlen.o \
 ./o64/arctic/libc/a64/syscall0.o \
 ./o64/arctic/libc/a64/syscall1.o \
 ./o64/arctic/libc/a64/syscall2.o \
 ./o64/arctic/libc/a64/syscall3.o \
 ./o64/arctic/libc/a64/syscall4.o \
 ./o64/arctic/libc/a64/syscall5.o \
 ./o64/arctic/libc/ctype/isalnum.o \
 ./o64/arctic/libc/ctype/isalpha.o \
 ./o64/arctic/libc/ctype/isblank.o \
 ./o64/arctic/libc/ctype/iscntrl.o \
 ./o64/arctic/libc/ctype/isdigit.o \
 ./o64/arctic/libc/ctype/isgraph.o \
 ./o64/arctic/libc/ctype/islower.o \
 ./o64/arctic/libc/ctype/isprint.o \
 ./o64/arctic/libc/ctype/ispunct.o \
 ./o64/arctic/libc/ctype/isspace.o \
 ./o64/arctic/libc/ctype/isupper.o \
 ./o64/arctic/libc/ctype/isxdigit.o \
 ./o64/arctic/libc/ctype/tolower.o \
 ./o64/arctic/libc/ctype/toupper.o \
 ./o64/arctic/libc/dirent/dir.o \
 ./o64/arctic/libc/dirent/dirfd.o \
 ./o64/arctic/libc/dirent/rewind.o \
 ./o64/arctic/libc/errno/errno.o \
 ./o64/arctic/libc/fcntl/fcntl.o \
 ./o64/arctic/libc/fcntl/open.o \
 ./o64/arctic/libc/keymap/_keymap.o \
 ./o64/arctic/libc/keymap/_write.o \
 ./o64/arctic/libc/misc/__main.o \
 ./o64/arctic/libc/misc/ull.o \
 ./o64/arctic/libc/poll/poll.o \
 ./o64/arctic/libc/pty/openpty.o \
 ./o64/arctic/libc/signal/kill.o \
 ./o64/arctic/libc/signal/raise.o \
 ./o64/arctic/libc/spawn/actions.o \
 ./o64/arctic/libc/spawn/addclose.o \
 ./o64/arctic/libc/spawn/adddup2.o \
 ./o64/arctic/libc/spawn/addopen.o \
 ./o64/arctic/libc/spawn/attr.o \
 ./o64/arctic/libc/spawn/flags.o \
 ./o64/arctic/libc/spawn/pgroup.o \
 ./o64/arctic/libc/spawn/policy.o \
 ./o64/arctic/libc/spawn/sched.o \
 ./o64/arctic/libc/spawn/sigdef.o \
 ./o64/arctic/libc/spawn/sigmask.o \
 ./o64/arctic/libc/spawn/spawn.o \
 ./o64/arctic/libc/spawn/spawnp.o \
 ./o64/arctic/libc/start.o \
 ./o64/arctic/libc/stdio/_io.o \
 ./o64/arctic/libc/stdio/clearerr.o \
 ./o64/arctic/libc/stdio/fclose.o \
 ./o64/arctic/libc/stdio/feof.o \
 ./o64/arctic/libc/stdio/ferror.o \
 ./o64/arctic/libc/stdio/fflush.o \
 ./o64/arctic/libc/stdio/fgetc.o \
 ./o64/arctic/libc/stdio/fgetpos.o \
 ./o64/arctic/libc/stdio/fgets.o \
 ./o64/arctic/libc/stdio/fileno.o \
 ./o64/arctic/libc/stdio/fopen.o \
 ./o64/arctic/libc/stdio/fprintf.o \
 ./o64/arctic/libc/stdio/fputc.o \
 ./o64/arctic/libc/stdio/fputs.o \
 ./o64/arctic/libc/stdio/fread.o \
 ./o64/arctic/libc/stdio/freopen.o \
 ./o64/arctic/libc/stdio/fseek.o \
 ./o64/arctic/libc/stdio/fseeko.o \
 ./o64/arctic/libc/stdio/fsetpos.o \
 ./o64/arctic/libc/stdio/ftell.o \
 ./o64/arctic/libc/stdio/ftello.o \
 ./o64/arctic/libc/stdio/fwrite.o \
 ./o64/arctic/libc/stdio/getc.o \
 ./o64/arctic/libc/stdio/getchar.o \
 ./o64/arctic/libc/stdio/perror.o \
 ./o64/arctic/libc/stdio/printf.o \
 ./o64/arctic/libc/stdio/putc.o \
 ./o64/arctic/libc/stdio/putchar.o \
 ./o64/arctic/libc/stdio/puts.o \
 ./o64/arctic/libc/stdio/remove.o \
 ./o64/arctic/libc/stdio/rename.o \
 ./o64/arctic/libc/stdio/rewind.o \
 ./o64/arctic/libc/stdio/setbuf.o \
 ./o64/arctic/libc/stdio/setvbuf.o \
 ./o64/arctic/libc/stdio/snprintf.o \
 ./o64/arctic/libc/stdio/sprintf.o \
 ./o64/arctic/libc/stdio/ungetc.o \
 ./o64/arctic/libc/stdio/vfprintf.o \
 ./o64/arctic/libc/stdio/vsprintf.o \
 ./o64/arctic/libc/stdlib/abort.o \
 ./o64/arctic/libc/stdlib/abs.o \
 ./o64/arctic/libc/stdlib/alloc.o \
 ./o64/arctic/libc/stdlib/atexit.o \
 ./o64/arctic/libc/stdlib/atoi.o \
 ./o64/arctic/libc/stdlib/atol.o \
 ./o64/arctic/libc/stdlib/atoll.o \
 ./o64/arctic/libc/stdlib/exit.o \
 ./o64/arctic/libc/stdlib/getenv.o \
 ./o64/arctic/libc/stdlib/labs.o \
 ./o64/arctic/libc/stdlib/llabs.o \
 ./o64/arctic/libc/stdlib/qsort.o \
 ./o64/arctic/libc/stdlib/realpath.o \
 ./o64/arctic/libc/stdlib/strtol.o \
 ./o64/arctic/libc/stdlib/strtoll.o \
 ./o64/arctic/libc/stdlib/strtoul.o \
 ./o64/arctic/libc/stdlib/strtoull.o \
 ./o64/arctic/libc/string/casecmp.o \
 ./o64/arctic/libc/string/memchr.o \
 ./o64/arctic/libc/string/strcat.o \
 ./o64/arctic/libc/string/strchr.o \
 ./o64/arctic/libc/string/strcmp.o \
 ./o64/arctic/libc/string/strcpy.o \
 ./o64/arctic/libc/string/strcspn.o \
 ./o64/arctic/libc/string/strdup.o \
 ./o64/arctic/libc/string/strerror.o \
 ./o64/arctic/libc/string/strncmp.o \
 ./o64/arctic/libc/string/strncpy.o \
 ./o64/arctic/libc/string/strndup.o \
 ./o64/arctic/libc/string/strpbrk.o \
 ./o64/arctic/libc/string/strrchr.o \
 ./o64/arctic/libc/string/strspn.o \
 ./o64/arctic/libc/string/strstr.o \
 ./o64/arctic/libc/string/strtok.o \
 ./o64/arctic/libc/sys/chmod.o \
 ./o64/arctic/libc/sys/fstat.o \
 ./o64/arctic/libc/sys/ioctl.o \
 ./o64/arctic/libc/sys/lstat.o \
 ./o64/arctic/libc/sys/mkdir.o \
 ./o64/arctic/libc/sys/mmap.o \
 ./o64/arctic/libc/sys/mprotect.o \
 ./o64/arctic/libc/sys/msync.o \
 ./o64/arctic/libc/sys/munmap.o \
 ./o64/arctic/libc/sys/stat.o \
 ./o64/arctic/libc/sys/umask.o \
 ./o64/arctic/libc/sys/wait.o \
 ./o64/arctic/libc/sys/waitpid.o \
 ./o64/arctic/libc/termios/getattr.o \
 ./o64/arctic/libc/termios/setattr.o \
 ./o64/arctic/libc/threads/mtx.o \
 ./o64/arctic/libc/time/gettime.o \
 ./o64/arctic/libc/time/settime.o \
 ./o64/arctic/libc/time/sleep.o \
 ./o64/arctic/libc/time/time.o \
 ./o64/arctic/libc/unistd/_exit.o \
 ./o64/arctic/libc/unistd/chdir.o \
 ./o64/arctic/libc/unistd/close.o \
 ./o64/arctic/libc/unistd/dup.o \
 ./o64/arctic/libc/unistd/dup2.o \
 ./o64/arctic/libc/unistd/execve.o \
 ./o64/arctic/libc/unistd/getcwd.o \
 ./o64/arctic/libc/unistd/getpgid.o \
 ./o64/arctic/libc/unistd/getpgrp.o \
 ./o64/arctic/libc/unistd/getpid.o \
 ./o64/arctic/libc/unistd/getppid.o \
 ./o64/arctic/libc/unistd/getsid.o \
 ./o64/arctic/libc/unistd/isatty.o \
 ./o64/arctic/libc/unistd/lseek.o \
 ./o64/arctic/libc/unistd/pipe.o \
 ./o64/arctic/libc/unistd/read.o \
 ./o64/arctic/libc/unistd/rmdir.o \
 ./o64/arctic/libc/unistd/unlink.o \
 ./o64/arctic/libc/unistd/write.o \
 ./o64/common/a64/dark.o \

##############################################################################

./o32/arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_32)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_32)

./o64/arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_64)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_64)
