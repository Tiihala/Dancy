# Dancy Operating System

##############################################################################

./o32/arctic/apps/editor/__dancy.o: \
    ./arctic/apps/editor/__dancy.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_EDITOR_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/apps/editor/__dancy.c

./o32/arctic/apps/hello/main.o: \
    ./arctic/apps/hello/main.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_HELLO_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/apps/hello/main.c

./o32/arctic/apps/hello/operate.o: \
    ./arctic/apps/hello/operate.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_HELLO_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/apps/hello/operate.c

./o32/arctic/libc/a32/memcmp.o: \
    ./arctic/libc/a32/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcmp.asm

./o32/arctic/libc/a32/memcpy.o: \
    ./arctic/libc/a32/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcpy.asm

./o32/arctic/libc/a32/memmove.o: \
    ./arctic/libc/a32/memmove.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memmove.asm

./o32/arctic/libc/a32/memset.o: \
    ./arctic/libc/a32/memset.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memset.asm

./o32/arctic/libc/a32/pointer.o: \
    ./arctic/libc/a32/pointer.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/pointer.asm

./o32/arctic/libc/a32/spin.o: \
    ./arctic/libc/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/spin.asm

./o32/arctic/libc/a32/start.o: \
    ./arctic/libc/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/start.asm

./o32/arctic/libc/a32/strlen.o: \
    ./arctic/libc/a32/strlen.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/strlen.asm

./o32/arctic/libc/a32/syscall0.o: \
    ./arctic/libc/a32/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall0.asm

./o32/arctic/libc/a32/syscall1.o: \
    ./arctic/libc/a32/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall1.asm

./o32/arctic/libc/a32/syscall2.o: \
    ./arctic/libc/a32/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall2.asm

./o32/arctic/libc/a32/syscall3.o: \
    ./arctic/libc/a32/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall3.asm

./o32/arctic/libc/a32/syscall4.o: \
    ./arctic/libc/a32/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall4.asm

./o32/arctic/libc/a32/syscall5.o: \
    ./arctic/libc/a32/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall5.asm

./o32/arctic/libc/ctype/isalnum.o: \
    ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalnum.c

./o32/arctic/libc/ctype/isalpha.o: \
    ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalpha.c

./o32/arctic/libc/ctype/isblank.o: \
    ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isblank.c

./o32/arctic/libc/ctype/iscntrl.o: \
    ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/iscntrl.c

./o32/arctic/libc/ctype/isdigit.o: \
    ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isdigit.c

./o32/arctic/libc/ctype/isgraph.o: \
    ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isgraph.c

./o32/arctic/libc/ctype/islower.o: \
    ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/islower.c

./o32/arctic/libc/ctype/isprint.o: \
    ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isprint.c

./o32/arctic/libc/ctype/ispunct.o: \
    ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/ispunct.c

./o32/arctic/libc/ctype/isspace.o: \
    ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isspace.c

./o32/arctic/libc/ctype/isupper.o: \
    ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isupper.c

./o32/arctic/libc/ctype/isxdigit.o: \
    ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isxdigit.c

./o32/arctic/libc/ctype/tolower.o: \
    ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/tolower.c

./o32/arctic/libc/ctype/toupper.o: \
    ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/toupper.c

./o32/arctic/libc/dirent/dir.o: \
    ./arctic/libc/dirent/dir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/dir.c

./o32/arctic/libc/dirent/dirfd.o: \
    ./arctic/libc/dirent/dirfd.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/dirfd.c

./o32/arctic/libc/dirent/rewind.o: \
    ./arctic/libc/dirent/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/dirent/rewind.c

./o32/arctic/libc/err/err.o: \
    ./arctic/libc/err/err.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/err/err.c

./o32/arctic/libc/err/errx.o: \
    ./arctic/libc/err/errx.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/err/errx.c

./o32/arctic/libc/err/warn.o: \
    ./arctic/libc/err/warn.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/err/warn.c

./o32/arctic/libc/err/warnx.o: \
    ./arctic/libc/err/warnx.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/err/warnx.c

./o32/arctic/libc/errno/errno.o: \
    ./arctic/libc/errno/errno.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/errno/errno.c

./o32/arctic/libc/fcntl/fcntl.o: \
    ./arctic/libc/fcntl/fcntl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/fcntl/fcntl.c

./o32/arctic/libc/fcntl/open.o: \
    ./arctic/libc/fcntl/open.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/fcntl/open.c

./o32/arctic/libc/glob/glob.o: \
    ./arctic/libc/glob/glob.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/glob/glob.c

./o32/arctic/libc/glob/globfree.o: \
    ./arctic/libc/glob/globfree.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/glob/globfree.c

./o32/arctic/libc/keymap/_keymap.o: \
    ./arctic/libc/keymap/_keymap.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/_keymap.c

./o32/arctic/libc/keymap/_write.o: \
    ./arctic/libc/keymap/_write.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/_write.c

./o32/arctic/libc/keymap/de.o: \
    ./arctic/libc/keymap/de.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/de.c

./o32/arctic/libc/keymap/dk.o: \
    ./arctic/libc/keymap/dk.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/dk.c

./o32/arctic/libc/keymap/et.o: \
    ./arctic/libc/keymap/et.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/et.c

./o32/arctic/libc/keymap/fi.o: \
    ./arctic/libc/keymap/fi.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/fi.c

./o32/arctic/libc/keymap/no.o: \
    ./arctic/libc/keymap/no.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/no.c

./o32/arctic/libc/keymap/sv.o: \
    ./arctic/libc/keymap/sv.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/sv.c

./o32/arctic/libc/keymap/uk.o: \
    ./arctic/libc/keymap/uk.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/uk.c

./o32/arctic/libc/keymap/us.o: \
    ./arctic/libc/keymap/us.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/keymap/us.c

./o32/arctic/libc/locale/conv.o: \
    ./arctic/libc/locale/conv.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/locale/conv.c

./o32/arctic/libc/locale/set.o: \
    ./arctic/libc/locale/set.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/locale/set.c

./o32/arctic/libc/misc/__assert.o: \
    ./arctic/libc/misc/__assert.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/__assert.c

./o32/arctic/libc/misc/__main.o: \
    ./arctic/libc/misc/__main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/__main.c

./o32/arctic/libc/misc/memusage.o: \
    ./arctic/libc/misc/memusage.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/memusage.c

./o32/arctic/libc/misc/procinfo.o: \
    ./arctic/libc/misc/procinfo.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/procinfo.c

./o32/arctic/libc/misc/proclist.o: \
    ./arctic/libc/misc/proclist.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/misc/proclist.c

./o32/arctic/libc/poll/poll.o: \
    ./arctic/libc/poll/poll.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/poll/poll.c

./o32/arctic/libc/poll/ppoll.o: \
    ./arctic/libc/poll/ppoll.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/poll/ppoll.c

./o32/arctic/libc/pty/openpty.o: \
    ./arctic/libc/pty/openpty.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/pty/openpty.c

./o32/arctic/libc/regex/regcomp.o: \
    ./arctic/libc/regex/regcomp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/regex/regcomp.c

./o32/arctic/libc/regex/regerror.o: \
    ./arctic/libc/regex/regerror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/regex/regerror.c

./o32/arctic/libc/regex/regexec.o: \
    ./arctic/libc/regex/regexec.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/regex/regexec.c

./o32/arctic/libc/regex/regfree.o: \
    ./arctic/libc/regex/regfree.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/regex/regfree.c

./o32/arctic/libc/signal/action.o: \
    ./arctic/libc/signal/action.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/action.c

./o32/arctic/libc/signal/kill.o: \
    ./arctic/libc/signal/kill.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/kill.c

./o32/arctic/libc/signal/pending.o: \
    ./arctic/libc/signal/pending.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/pending.c

./o32/arctic/libc/signal/procmask.o: \
    ./arctic/libc/signal/procmask.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/procmask.c

./o32/arctic/libc/signal/raise.o: \
    ./arctic/libc/signal/raise.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/raise.c

./o32/arctic/libc/signal/signal.o: \
    ./arctic/libc/signal/signal.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/signal.c

./o32/arctic/libc/signal/sigset_t.o: \
    ./arctic/libc/signal/sigset_t.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/signal/sigset_t.c

./o32/arctic/libc/spawn/actions.o: \
    ./arctic/libc/spawn/actions.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/actions.c

./o32/arctic/libc/spawn/addclose.o: \
    ./arctic/libc/spawn/addclose.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/addclose.c

./o32/arctic/libc/spawn/adddup2.o: \
    ./arctic/libc/spawn/adddup2.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/adddup2.c

./o32/arctic/libc/spawn/addopen.o: \
    ./arctic/libc/spawn/addopen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/addopen.c

./o32/arctic/libc/spawn/attr.o: \
    ./arctic/libc/spawn/attr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/attr.c

./o32/arctic/libc/spawn/flags.o: \
    ./arctic/libc/spawn/flags.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/flags.c

./o32/arctic/libc/spawn/pgroup.o: \
    ./arctic/libc/spawn/pgroup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/pgroup.c

./o32/arctic/libc/spawn/policy.o: \
    ./arctic/libc/spawn/policy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/policy.c

./o32/arctic/libc/spawn/sched.o: \
    ./arctic/libc/spawn/sched.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sched.c

./o32/arctic/libc/spawn/sigdef.o: \
    ./arctic/libc/spawn/sigdef.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sigdef.c

./o32/arctic/libc/spawn/sigmask.o: \
    ./arctic/libc/spawn/sigmask.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/sigmask.c

./o32/arctic/libc/spawn/spawn.o: \
    ./arctic/libc/spawn/spawn.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/spawn.c

./o32/arctic/libc/spawn/spawnp.o: \
    ./arctic/libc/spawn/spawnp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/spawnp.c

./o32/arctic/libc/spawn/tcset.o: \
    ./arctic/libc/spawn/tcset.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/spawn/tcset.c

./o32/arctic/libc/start.o: \
    ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/start.c

./o32/arctic/libc/stdio/_io.o: \
    ./arctic/libc/stdio/_io.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/_io.c

./o32/arctic/libc/stdio/_scan.o: \
    ./arctic/libc/stdio/_scan.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/_scan.c

./o32/arctic/libc/stdio/asprintf.o: \
    ./arctic/libc/stdio/asprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/asprintf.c

./o32/arctic/libc/stdio/clearerr.o: \
    ./arctic/libc/stdio/clearerr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/clearerr.c

./o32/arctic/libc/stdio/fclose.o: \
    ./arctic/libc/stdio/fclose.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fclose.c

./o32/arctic/libc/stdio/feof.o: \
    ./arctic/libc/stdio/feof.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/feof.c

./o32/arctic/libc/stdio/ferror.o: \
    ./arctic/libc/stdio/ferror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/ferror.c

./o32/arctic/libc/stdio/fflush.o: \
    ./arctic/libc/stdio/fflush.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fflush.c

./o32/arctic/libc/stdio/fgetc.o: \
    ./arctic/libc/stdio/fgetc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fgetc.c

./o32/arctic/libc/stdio/fgetpos.o: \
    ./arctic/libc/stdio/fgetpos.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fgetpos.c

./o32/arctic/libc/stdio/fgets.o: \
    ./arctic/libc/stdio/fgets.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fgets.c

./o32/arctic/libc/stdio/fileno.o: \
    ./arctic/libc/stdio/fileno.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fileno.c

./o32/arctic/libc/stdio/fopen.o: \
    ./arctic/libc/stdio/fopen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fopen.c

./o32/arctic/libc/stdio/fprintf.o: \
    ./arctic/libc/stdio/fprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fprintf.c

./o32/arctic/libc/stdio/fputc.o: \
    ./arctic/libc/stdio/fputc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fputc.c

./o32/arctic/libc/stdio/fputs.o: \
    ./arctic/libc/stdio/fputs.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fputs.c

./o32/arctic/libc/stdio/fread.o: \
    ./arctic/libc/stdio/fread.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fread.c

./o32/arctic/libc/stdio/freopen.o: \
    ./arctic/libc/stdio/freopen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/freopen.c

./o32/arctic/libc/stdio/fscanf.o: \
    ./arctic/libc/stdio/fscanf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fscanf.c

./o32/arctic/libc/stdio/fseek.o: \
    ./arctic/libc/stdio/fseek.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fseek.c

./o32/arctic/libc/stdio/fseeko.o: \
    ./arctic/libc/stdio/fseeko.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fseeko.c

./o32/arctic/libc/stdio/fsetpos.o: \
    ./arctic/libc/stdio/fsetpos.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fsetpos.c

./o32/arctic/libc/stdio/ftell.o: \
    ./arctic/libc/stdio/ftell.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/ftell.c

./o32/arctic/libc/stdio/ftello.o: \
    ./arctic/libc/stdio/ftello.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/ftello.c

./o32/arctic/libc/stdio/fwrite.o: \
    ./arctic/libc/stdio/fwrite.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/fwrite.c

./o32/arctic/libc/stdio/getc.o: \
    ./arctic/libc/stdio/getc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/getc.c

./o32/arctic/libc/stdio/getchar.o: \
    ./arctic/libc/stdio/getchar.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/getchar.c

./o32/arctic/libc/stdio/getdelim.o: \
    ./arctic/libc/stdio/getdelim.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/getdelim.c

./o32/arctic/libc/stdio/getline.o: \
    ./arctic/libc/stdio/getline.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/getline.c

./o32/arctic/libc/stdio/pclose.o: \
    ./arctic/libc/stdio/pclose.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/pclose.c

./o32/arctic/libc/stdio/perror.o: \
    ./arctic/libc/stdio/perror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/perror.c

./o32/arctic/libc/stdio/popen.o: \
    ./arctic/libc/stdio/popen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/popen.c

./o32/arctic/libc/stdio/printf.o: \
    ./arctic/libc/stdio/printf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/printf.c

./o32/arctic/libc/stdio/putc.o: \
    ./arctic/libc/stdio/putc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/putc.c

./o32/arctic/libc/stdio/putchar.o: \
    ./arctic/libc/stdio/putchar.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/putchar.c

./o32/arctic/libc/stdio/puts.o: \
    ./arctic/libc/stdio/puts.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/puts.c

./o32/arctic/libc/stdio/remove.o: \
    ./arctic/libc/stdio/remove.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/remove.c

./o32/arctic/libc/stdio/rename.o: \
    ./arctic/libc/stdio/rename.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/rename.c

./o32/arctic/libc/stdio/rewind.o: \
    ./arctic/libc/stdio/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/rewind.c

./o32/arctic/libc/stdio/scanf.o: \
    ./arctic/libc/stdio/scanf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/scanf.c

./o32/arctic/libc/stdio/setbuf.o: \
    ./arctic/libc/stdio/setbuf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/setbuf.c

./o32/arctic/libc/stdio/setvbuf.o: \
    ./arctic/libc/stdio/setvbuf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/setvbuf.c

./o32/arctic/libc/stdio/snprintf.o: \
    ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/snprintf.c

./o32/arctic/libc/stdio/sprintf.o: \
    ./arctic/libc/stdio/sprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/sprintf.c

./o32/arctic/libc/stdio/sscanf.o: \
    ./arctic/libc/stdio/sscanf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/sscanf.c

./o32/arctic/libc/stdio/ungetc.o: \
    ./arctic/libc/stdio/ungetc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/ungetc.c

./o32/arctic/libc/stdio/vfprintf.o: \
    ./arctic/libc/stdio/vfprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/vfprintf.c

./o32/arctic/libc/stdio/vsprintf.o: \
    ./arctic/libc/stdio/vsprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/vsprintf.c

./o32/arctic/libc/stdlib/_ull.o: \
    ./arctic/libc/stdlib/_ull.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/_ull.c

./o32/arctic/libc/stdlib/abort.o: \
    ./arctic/libc/stdlib/abort.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/abort.c

./o32/arctic/libc/stdlib/abs.o: \
    ./arctic/libc/stdlib/abs.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/abs.c

./o32/arctic/libc/stdlib/alloc.o: \
    ./arctic/libc/stdlib/alloc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/alloc.c

./o32/arctic/libc/stdlib/atexit.o: \
    ./arctic/libc/stdlib/atexit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/atexit.c

./o32/arctic/libc/stdlib/atoi.o: \
    ./arctic/libc/stdlib/atoi.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/atoi.c

./o32/arctic/libc/stdlib/atol.o: \
    ./arctic/libc/stdlib/atol.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/atol.c

./o32/arctic/libc/stdlib/atoll.o: \
    ./arctic/libc/stdlib/atoll.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/atoll.c

./o32/arctic/libc/stdlib/exit.o: \
    ./arctic/libc/stdlib/exit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/exit.c

./o32/arctic/libc/stdlib/getenv.o: \
    ./arctic/libc/stdlib/getenv.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/getenv.c

./o32/arctic/libc/stdlib/labs.o: \
    ./arctic/libc/stdlib/labs.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/labs.c

./o32/arctic/libc/stdlib/llabs.o: \
    ./arctic/libc/stdlib/llabs.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/llabs.c

./o32/arctic/libc/stdlib/qsort.o: \
    ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/qsort.c

./o32/arctic/libc/stdlib/realpath.o: \
    ./arctic/libc/stdlib/realpath.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/realpath.c

./o32/arctic/libc/stdlib/strtol.o: \
    ./arctic/libc/stdlib/strtol.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/strtol.c

./o32/arctic/libc/stdlib/strtoll.o: \
    ./arctic/libc/stdlib/strtoll.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/strtoll.c

./o32/arctic/libc/stdlib/strtoul.o: \
    ./arctic/libc/stdlib/strtoul.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/strtoul.c

./o32/arctic/libc/stdlib/strtoull.o: \
    ./arctic/libc/stdlib/strtoull.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/strtoull.c

./o32/arctic/libc/stdlib/system.o: \
    ./arctic/libc/stdlib/system.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/system.c

./o32/arctic/libc/string/casecmp.o: \
    ./arctic/libc/string/casecmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/casecmp.c

./o32/arctic/libc/string/memchr.o: \
    ./arctic/libc/string/memchr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/memchr.c

./o32/arctic/libc/string/strcat.o: \
    ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcat.c

./o32/arctic/libc/string/strchr.o: \
    ./arctic/libc/string/strchr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strchr.c

./o32/arctic/libc/string/strcmp.o: \
    ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcmp.c

./o32/arctic/libc/string/strcpy.o: \
    ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcpy.c

./o32/arctic/libc/string/strcspn.o: \
    ./arctic/libc/string/strcspn.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcspn.c

./o32/arctic/libc/string/strdup.o: \
    ./arctic/libc/string/strdup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strdup.c

./o32/arctic/libc/string/strerror.o: \
    ./arctic/libc/string/strerror.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strerror.c

./o32/arctic/libc/string/strncmp.o: \
    ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncmp.c

./o32/arctic/libc/string/strncpy.o: \
    ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncpy.c

./o32/arctic/libc/string/strndup.o: \
    ./arctic/libc/string/strndup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strndup.c

./o32/arctic/libc/string/strpbrk.o: \
    ./arctic/libc/string/strpbrk.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strpbrk.c

./o32/arctic/libc/string/strrchr.o: \
    ./arctic/libc/string/strrchr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strrchr.c

./o32/arctic/libc/string/strspn.o: \
    ./arctic/libc/string/strspn.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strspn.c

./o32/arctic/libc/string/strstr.o: \
    ./arctic/libc/string/strstr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strstr.c

./o32/arctic/libc/string/strtok.o: \
    ./arctic/libc/string/strtok.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strtok.c

./o32/arctic/libc/sys/chmod.o: \
    ./arctic/libc/sys/chmod.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/chmod.c

./o32/arctic/libc/sys/fdset.o: \
    ./arctic/libc/sys/fdset.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/fdset.c

./o32/arctic/libc/sys/fstat.o: \
    ./arctic/libc/sys/fstat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/fstat.c

./o32/arctic/libc/sys/ioctl.o: \
    ./arctic/libc/sys/ioctl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/ioctl.c

./o32/arctic/libc/sys/lstat.o: \
    ./arctic/libc/sys/lstat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/lstat.c

./o32/arctic/libc/sys/mkdir.o: \
    ./arctic/libc/sys/mkdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mkdir.c

./o32/arctic/libc/sys/mmap.o: \
    ./arctic/libc/sys/mmap.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mmap.c

./o32/arctic/libc/sys/mprotect.o: \
    ./arctic/libc/sys/mprotect.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/mprotect.c

./o32/arctic/libc/sys/msync.o: \
    ./arctic/libc/sys/msync.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/msync.c

./o32/arctic/libc/sys/munmap.o: \
    ./arctic/libc/sys/munmap.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/munmap.c

./o32/arctic/libc/sys/select.o: \
    ./arctic/libc/sys/select.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/select.c

./o32/arctic/libc/sys/stat.o: \
    ./arctic/libc/sys/stat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/stat.c

./o32/arctic/libc/sys/umask.o: \
    ./arctic/libc/sys/umask.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/umask.c

./o32/arctic/libc/sys/wait.o: \
    ./arctic/libc/sys/wait.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/wait.c

./o32/arctic/libc/sys/waitpid.o: \
    ./arctic/libc/sys/waitpid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/sys/waitpid.c

./o32/arctic/libc/termios/getattr.o: \
    ./arctic/libc/termios/getattr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/termios/getattr.c

./o32/arctic/libc/termios/getspeed.o: \
    ./arctic/libc/termios/getspeed.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/termios/getspeed.c

./o32/arctic/libc/termios/setattr.o: \
    ./arctic/libc/termios/setattr.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/termios/setattr.c

./o32/arctic/libc/termios/setspeed.o: \
    ./arctic/libc/termios/setspeed.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/termios/setspeed.c

./o32/arctic/libc/threads/mtx.o: \
    ./arctic/libc/threads/mtx.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/threads/mtx.c

./o32/arctic/libc/time/_time.o: \
    ./arctic/libc/time/_time.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/_time.c

./o32/arctic/libc/time/asctime.o: \
    ./arctic/libc/time/asctime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/asctime.c

./o32/arctic/libc/time/clock.o: \
    ./arctic/libc/time/clock.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/clock.c

./o32/arctic/libc/time/ctime.o: \
    ./arctic/libc/time/ctime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/ctime.c

./o32/arctic/libc/time/difftime.o: \
    ./arctic/libc/time/difftime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/difftime.c

./o32/arctic/libc/time/gettime.o: \
    ./arctic/libc/time/gettime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/gettime.c

./o32/arctic/libc/time/gmtime.o: \
    ./arctic/libc/time/gmtime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/gmtime.c

./o32/arctic/libc/time/local.o: \
    ./arctic/libc/time/local.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/local.c

./o32/arctic/libc/time/mktime.o: \
    ./arctic/libc/time/mktime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/mktime.c

./o32/arctic/libc/time/settime.o: \
    ./arctic/libc/time/settime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/settime.c

./o32/arctic/libc/time/sleep.o: \
    ./arctic/libc/time/sleep.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/sleep.c

./o32/arctic/libc/time/strftime.o: \
    ./arctic/libc/time/strftime.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/strftime.c

./o32/arctic/libc/time/time.o: \
    ./arctic/libc/time/time.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/time/time.c

./o32/arctic/libc/unistd/_exit.o: \
    ./arctic/libc/unistd/_exit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/_exit.c

./o32/arctic/libc/unistd/chdir.o: \
    ./arctic/libc/unistd/chdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/chdir.c

./o32/arctic/libc/unistd/close.o: \
    ./arctic/libc/unistd/close.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/close.c

./o32/arctic/libc/unistd/dup.o: \
    ./arctic/libc/unistd/dup.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/dup.c

./o32/arctic/libc/unistd/dup2.o: \
    ./arctic/libc/unistd/dup2.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/dup2.c

./o32/arctic/libc/unistd/execve.o: \
    ./arctic/libc/unistd/execve.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/execve.c

./o32/arctic/libc/unistd/getcwd.o: \
    ./arctic/libc/unistd/getcwd.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getcwd.c

./o32/arctic/libc/unistd/getpgid.o: \
    ./arctic/libc/unistd/getpgid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getpgid.c

./o32/arctic/libc/unistd/getpgrp.o: \
    ./arctic/libc/unistd/getpgrp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getpgrp.c

./o32/arctic/libc/unistd/getpid.o: \
    ./arctic/libc/unistd/getpid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getpid.c

./o32/arctic/libc/unistd/getppid.o: \
    ./arctic/libc/unistd/getppid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getppid.c

./o32/arctic/libc/unistd/getsid.o: \
    ./arctic/libc/unistd/getsid.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/getsid.c

./o32/arctic/libc/unistd/isatty.o: \
    ./arctic/libc/unistd/isatty.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/isatty.c

./o32/arctic/libc/unistd/lseek.o: \
    ./arctic/libc/unistd/lseek.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/lseek.c

./o32/arctic/libc/unistd/pipe.o: \
    ./arctic/libc/unistd/pipe.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/pipe.c

./o32/arctic/libc/unistd/read.o: \
    ./arctic/libc/unistd/read.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/read.c

./o32/arctic/libc/unistd/rmdir.o: \
    ./arctic/libc/unistd/rmdir.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/rmdir.c

./o32/arctic/libc/unistd/tcpgrp.o: \
    ./arctic/libc/unistd/tcpgrp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/tcpgrp.c

./o32/arctic/libc/unistd/unlink.o: \
    ./arctic/libc/unistd/unlink.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/unlink.c

./o32/arctic/libc/unistd/write.o: \
    ./arctic/libc/unistd/write.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/unistd/write.c

./o32/arctic/libc/wchar/btowc.o: \
    ./arctic/libc/wchar/btowc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/btowc.c

./o32/arctic/libc/wchar/mbrtowc.o: \
    ./arctic/libc/wchar/mbrtowc.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/mbrtowc.c

./o32/arctic/libc/wchar/mbsinit.o: \
    ./arctic/libc/wchar/mbsinit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/mbsinit.c

./o32/arctic/libc/wchar/wcrtomb.o: \
    ./arctic/libc/wchar/wcrtomb.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/wcrtomb.c

./o32/arctic/libc/wchar/wcslen.o: \
    ./arctic/libc/wchar/wcslen.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/wcslen.c

./o32/arctic/libc/wchar/wcsncmp.o: \
    ./arctic/libc/wchar/wcsncmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/wcsncmp.c

./o32/arctic/libc/wchar/wcwidth.o: \
    ./arctic/libc/wchar/wcwidth.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wchar/wcwidth.c

./o32/arctic/libc/wctype/iswalnum.o: \
    ./arctic/libc/wctype/iswalnum.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswalnum.c

./o32/arctic/libc/wctype/iswalpha.o: \
    ./arctic/libc/wctype/iswalpha.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswalpha.c

./o32/arctic/libc/wctype/iswblank.o: \
    ./arctic/libc/wctype/iswblank.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswblank.c

./o32/arctic/libc/wctype/iswcntrl.o: \
    ./arctic/libc/wctype/iswcntrl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswcntrl.c

./o32/arctic/libc/wctype/iswdigit.o: \
    ./arctic/libc/wctype/iswdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswdigit.c

./o32/arctic/libc/wctype/iswgraph.o: \
    ./arctic/libc/wctype/iswgraph.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswgraph.c

./o32/arctic/libc/wctype/iswlower.o: \
    ./arctic/libc/wctype/iswlower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswlower.c

./o32/arctic/libc/wctype/iswprint.o: \
    ./arctic/libc/wctype/iswprint.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswprint.c

./o32/arctic/libc/wctype/iswpunct.o: \
    ./arctic/libc/wctype/iswpunct.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswpunct.c

./o32/arctic/libc/wctype/iswspace.o: \
    ./arctic/libc/wctype/iswspace.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswspace.c

./o32/arctic/libc/wctype/iswupper.o: \
    ./arctic/libc/wctype/iswupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswupper.c

./o32/arctic/libc/wctype/iswxdigi.o: \
    ./arctic/libc/wctype/iswxdigi.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/iswxdigi.c

./o32/arctic/libc/wctype/towlower.o: \
    ./arctic/libc/wctype/towlower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/towlower.c

./o32/arctic/libc/wctype/towupper.o: \
    ./arctic/libc/wctype/towupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/wctype/towupper.c

./o32/arctic/programs/cat/main.o: \
    ./arctic/programs/cat/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CAT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/cat/main.c

./o32/arctic/programs/cat/operate.o: \
    ./arctic/programs/cat/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CAT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/cat/operate.c

./o32/arctic/programs/cp/main.o: \
    ./arctic/programs/cp/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/cp/main.c

./o32/arctic/programs/cp/operate.o: \
    ./arctic/programs/cp/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/cp/operate.c

./o32/arctic/programs/date/main.o: \
    ./arctic/programs/date/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DATE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/date/main.c

./o32/arctic/programs/date/operate.o: \
    ./arctic/programs/date/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DATE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/date/operate.c

./o32/arctic/programs/dsh/builtin.o: \
    ./arctic/programs/dsh/builtin.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/builtin.c

./o32/arctic/programs/dsh/execute.o: \
    ./arctic/programs/dsh/execute.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/execute.c

./o32/arctic/programs/dsh/line.o: \
    ./arctic/programs/dsh/line.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/line.c

./o32/arctic/programs/dsh/main.o: \
    ./arctic/programs/dsh/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/main.c

./o32/arctic/programs/dsh/operate.o: \
    ./arctic/programs/dsh/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/operate.c

./o32/arctic/programs/dsh/parser.o: \
    ./arctic/programs/dsh/parser.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/parser.c

./o32/arctic/programs/dsh/prompt.o: \
    ./arctic/programs/dsh/prompt.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/prompt.c

./o32/arctic/programs/dsh/tab.o: \
    ./arctic/programs/dsh/tab.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/tab.c

./o32/arctic/programs/dsh/token.o: \
    ./arctic/programs/dsh/token.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/token.c

./o32/arctic/programs/dsh/var.o: \
    ./arctic/programs/dsh/var.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/dsh/var.c

./o32/arctic/programs/env/main.o: \
    ./arctic/programs/env/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_ENV_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/env/main.c

./o32/arctic/programs/env/operate.o: \
    ./arctic/programs/env/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_ENV_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/env/operate.c

./o32/arctic/programs/false/main.o: \
    ./arctic/programs/false/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_FALSE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/false/main.c

./o32/arctic/programs/hd/main.o: \
    ./arctic/programs/hd/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HD_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/hd/main.c

./o32/arctic/programs/hexdump/main.o: \
    ./arctic/programs/hexdump/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HEXDUMP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/hexdump/main.c

./o32/arctic/programs/hexdump/operate.o: \
    ./arctic/programs/hexdump/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HEXDUMP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/hexdump/operate.c

./o32/arctic/programs/init/main.o: \
    ./arctic/programs/init/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_INIT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/init/main.c

./o32/arctic/programs/init/operate.o: \
    ./arctic/programs/init/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_INIT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/init/operate.c

./o32/arctic/programs/ls/main.o: \
    ./arctic/programs/ls/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LS_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/ls/main.c

./o32/arctic/programs/ls/operate.o: \
    ./arctic/programs/ls/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LS_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/ls/operate.c

./o32/arctic/programs/lsusb/main.o: \
    ./arctic/programs/lsusb/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LSUSB_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/lsusb/main.c

./o32/arctic/programs/lsusb/operate.o: \
    ./arctic/programs/lsusb/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LSUSB_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/lsusb/operate.c

./o32/arctic/programs/mkdir/main.o: \
    ./arctic/programs/mkdir/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MKDIR_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/mkdir/main.c

./o32/arctic/programs/mkdir/operate.o: \
    ./arctic/programs/mkdir/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MKDIR_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/mkdir/operate.c

./o32/arctic/programs/more/cat.o: \
    ./arctic/programs/more/cat.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/more/cat.c

./o32/arctic/programs/more/main.o: \
    ./arctic/programs/more/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/more/main.c

./o32/arctic/programs/more/operate.o: \
    ./arctic/programs/more/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/more/operate.c

./o32/arctic/programs/more/term.o: \
    ./arctic/programs/more/term.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/more/term.c

./o32/arctic/programs/nproc/main.o: \
    ./arctic/programs/nproc/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_NPROC_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/nproc/main.c

./o32/arctic/programs/nproc/operate.o: \
    ./arctic/programs/nproc/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_NPROC_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/nproc/operate.c

./o32/arctic/programs/poweroff/main.o: \
    ./arctic/programs/poweroff/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_POWEROFF_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/poweroff/main.c

./o32/arctic/programs/poweroff/operate.o: \
    ./arctic/programs/poweroff/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_POWEROFF_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/poweroff/operate.c

./o32/arctic/programs/ps/main.o: \
    ./arctic/programs/ps/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PS_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/ps/main.c

./o32/arctic/programs/ps/operate.o: \
    ./arctic/programs/ps/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PS_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/ps/operate.c

./o32/arctic/programs/pwd/main.o: \
    ./arctic/programs/pwd/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PWD_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/pwd/main.c

./o32/arctic/programs/pwd/operate.o: \
    ./arctic/programs/pwd/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PWD_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/pwd/operate.c

./o32/arctic/programs/reboot/main.o: \
    ./arctic/programs/reboot/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_REBOOT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/reboot/main.c

./o32/arctic/programs/reboot/operate.o: \
    ./arctic/programs/reboot/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_REBOOT_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/reboot/operate.c

./o32/arctic/programs/rm/main.o: \
    ./arctic/programs/rm/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RM_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/rm/main.c

./o32/arctic/programs/rm/operate.o: \
    ./arctic/programs/rm/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RM_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/rm/operate.c

./o32/arctic/programs/rmdir/main.o: \
    ./arctic/programs/rmdir/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RMDIR_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/rmdir/main.c

./o32/arctic/programs/rmdir/operate.o: \
    ./arctic/programs/rmdir/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RMDIR_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/rmdir/operate.c

./o32/arctic/programs/sleep/main.o: \
    ./arctic/programs/sleep/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_SLEEP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/sleep/main.c

./o32/arctic/programs/sleep/operate.o: \
    ./arctic/programs/sleep/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_SLEEP_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/sleep/operate.c

./o32/arctic/programs/tee/main.o: \
    ./arctic/programs/tee/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TEE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/tee/main.c

./o32/arctic/programs/tee/operate.o: \
    ./arctic/programs/tee/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TEE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/tee/operate.c

./o32/arctic/programs/terminal/keymap.o: \
    ./arctic/programs/terminal/keymap.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/terminal/keymap.c

./o32/arctic/programs/terminal/main.o: \
    ./arctic/programs/terminal/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/terminal/main.c

./o32/arctic/programs/terminal/operate.o: \
    ./arctic/programs/terminal/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/terminal/operate.c

./o32/arctic/programs/true/main.o: \
    ./arctic/programs/true/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TRUE_HEADERS)
	$(ARCTIC_O32)$@ ./arctic/programs/true/main.c

./o32/boot/init/a32/gdt.o: \
    ./boot/init/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/gdt.asm

./o32/boot/init/a32/idt.o: \
    ./boot/init/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/idt.asm

./o32/boot/init/a32/kernel.o: \
    ./boot/init/a32/kernel.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/kernel.asm

./o32/boot/init/a32/start.o: \
    ./boot/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/start.asm

./o32/boot/init/a32/syscall.o: \
    ./boot/init/a32/syscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/syscall.asm

./o32/boot/init/acpi.o: \
    ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/acpi.c

./o32/boot/init/apic.o: \
    ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/apic.c

./o32/boot/init/cpu.o: \
    ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/cpu.c

./o32/boot/init/db.o: \
    ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/db.c

./o32/boot/init/delay.o: \
    ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/delay.c

./o32/boot/init/fs.o: \
    ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/fs.c

./o32/boot/init/gui.o: \
    ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/gui.c

./o32/boot/init/hpet.o: \
    ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/hpet.c

./o32/boot/init/idt.o: \
    ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/idt.c

./o32/boot/init/init.o: \
    ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/init.c

./o32/boot/init/kernel.o: \
    ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/kernel.c

./o32/boot/init/ld.o: \
    ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/ld.c

./o32/boot/init/log.o: \
    ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/log.c

./o32/boot/init/memory.o: \
    ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/memory.c

./o32/boot/init/panic.o: \
    ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/panic.c

./o32/boot/init/pci.o: \
    ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pci.c

./o32/boot/init/pg.o: \
    ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pg.c

./o32/boot/init/pit.o: \
    ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pit.c

./o32/boot/init/print.o: \
    ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/print.c

./o32/boot/init/rtc.o: \
    ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/rtc.c

./o32/boot/init/smp/smp.o: \
    ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/smp.c

./o32/boot/init/smp/start32.o: \
    ./boot/init/smp/start32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/start32.c

./o32/boot/init/smp/start64.o: \
    ./boot/init/smp/start64.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/start64.c

./o32/boot/init/start.o: \
    ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/start.c

./o32/boot/init/table.o: \
    ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/table.c

./o32/boot/init/usb.o: \
    ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/usb.c

./o32/boot/init/vga.o: \
    ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/vga.c

./o32/boot/init/video/_driver.o: \
    ./boot/init/video/_driver.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/video/_driver.c

./o32/boot/init/video/bochs.o: \
    ./boot/init/video/bochs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/video/bochs.c

./o32/common/a32/dark.o: \
    ./common/a32/dark.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./common/a32/dark.asm

./o32/common/a32/string.o: \
    ./common/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./common/a32/string.asm

./o32/common/crc32.o: \
    ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32.c

./o32/common/crc32c.o: \
    ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32c.c

./o32/common/epoch.o: \
    ./common/epoch.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/epoch.c

./o32/common/fat.o: \
    ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/fat.c

./o32/common/stack.o: \
    ./common/stack.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/stack.c

./o32/common/ttf.o: \
    ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/ttf.c

./o32/common/utf8.o: \
    ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/utf8.c

./o32/kernel/base/a32/fb.o: \
    ./kernel/base/a32/fb.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/fb.asm

./o32/kernel/base/a32/gdt.o: \
    ./kernel/base/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/gdt.asm

./o32/kernel/base/a32/idt.o: \
    ./kernel/base/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/idt.asm

./o32/kernel/base/a32/task.o: \
    ./kernel/base/a32/task.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/task.asm

./o32/kernel/base/a32/timer.o: \
    ./kernel/base/a32/timer.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/timer.asm

./o32/kernel/base/apic.o: \
    ./kernel/base/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/apic.c

./o32/kernel/base/console.o: \
    ./kernel/base/console.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/console.c

./o32/kernel/base/delay.o: \
    ./kernel/base/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/delay.c

./o32/kernel/base/event.o: \
    ./kernel/base/event.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/event.c

./o32/kernel/base/fb.o: \
    ./kernel/base/fb.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/fb.c

./o32/kernel/base/gdt.o: \
    ./kernel/base/gdt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/gdt.c

./o32/kernel/base/heap.o: \
    ./kernel/base/heap.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/heap.c

./o32/kernel/base/idt.o: \
    ./kernel/base/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/idt.c

./o32/kernel/base/idt_user.o: \
    ./kernel/base/idt_user.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/idt_user.c

./o32/kernel/base/irq.o: \
    ./kernel/base/irq.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/irq.c

./o32/kernel/base/kmsg.o: \
    ./kernel/base/kmsg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/kmsg.c

./o32/kernel/base/mm.o: \
    ./kernel/base/mm.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/mm.c

./o32/kernel/base/mtx.o: \
    ./kernel/base/mtx.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/mtx.c

./o32/kernel/base/panic.o: \
    ./kernel/base/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/panic.c

./o32/kernel/base/pg.o: \
    ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/pg.c

./o32/kernel/base/ret_user.o: \
    ./kernel/base/ret_user.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/ret_user.c

./o32/kernel/base/runlevel.o: \
    ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/runlevel.c

./o32/kernel/base/spin.o: \
    ./kernel/base/spin.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/spin.c

./o32/kernel/base/start.o: \
    ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/start.c

./o32/kernel/base/task.o: \
    ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/task.c

./o32/kernel/debug/debug.o: \
    ./kernel/debug/debug.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/debug/debug.c

./o32/kernel/lib/a32/coff.o: \
    ./kernel/lib/a32/coff.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/coff.asm

./o32/kernel/lib/a32/cpu.o: \
    ./kernel/lib/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/cpu.asm

./o32/kernel/lib/a32/spin.o: \
    ./kernel/lib/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/spin.asm

./o32/kernel/lib/bitarray.o: \
    ./kernel/lib/bitarray.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/bitarray.c

./o32/kernel/lib/ctype.o: \
    ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/ctype.c

./o32/kernel/lib/huffman.o: \
    ./kernel/lib/huffman.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/huffman.c

./o32/kernel/lib/inflate.o: \
    ./kernel/lib/inflate.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/inflate.c

./o32/kernel/lib/snprintf.o: \
    ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/snprintf.c

./o32/kernel/lib/stdlib.o: \
    ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/stdlib.c

./o32/kernel/lib/strerror.o: \
    ./kernel/lib/strerror.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/strerror.c

./o32/kernel/lib/string.o: \
    ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/string.c

./o32/kernel/misc/bin.o: \
    ./kernel/misc/bin.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/bin.c

./o32/kernel/misc/coff.o: \
    ./kernel/misc/coff.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/coff.c

./o32/kernel/misc/console.o: \
    ./kernel/misc/console.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/console.c

./o32/kernel/misc/dma.o: \
    ./kernel/misc/dma.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/dma.c

./o32/kernel/misc/epoch.o: \
    ./kernel/misc/epoch.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/epoch.c

./o32/kernel/misc/fb_user.o: \
    ./kernel/misc/fb_user.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/fb_user.c

./o32/kernel/misc/floppy.o: \
    ./kernel/misc/floppy.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/floppy.c

./o32/kernel/misc/hdd_mnt.o: \
    ./kernel/misc/hdd_mnt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/hdd_mnt.c

./o32/kernel/misc/hdd_part.o: \
    ./kernel/misc/hdd_part.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/hdd_part.c

./o32/kernel/misc/keyboard.o: \
    ./kernel/misc/keyboard.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/keyboard.c

./o32/kernel/misc/kmsg.o: \
    ./kernel/misc/kmsg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/kmsg.c

./o32/kernel/misc/mouse.o: \
    ./kernel/misc/mouse.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/mouse.c

./o32/kernel/misc/rtc.o: \
    ./kernel/misc/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/rtc.c

./o32/kernel/misc/serial.o: \
    ./kernel/misc/serial.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/serial.c

./o32/kernel/misc/zero.o: \
    ./kernel/misc/zero.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/zero.c

./o32/kernel/network/e1000.o: \
    ./kernel/network/e1000.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/network/e1000.c

./o32/kernel/network/net_node.o: \
    ./kernel/network/net_node.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/network/net_node.c

./o32/kernel/pci/ahci.o: \
    ./kernel/pci/ahci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/pci/ahci.c

./o32/kernel/pci/ide_ctrl.o: \
    ./kernel/pci/ide_ctrl.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/pci/ide_ctrl.c

./o32/kernel/pci/pci.o: \
    ./kernel/pci/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/pci/pci.c

./o32/kernel/ps2/8042.o: \
    ./kernel/ps2/8042.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/8042.c

./o32/kernel/ps2/keyboard.o: \
    ./kernel/ps2/keyboard.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/keyboard.c

./o32/kernel/ps2/mouse.o: \
    ./kernel/ps2/mouse.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/mouse.c

./o32/kernel/run/run.o: \
    ./kernel/run/run.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/run/run.c

./o32/kernel/sched/sched.o: \
    ./kernel/sched/sched.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/sched/sched.c

./o32/kernel/syscall/a32/trap.o: \
    ./kernel/syscall/a32/trap.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/syscall/a32/trap.asm

./o32/kernel/syscall/arg.o: \
    ./kernel/syscall/arg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/arg.c

./o32/kernel/syscall/errno.o: \
    ./kernel/syscall/errno.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/errno.c

./o32/kernel/syscall/file.o: \
    ./kernel/syscall/file.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/file.c

./o32/kernel/syscall/ioctl.o: \
    ./kernel/syscall/ioctl.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/ioctl.c

./o32/kernel/syscall/kill.o: \
    ./kernel/syscall/kill.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/kill.c

./o32/kernel/syscall/misc.o: \
    ./kernel/syscall/misc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/misc.c

./o32/kernel/syscall/proc.o: \
    ./kernel/syscall/proc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/proc.c

./o32/kernel/syscall/reboot.o: \
    ./kernel/syscall/reboot.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/reboot.c

./o32/kernel/syscall/sleep.o: \
    ./kernel/syscall/sleep.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/sleep.c

./o32/kernel/syscall/spawn.o: \
    ./kernel/syscall/spawn.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/spawn.c

./o32/kernel/syscall/syscall.o: \
    ./kernel/syscall/syscall.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/syscall.c

./o32/kernel/usb/boot_kbd.o: \
    ./kernel/usb/boot_kbd.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/boot_kbd.c

./o32/kernel/usb/boot_mse.o: \
    ./kernel/usb/boot_mse.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/boot_mse.c

./o32/kernel/usb/ehci.o: \
    ./kernel/usb/ehci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/ehci.c

./o32/kernel/usb/ohci.o: \
    ./kernel/usb/ohci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/ohci.c

./o32/kernel/usb/uhci.o: \
    ./kernel/usb/uhci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/uhci.c

./o32/kernel/usb/usb_ep.o: \
    ./kernel/usb/usb_ep.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_ep.c

./o32/kernel/usb/usb_hid.o: \
    ./kernel/usb/usb_hid.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_hid.c

./o32/kernel/usb/usb_mnt.o: \
    ./kernel/usb/usb_mnt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_mnt.c

./o32/kernel/usb/usb_msc.o: \
    ./kernel/usb/usb_msc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_msc.c

./o32/kernel/usb/usb_node.o: \
    ./kernel/usb/usb_node.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_node.c

./o32/kernel/usb/usb_task.o: \
    ./kernel/usb/usb_task.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/usb_task.c

./o32/kernel/usb/xhci.o: \
    ./kernel/usb/xhci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/usb/xhci.c

./o32/kernel/vfs/default.o: \
    ./kernel/vfs/default.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/default.c

./o32/kernel/vfs/devfs.o: \
    ./kernel/vfs/devfs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/devfs.c

./o32/kernel/vfs/fat_io.o: \
    ./kernel/vfs/fat_io.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/fat_io.c

./o32/kernel/vfs/pipe.o: \
    ./kernel/vfs/pipe.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/pipe.c

./o32/kernel/vfs/pty.o: \
    ./kernel/vfs/pty.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/pty.c

./o32/kernel/vfs/root.o: \
    ./kernel/vfs/root.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/root.c

./o32/kernel/vfs/vfs.o: \
    ./kernel/vfs/vfs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/vfs.c

##############################################################################

./o64/arctic/apps/editor/__dancy.o: \
    ./arctic/apps/editor/__dancy.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_EDITOR_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/apps/editor/__dancy.c

./o64/arctic/apps/hello/main.o: \
    ./arctic/apps/hello/main.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_HELLO_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/apps/hello/main.c

./o64/arctic/apps/hello/operate.o: \
    ./arctic/apps/hello/operate.c $(DANCY_DEPS) \
    $(ARCTIC_APPS_HELLO_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/apps/hello/operate.c

./o64/arctic/libc/a64/memcmp.o: \
    ./arctic/libc/a64/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcmp.asm

./o64/arctic/libc/a64/memcpy.o: \
    ./arctic/libc/a64/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcpy.asm

./o64/arctic/libc/a64/memmove.o: \
    ./arctic/libc/a64/memmove.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memmove.asm

./o64/arctic/libc/a64/memset.o: \
    ./arctic/libc/a64/memset.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memset.asm

./o64/arctic/libc/a64/pointer.o: \
    ./arctic/libc/a64/pointer.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/pointer.asm

./o64/arctic/libc/a64/spin.o: \
    ./arctic/libc/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/spin.asm

./o64/arctic/libc/a64/start.o: \
    ./arctic/libc/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/start.asm

./o64/arctic/libc/a64/strlen.o: \
    ./arctic/libc/a64/strlen.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/strlen.asm

./o64/arctic/libc/a64/syscall0.o: \
    ./arctic/libc/a64/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall0.asm

./o64/arctic/libc/a64/syscall1.o: \
    ./arctic/libc/a64/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall1.asm

./o64/arctic/libc/a64/syscall2.o: \
    ./arctic/libc/a64/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall2.asm

./o64/arctic/libc/a64/syscall3.o: \
    ./arctic/libc/a64/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall3.asm

./o64/arctic/libc/a64/syscall4.o: \
    ./arctic/libc/a64/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall4.asm

./o64/arctic/libc/a64/syscall5.o: \
    ./arctic/libc/a64/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall5.asm

./o64/arctic/libc/ctype/isalnum.o: \
    ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalnum.c

./o64/arctic/libc/ctype/isalpha.o: \
    ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalpha.c

./o64/arctic/libc/ctype/isblank.o: \
    ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isblank.c

./o64/arctic/libc/ctype/iscntrl.o: \
    ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/iscntrl.c

./o64/arctic/libc/ctype/isdigit.o: \
    ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isdigit.c

./o64/arctic/libc/ctype/isgraph.o: \
    ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isgraph.c

./o64/arctic/libc/ctype/islower.o: \
    ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/islower.c

./o64/arctic/libc/ctype/isprint.o: \
    ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isprint.c

./o64/arctic/libc/ctype/ispunct.o: \
    ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/ispunct.c

./o64/arctic/libc/ctype/isspace.o: \
    ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isspace.c

./o64/arctic/libc/ctype/isupper.o: \
    ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isupper.c

./o64/arctic/libc/ctype/isxdigit.o: \
    ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isxdigit.c

./o64/arctic/libc/ctype/tolower.o: \
    ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/tolower.c

./o64/arctic/libc/ctype/toupper.o: \
    ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/toupper.c

./o64/arctic/libc/dirent/dir.o: \
    ./arctic/libc/dirent/dir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/dir.c

./o64/arctic/libc/dirent/dirfd.o: \
    ./arctic/libc/dirent/dirfd.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/dirfd.c

./o64/arctic/libc/dirent/rewind.o: \
    ./arctic/libc/dirent/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/dirent/rewind.c

./o64/arctic/libc/err/err.o: \
    ./arctic/libc/err/err.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/err/err.c

./o64/arctic/libc/err/errx.o: \
    ./arctic/libc/err/errx.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/err/errx.c

./o64/arctic/libc/err/warn.o: \
    ./arctic/libc/err/warn.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/err/warn.c

./o64/arctic/libc/err/warnx.o: \
    ./arctic/libc/err/warnx.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/err/warnx.c

./o64/arctic/libc/errno/errno.o: \
    ./arctic/libc/errno/errno.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/errno/errno.c

./o64/arctic/libc/fcntl/fcntl.o: \
    ./arctic/libc/fcntl/fcntl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/fcntl/fcntl.c

./o64/arctic/libc/fcntl/open.o: \
    ./arctic/libc/fcntl/open.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/fcntl/open.c

./o64/arctic/libc/glob/glob.o: \
    ./arctic/libc/glob/glob.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/glob/glob.c

./o64/arctic/libc/glob/globfree.o: \
    ./arctic/libc/glob/globfree.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/glob/globfree.c

./o64/arctic/libc/keymap/_keymap.o: \
    ./arctic/libc/keymap/_keymap.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/_keymap.c

./o64/arctic/libc/keymap/_write.o: \
    ./arctic/libc/keymap/_write.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/_write.c

./o64/arctic/libc/keymap/de.o: \
    ./arctic/libc/keymap/de.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/de.c

./o64/arctic/libc/keymap/dk.o: \
    ./arctic/libc/keymap/dk.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/dk.c

./o64/arctic/libc/keymap/et.o: \
    ./arctic/libc/keymap/et.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/et.c

./o64/arctic/libc/keymap/fi.o: \
    ./arctic/libc/keymap/fi.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/fi.c

./o64/arctic/libc/keymap/no.o: \
    ./arctic/libc/keymap/no.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/no.c

./o64/arctic/libc/keymap/sv.o: \
    ./arctic/libc/keymap/sv.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/sv.c

./o64/arctic/libc/keymap/uk.o: \
    ./arctic/libc/keymap/uk.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/uk.c

./o64/arctic/libc/keymap/us.o: \
    ./arctic/libc/keymap/us.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/keymap/us.c

./o64/arctic/libc/locale/conv.o: \
    ./arctic/libc/locale/conv.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/locale/conv.c

./o64/arctic/libc/locale/set.o: \
    ./arctic/libc/locale/set.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/locale/set.c

./o64/arctic/libc/misc/__assert.o: \
    ./arctic/libc/misc/__assert.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/__assert.c

./o64/arctic/libc/misc/__main.o: \
    ./arctic/libc/misc/__main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/__main.c

./o64/arctic/libc/misc/memusage.o: \
    ./arctic/libc/misc/memusage.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/memusage.c

./o64/arctic/libc/misc/procinfo.o: \
    ./arctic/libc/misc/procinfo.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/procinfo.c

./o64/arctic/libc/misc/proclist.o: \
    ./arctic/libc/misc/proclist.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/misc/proclist.c

./o64/arctic/libc/poll/poll.o: \
    ./arctic/libc/poll/poll.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/poll/poll.c

./o64/arctic/libc/poll/ppoll.o: \
    ./arctic/libc/poll/ppoll.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/poll/ppoll.c

./o64/arctic/libc/pty/openpty.o: \
    ./arctic/libc/pty/openpty.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/pty/openpty.c

./o64/arctic/libc/regex/regcomp.o: \
    ./arctic/libc/regex/regcomp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/regex/regcomp.c

./o64/arctic/libc/regex/regerror.o: \
    ./arctic/libc/regex/regerror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/regex/regerror.c

./o64/arctic/libc/regex/regexec.o: \
    ./arctic/libc/regex/regexec.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/regex/regexec.c

./o64/arctic/libc/regex/regfree.o: \
    ./arctic/libc/regex/regfree.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/regex/regfree.c

./o64/arctic/libc/signal/action.o: \
    ./arctic/libc/signal/action.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/action.c

./o64/arctic/libc/signal/kill.o: \
    ./arctic/libc/signal/kill.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/kill.c

./o64/arctic/libc/signal/pending.o: \
    ./arctic/libc/signal/pending.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/pending.c

./o64/arctic/libc/signal/procmask.o: \
    ./arctic/libc/signal/procmask.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/procmask.c

./o64/arctic/libc/signal/raise.o: \
    ./arctic/libc/signal/raise.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/raise.c

./o64/arctic/libc/signal/signal.o: \
    ./arctic/libc/signal/signal.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/signal.c

./o64/arctic/libc/signal/sigset_t.o: \
    ./arctic/libc/signal/sigset_t.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/signal/sigset_t.c

./o64/arctic/libc/spawn/actions.o: \
    ./arctic/libc/spawn/actions.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/actions.c

./o64/arctic/libc/spawn/addclose.o: \
    ./arctic/libc/spawn/addclose.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/addclose.c

./o64/arctic/libc/spawn/adddup2.o: \
    ./arctic/libc/spawn/adddup2.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/adddup2.c

./o64/arctic/libc/spawn/addopen.o: \
    ./arctic/libc/spawn/addopen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/addopen.c

./o64/arctic/libc/spawn/attr.o: \
    ./arctic/libc/spawn/attr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/attr.c

./o64/arctic/libc/spawn/flags.o: \
    ./arctic/libc/spawn/flags.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/flags.c

./o64/arctic/libc/spawn/pgroup.o: \
    ./arctic/libc/spawn/pgroup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/pgroup.c

./o64/arctic/libc/spawn/policy.o: \
    ./arctic/libc/spawn/policy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/policy.c

./o64/arctic/libc/spawn/sched.o: \
    ./arctic/libc/spawn/sched.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sched.c

./o64/arctic/libc/spawn/sigdef.o: \
    ./arctic/libc/spawn/sigdef.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sigdef.c

./o64/arctic/libc/spawn/sigmask.o: \
    ./arctic/libc/spawn/sigmask.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/sigmask.c

./o64/arctic/libc/spawn/spawn.o: \
    ./arctic/libc/spawn/spawn.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/spawn.c

./o64/arctic/libc/spawn/spawnp.o: \
    ./arctic/libc/spawn/spawnp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/spawnp.c

./o64/arctic/libc/spawn/tcset.o: \
    ./arctic/libc/spawn/tcset.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/spawn/tcset.c

./o64/arctic/libc/start.o: \
    ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/start.c

./o64/arctic/libc/stdio/_io.o: \
    ./arctic/libc/stdio/_io.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/_io.c

./o64/arctic/libc/stdio/_scan.o: \
    ./arctic/libc/stdio/_scan.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/_scan.c

./o64/arctic/libc/stdio/asprintf.o: \
    ./arctic/libc/stdio/asprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/asprintf.c

./o64/arctic/libc/stdio/clearerr.o: \
    ./arctic/libc/stdio/clearerr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/clearerr.c

./o64/arctic/libc/stdio/fclose.o: \
    ./arctic/libc/stdio/fclose.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fclose.c

./o64/arctic/libc/stdio/feof.o: \
    ./arctic/libc/stdio/feof.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/feof.c

./o64/arctic/libc/stdio/ferror.o: \
    ./arctic/libc/stdio/ferror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/ferror.c

./o64/arctic/libc/stdio/fflush.o: \
    ./arctic/libc/stdio/fflush.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fflush.c

./o64/arctic/libc/stdio/fgetc.o: \
    ./arctic/libc/stdio/fgetc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fgetc.c

./o64/arctic/libc/stdio/fgetpos.o: \
    ./arctic/libc/stdio/fgetpos.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fgetpos.c

./o64/arctic/libc/stdio/fgets.o: \
    ./arctic/libc/stdio/fgets.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fgets.c

./o64/arctic/libc/stdio/fileno.o: \
    ./arctic/libc/stdio/fileno.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fileno.c

./o64/arctic/libc/stdio/fopen.o: \
    ./arctic/libc/stdio/fopen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fopen.c

./o64/arctic/libc/stdio/fprintf.o: \
    ./arctic/libc/stdio/fprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fprintf.c

./o64/arctic/libc/stdio/fputc.o: \
    ./arctic/libc/stdio/fputc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fputc.c

./o64/arctic/libc/stdio/fputs.o: \
    ./arctic/libc/stdio/fputs.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fputs.c

./o64/arctic/libc/stdio/fread.o: \
    ./arctic/libc/stdio/fread.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fread.c

./o64/arctic/libc/stdio/freopen.o: \
    ./arctic/libc/stdio/freopen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/freopen.c

./o64/arctic/libc/stdio/fscanf.o: \
    ./arctic/libc/stdio/fscanf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fscanf.c

./o64/arctic/libc/stdio/fseek.o: \
    ./arctic/libc/stdio/fseek.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fseek.c

./o64/arctic/libc/stdio/fseeko.o: \
    ./arctic/libc/stdio/fseeko.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fseeko.c

./o64/arctic/libc/stdio/fsetpos.o: \
    ./arctic/libc/stdio/fsetpos.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fsetpos.c

./o64/arctic/libc/stdio/ftell.o: \
    ./arctic/libc/stdio/ftell.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/ftell.c

./o64/arctic/libc/stdio/ftello.o: \
    ./arctic/libc/stdio/ftello.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/ftello.c

./o64/arctic/libc/stdio/fwrite.o: \
    ./arctic/libc/stdio/fwrite.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/fwrite.c

./o64/arctic/libc/stdio/getc.o: \
    ./arctic/libc/stdio/getc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/getc.c

./o64/arctic/libc/stdio/getchar.o: \
    ./arctic/libc/stdio/getchar.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/getchar.c

./o64/arctic/libc/stdio/getdelim.o: \
    ./arctic/libc/stdio/getdelim.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/getdelim.c

./o64/arctic/libc/stdio/getline.o: \
    ./arctic/libc/stdio/getline.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/getline.c

./o64/arctic/libc/stdio/pclose.o: \
    ./arctic/libc/stdio/pclose.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/pclose.c

./o64/arctic/libc/stdio/perror.o: \
    ./arctic/libc/stdio/perror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/perror.c

./o64/arctic/libc/stdio/popen.o: \
    ./arctic/libc/stdio/popen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/popen.c

./o64/arctic/libc/stdio/printf.o: \
    ./arctic/libc/stdio/printf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/printf.c

./o64/arctic/libc/stdio/putc.o: \
    ./arctic/libc/stdio/putc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/putc.c

./o64/arctic/libc/stdio/putchar.o: \
    ./arctic/libc/stdio/putchar.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/putchar.c

./o64/arctic/libc/stdio/puts.o: \
    ./arctic/libc/stdio/puts.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/puts.c

./o64/arctic/libc/stdio/remove.o: \
    ./arctic/libc/stdio/remove.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/remove.c

./o64/arctic/libc/stdio/rename.o: \
    ./arctic/libc/stdio/rename.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/rename.c

./o64/arctic/libc/stdio/rewind.o: \
    ./arctic/libc/stdio/rewind.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/rewind.c

./o64/arctic/libc/stdio/scanf.o: \
    ./arctic/libc/stdio/scanf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/scanf.c

./o64/arctic/libc/stdio/setbuf.o: \
    ./arctic/libc/stdio/setbuf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/setbuf.c

./o64/arctic/libc/stdio/setvbuf.o: \
    ./arctic/libc/stdio/setvbuf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/setvbuf.c

./o64/arctic/libc/stdio/snprintf.o: \
    ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/snprintf.c

./o64/arctic/libc/stdio/sprintf.o: \
    ./arctic/libc/stdio/sprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/sprintf.c

./o64/arctic/libc/stdio/sscanf.o: \
    ./arctic/libc/stdio/sscanf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/sscanf.c

./o64/arctic/libc/stdio/ungetc.o: \
    ./arctic/libc/stdio/ungetc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/ungetc.c

./o64/arctic/libc/stdio/vfprintf.o: \
    ./arctic/libc/stdio/vfprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/vfprintf.c

./o64/arctic/libc/stdio/vsprintf.o: \
    ./arctic/libc/stdio/vsprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/vsprintf.c

./o64/arctic/libc/stdlib/_ull.o: \
    ./arctic/libc/stdlib/_ull.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/_ull.c

./o64/arctic/libc/stdlib/abort.o: \
    ./arctic/libc/stdlib/abort.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/abort.c

./o64/arctic/libc/stdlib/abs.o: \
    ./arctic/libc/stdlib/abs.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/abs.c

./o64/arctic/libc/stdlib/alloc.o: \
    ./arctic/libc/stdlib/alloc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/alloc.c

./o64/arctic/libc/stdlib/atexit.o: \
    ./arctic/libc/stdlib/atexit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/atexit.c

./o64/arctic/libc/stdlib/atoi.o: \
    ./arctic/libc/stdlib/atoi.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/atoi.c

./o64/arctic/libc/stdlib/atol.o: \
    ./arctic/libc/stdlib/atol.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/atol.c

./o64/arctic/libc/stdlib/atoll.o: \
    ./arctic/libc/stdlib/atoll.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/atoll.c

./o64/arctic/libc/stdlib/exit.o: \
    ./arctic/libc/stdlib/exit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/exit.c

./o64/arctic/libc/stdlib/getenv.o: \
    ./arctic/libc/stdlib/getenv.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/getenv.c

./o64/arctic/libc/stdlib/labs.o: \
    ./arctic/libc/stdlib/labs.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/labs.c

./o64/arctic/libc/stdlib/llabs.o: \
    ./arctic/libc/stdlib/llabs.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/llabs.c

./o64/arctic/libc/stdlib/qsort.o: \
    ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/qsort.c

./o64/arctic/libc/stdlib/realpath.o: \
    ./arctic/libc/stdlib/realpath.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/realpath.c

./o64/arctic/libc/stdlib/strtol.o: \
    ./arctic/libc/stdlib/strtol.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/strtol.c

./o64/arctic/libc/stdlib/strtoll.o: \
    ./arctic/libc/stdlib/strtoll.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/strtoll.c

./o64/arctic/libc/stdlib/strtoul.o: \
    ./arctic/libc/stdlib/strtoul.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/strtoul.c

./o64/arctic/libc/stdlib/strtoull.o: \
    ./arctic/libc/stdlib/strtoull.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/strtoull.c

./o64/arctic/libc/stdlib/system.o: \
    ./arctic/libc/stdlib/system.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/system.c

./o64/arctic/libc/string/casecmp.o: \
    ./arctic/libc/string/casecmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/casecmp.c

./o64/arctic/libc/string/memchr.o: \
    ./arctic/libc/string/memchr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/memchr.c

./o64/arctic/libc/string/strcat.o: \
    ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcat.c

./o64/arctic/libc/string/strchr.o: \
    ./arctic/libc/string/strchr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strchr.c

./o64/arctic/libc/string/strcmp.o: \
    ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcmp.c

./o64/arctic/libc/string/strcpy.o: \
    ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcpy.c

./o64/arctic/libc/string/strcspn.o: \
    ./arctic/libc/string/strcspn.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcspn.c

./o64/arctic/libc/string/strdup.o: \
    ./arctic/libc/string/strdup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strdup.c

./o64/arctic/libc/string/strerror.o: \
    ./arctic/libc/string/strerror.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strerror.c

./o64/arctic/libc/string/strncmp.o: \
    ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncmp.c

./o64/arctic/libc/string/strncpy.o: \
    ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncpy.c

./o64/arctic/libc/string/strndup.o: \
    ./arctic/libc/string/strndup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strndup.c

./o64/arctic/libc/string/strpbrk.o: \
    ./arctic/libc/string/strpbrk.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strpbrk.c

./o64/arctic/libc/string/strrchr.o: \
    ./arctic/libc/string/strrchr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strrchr.c

./o64/arctic/libc/string/strspn.o: \
    ./arctic/libc/string/strspn.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strspn.c

./o64/arctic/libc/string/strstr.o: \
    ./arctic/libc/string/strstr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strstr.c

./o64/arctic/libc/string/strtok.o: \
    ./arctic/libc/string/strtok.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strtok.c

./o64/arctic/libc/sys/chmod.o: \
    ./arctic/libc/sys/chmod.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/chmod.c

./o64/arctic/libc/sys/fdset.o: \
    ./arctic/libc/sys/fdset.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/fdset.c

./o64/arctic/libc/sys/fstat.o: \
    ./arctic/libc/sys/fstat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/fstat.c

./o64/arctic/libc/sys/ioctl.o: \
    ./arctic/libc/sys/ioctl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/ioctl.c

./o64/arctic/libc/sys/lstat.o: \
    ./arctic/libc/sys/lstat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/lstat.c

./o64/arctic/libc/sys/mkdir.o: \
    ./arctic/libc/sys/mkdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mkdir.c

./o64/arctic/libc/sys/mmap.o: \
    ./arctic/libc/sys/mmap.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mmap.c

./o64/arctic/libc/sys/mprotect.o: \
    ./arctic/libc/sys/mprotect.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/mprotect.c

./o64/arctic/libc/sys/msync.o: \
    ./arctic/libc/sys/msync.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/msync.c

./o64/arctic/libc/sys/munmap.o: \
    ./arctic/libc/sys/munmap.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/munmap.c

./o64/arctic/libc/sys/select.o: \
    ./arctic/libc/sys/select.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/select.c

./o64/arctic/libc/sys/stat.o: \
    ./arctic/libc/sys/stat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/stat.c

./o64/arctic/libc/sys/umask.o: \
    ./arctic/libc/sys/umask.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/umask.c

./o64/arctic/libc/sys/wait.o: \
    ./arctic/libc/sys/wait.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/wait.c

./o64/arctic/libc/sys/waitpid.o: \
    ./arctic/libc/sys/waitpid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/sys/waitpid.c

./o64/arctic/libc/termios/getattr.o: \
    ./arctic/libc/termios/getattr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/termios/getattr.c

./o64/arctic/libc/termios/getspeed.o: \
    ./arctic/libc/termios/getspeed.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/termios/getspeed.c

./o64/arctic/libc/termios/setattr.o: \
    ./arctic/libc/termios/setattr.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/termios/setattr.c

./o64/arctic/libc/termios/setspeed.o: \
    ./arctic/libc/termios/setspeed.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/termios/setspeed.c

./o64/arctic/libc/threads/mtx.o: \
    ./arctic/libc/threads/mtx.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/threads/mtx.c

./o64/arctic/libc/time/_time.o: \
    ./arctic/libc/time/_time.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/_time.c

./o64/arctic/libc/time/asctime.o: \
    ./arctic/libc/time/asctime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/asctime.c

./o64/arctic/libc/time/clock.o: \
    ./arctic/libc/time/clock.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/clock.c

./o64/arctic/libc/time/ctime.o: \
    ./arctic/libc/time/ctime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/ctime.c

./o64/arctic/libc/time/difftime.o: \
    ./arctic/libc/time/difftime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/difftime.c

./o64/arctic/libc/time/gettime.o: \
    ./arctic/libc/time/gettime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/gettime.c

./o64/arctic/libc/time/gmtime.o: \
    ./arctic/libc/time/gmtime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/gmtime.c

./o64/arctic/libc/time/local.o: \
    ./arctic/libc/time/local.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/local.c

./o64/arctic/libc/time/mktime.o: \
    ./arctic/libc/time/mktime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/mktime.c

./o64/arctic/libc/time/settime.o: \
    ./arctic/libc/time/settime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/settime.c

./o64/arctic/libc/time/sleep.o: \
    ./arctic/libc/time/sleep.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/sleep.c

./o64/arctic/libc/time/strftime.o: \
    ./arctic/libc/time/strftime.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/strftime.c

./o64/arctic/libc/time/time.o: \
    ./arctic/libc/time/time.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/time/time.c

./o64/arctic/libc/unistd/_exit.o: \
    ./arctic/libc/unistd/_exit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/_exit.c

./o64/arctic/libc/unistd/chdir.o: \
    ./arctic/libc/unistd/chdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/chdir.c

./o64/arctic/libc/unistd/close.o: \
    ./arctic/libc/unistd/close.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/close.c

./o64/arctic/libc/unistd/dup.o: \
    ./arctic/libc/unistd/dup.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/dup.c

./o64/arctic/libc/unistd/dup2.o: \
    ./arctic/libc/unistd/dup2.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/dup2.c

./o64/arctic/libc/unistd/execve.o: \
    ./arctic/libc/unistd/execve.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/execve.c

./o64/arctic/libc/unistd/getcwd.o: \
    ./arctic/libc/unistd/getcwd.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getcwd.c

./o64/arctic/libc/unistd/getpgid.o: \
    ./arctic/libc/unistd/getpgid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getpgid.c

./o64/arctic/libc/unistd/getpgrp.o: \
    ./arctic/libc/unistd/getpgrp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getpgrp.c

./o64/arctic/libc/unistd/getpid.o: \
    ./arctic/libc/unistd/getpid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getpid.c

./o64/arctic/libc/unistd/getppid.o: \
    ./arctic/libc/unistd/getppid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getppid.c

./o64/arctic/libc/unistd/getsid.o: \
    ./arctic/libc/unistd/getsid.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/getsid.c

./o64/arctic/libc/unistd/isatty.o: \
    ./arctic/libc/unistd/isatty.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/isatty.c

./o64/arctic/libc/unistd/lseek.o: \
    ./arctic/libc/unistd/lseek.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/lseek.c

./o64/arctic/libc/unistd/pipe.o: \
    ./arctic/libc/unistd/pipe.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/pipe.c

./o64/arctic/libc/unistd/read.o: \
    ./arctic/libc/unistd/read.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/read.c

./o64/arctic/libc/unistd/rmdir.o: \
    ./arctic/libc/unistd/rmdir.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/rmdir.c

./o64/arctic/libc/unistd/tcpgrp.o: \
    ./arctic/libc/unistd/tcpgrp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/tcpgrp.c

./o64/arctic/libc/unistd/unlink.o: \
    ./arctic/libc/unistd/unlink.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/unlink.c

./o64/arctic/libc/unistd/write.o: \
    ./arctic/libc/unistd/write.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/unistd/write.c

./o64/arctic/libc/wchar/btowc.o: \
    ./arctic/libc/wchar/btowc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/btowc.c

./o64/arctic/libc/wchar/mbrtowc.o: \
    ./arctic/libc/wchar/mbrtowc.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/mbrtowc.c

./o64/arctic/libc/wchar/mbsinit.o: \
    ./arctic/libc/wchar/mbsinit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/mbsinit.c

./o64/arctic/libc/wchar/wcrtomb.o: \
    ./arctic/libc/wchar/wcrtomb.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/wcrtomb.c

./o64/arctic/libc/wchar/wcslen.o: \
    ./arctic/libc/wchar/wcslen.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/wcslen.c

./o64/arctic/libc/wchar/wcsncmp.o: \
    ./arctic/libc/wchar/wcsncmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/wcsncmp.c

./o64/arctic/libc/wchar/wcwidth.o: \
    ./arctic/libc/wchar/wcwidth.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wchar/wcwidth.c

./o64/arctic/libc/wctype/iswalnum.o: \
    ./arctic/libc/wctype/iswalnum.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswalnum.c

./o64/arctic/libc/wctype/iswalpha.o: \
    ./arctic/libc/wctype/iswalpha.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswalpha.c

./o64/arctic/libc/wctype/iswblank.o: \
    ./arctic/libc/wctype/iswblank.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswblank.c

./o64/arctic/libc/wctype/iswcntrl.o: \
    ./arctic/libc/wctype/iswcntrl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswcntrl.c

./o64/arctic/libc/wctype/iswdigit.o: \
    ./arctic/libc/wctype/iswdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswdigit.c

./o64/arctic/libc/wctype/iswgraph.o: \
    ./arctic/libc/wctype/iswgraph.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswgraph.c

./o64/arctic/libc/wctype/iswlower.o: \
    ./arctic/libc/wctype/iswlower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswlower.c

./o64/arctic/libc/wctype/iswprint.o: \
    ./arctic/libc/wctype/iswprint.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswprint.c

./o64/arctic/libc/wctype/iswpunct.o: \
    ./arctic/libc/wctype/iswpunct.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswpunct.c

./o64/arctic/libc/wctype/iswspace.o: \
    ./arctic/libc/wctype/iswspace.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswspace.c

./o64/arctic/libc/wctype/iswupper.o: \
    ./arctic/libc/wctype/iswupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswupper.c

./o64/arctic/libc/wctype/iswxdigi.o: \
    ./arctic/libc/wctype/iswxdigi.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/iswxdigi.c

./o64/arctic/libc/wctype/towlower.o: \
    ./arctic/libc/wctype/towlower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/towlower.c

./o64/arctic/libc/wctype/towupper.o: \
    ./arctic/libc/wctype/towupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/wctype/towupper.c

./o64/arctic/programs/cat/main.o: \
    ./arctic/programs/cat/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CAT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/cat/main.c

./o64/arctic/programs/cat/operate.o: \
    ./arctic/programs/cat/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CAT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/cat/operate.c

./o64/arctic/programs/cp/main.o: \
    ./arctic/programs/cp/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/cp/main.c

./o64/arctic/programs/cp/operate.o: \
    ./arctic/programs/cp/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_CP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/cp/operate.c

./o64/arctic/programs/date/main.o: \
    ./arctic/programs/date/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DATE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/date/main.c

./o64/arctic/programs/date/operate.o: \
    ./arctic/programs/date/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DATE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/date/operate.c

./o64/arctic/programs/dsh/builtin.o: \
    ./arctic/programs/dsh/builtin.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/builtin.c

./o64/arctic/programs/dsh/execute.o: \
    ./arctic/programs/dsh/execute.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/execute.c

./o64/arctic/programs/dsh/line.o: \
    ./arctic/programs/dsh/line.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/line.c

./o64/arctic/programs/dsh/main.o: \
    ./arctic/programs/dsh/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/main.c

./o64/arctic/programs/dsh/operate.o: \
    ./arctic/programs/dsh/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/operate.c

./o64/arctic/programs/dsh/parser.o: \
    ./arctic/programs/dsh/parser.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/parser.c

./o64/arctic/programs/dsh/prompt.o: \
    ./arctic/programs/dsh/prompt.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/prompt.c

./o64/arctic/programs/dsh/tab.o: \
    ./arctic/programs/dsh/tab.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/tab.c

./o64/arctic/programs/dsh/token.o: \
    ./arctic/programs/dsh/token.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/token.c

./o64/arctic/programs/dsh/var.o: \
    ./arctic/programs/dsh/var.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_DSH_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/dsh/var.c

./o64/arctic/programs/env/main.o: \
    ./arctic/programs/env/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_ENV_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/env/main.c

./o64/arctic/programs/env/operate.o: \
    ./arctic/programs/env/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_ENV_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/env/operate.c

./o64/arctic/programs/false/main.o: \
    ./arctic/programs/false/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_FALSE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/false/main.c

./o64/arctic/programs/hd/main.o: \
    ./arctic/programs/hd/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HD_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/hd/main.c

./o64/arctic/programs/hexdump/main.o: \
    ./arctic/programs/hexdump/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HEXDUMP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/hexdump/main.c

./o64/arctic/programs/hexdump/operate.o: \
    ./arctic/programs/hexdump/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_HEXDUMP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/hexdump/operate.c

./o64/arctic/programs/init/main.o: \
    ./arctic/programs/init/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_INIT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/init/main.c

./o64/arctic/programs/init/operate.o: \
    ./arctic/programs/init/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_INIT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/init/operate.c

./o64/arctic/programs/ls/main.o: \
    ./arctic/programs/ls/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LS_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/ls/main.c

./o64/arctic/programs/ls/operate.o: \
    ./arctic/programs/ls/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LS_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/ls/operate.c

./o64/arctic/programs/lsusb/main.o: \
    ./arctic/programs/lsusb/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LSUSB_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/lsusb/main.c

./o64/arctic/programs/lsusb/operate.o: \
    ./arctic/programs/lsusb/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_LSUSB_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/lsusb/operate.c

./o64/arctic/programs/mkdir/main.o: \
    ./arctic/programs/mkdir/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MKDIR_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/mkdir/main.c

./o64/arctic/programs/mkdir/operate.o: \
    ./arctic/programs/mkdir/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MKDIR_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/mkdir/operate.c

./o64/arctic/programs/more/cat.o: \
    ./arctic/programs/more/cat.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/more/cat.c

./o64/arctic/programs/more/main.o: \
    ./arctic/programs/more/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/more/main.c

./o64/arctic/programs/more/operate.o: \
    ./arctic/programs/more/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/more/operate.c

./o64/arctic/programs/more/term.o: \
    ./arctic/programs/more/term.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_MORE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/more/term.c

./o64/arctic/programs/nproc/main.o: \
    ./arctic/programs/nproc/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_NPROC_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/nproc/main.c

./o64/arctic/programs/nproc/operate.o: \
    ./arctic/programs/nproc/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_NPROC_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/nproc/operate.c

./o64/arctic/programs/poweroff/main.o: \
    ./arctic/programs/poweroff/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_POWEROFF_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/poweroff/main.c

./o64/arctic/programs/poweroff/operate.o: \
    ./arctic/programs/poweroff/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_POWEROFF_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/poweroff/operate.c

./o64/arctic/programs/ps/main.o: \
    ./arctic/programs/ps/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PS_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/ps/main.c

./o64/arctic/programs/ps/operate.o: \
    ./arctic/programs/ps/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PS_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/ps/operate.c

./o64/arctic/programs/pwd/main.o: \
    ./arctic/programs/pwd/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PWD_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/pwd/main.c

./o64/arctic/programs/pwd/operate.o: \
    ./arctic/programs/pwd/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_PWD_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/pwd/operate.c

./o64/arctic/programs/reboot/main.o: \
    ./arctic/programs/reboot/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_REBOOT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/reboot/main.c

./o64/arctic/programs/reboot/operate.o: \
    ./arctic/programs/reboot/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_REBOOT_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/reboot/operate.c

./o64/arctic/programs/rm/main.o: \
    ./arctic/programs/rm/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RM_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/rm/main.c

./o64/arctic/programs/rm/operate.o: \
    ./arctic/programs/rm/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RM_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/rm/operate.c

./o64/arctic/programs/rmdir/main.o: \
    ./arctic/programs/rmdir/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RMDIR_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/rmdir/main.c

./o64/arctic/programs/rmdir/operate.o: \
    ./arctic/programs/rmdir/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_RMDIR_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/rmdir/operate.c

./o64/arctic/programs/sleep/main.o: \
    ./arctic/programs/sleep/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_SLEEP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/sleep/main.c

./o64/arctic/programs/sleep/operate.o: \
    ./arctic/programs/sleep/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_SLEEP_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/sleep/operate.c

./o64/arctic/programs/tee/main.o: \
    ./arctic/programs/tee/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TEE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/tee/main.c

./o64/arctic/programs/tee/operate.o: \
    ./arctic/programs/tee/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TEE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/tee/operate.c

./o64/arctic/programs/terminal/keymap.o: \
    ./arctic/programs/terminal/keymap.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/terminal/keymap.c

./o64/arctic/programs/terminal/main.o: \
    ./arctic/programs/terminal/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/terminal/main.c

./o64/arctic/programs/terminal/operate.o: \
    ./arctic/programs/terminal/operate.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TERMINAL_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/terminal/operate.c

./o64/arctic/programs/true/main.o: \
    ./arctic/programs/true/main.c $(DANCY_DEPS) \
    $(ARCTIC_PROGRAMS_TRUE_HEADERS)
	$(ARCTIC_O64)$@ ./arctic/programs/true/main.c

./o64/boot/init/a64/gdt.o: \
    ./boot/init/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/gdt.asm

./o64/boot/init/a64/idt.o: \
    ./boot/init/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/idt.asm

./o64/boot/init/a64/kernel.o: \
    ./boot/init/a64/kernel.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/kernel.asm

./o64/boot/init/a64/start.o: \
    ./boot/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/start.asm

./o64/boot/init/a64/syscall.o: \
    ./boot/init/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/syscall.asm

./o64/boot/init/acpi.o: \
    ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/acpi.c

./o64/boot/init/apic.o: \
    ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/apic.c

./o64/boot/init/cpu.o: \
    ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/cpu.c

./o64/boot/init/db.o: \
    ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/db.c

./o64/boot/init/delay.o: \
    ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/delay.c

./o64/boot/init/fs.o: \
    ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/fs.c

./o64/boot/init/gui.o: \
    ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/gui.c

./o64/boot/init/hpet.o: \
    ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/hpet.c

./o64/boot/init/idt.o: \
    ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/idt.c

./o64/boot/init/init.o: \
    ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/init.c

./o64/boot/init/kernel.o: \
    ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/kernel.c

./o64/boot/init/ld.o: \
    ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/ld.c

./o64/boot/init/log.o: \
    ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/log.c

./o64/boot/init/memory.o: \
    ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/memory.c

./o64/boot/init/panic.o: \
    ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/panic.c

./o64/boot/init/pci.o: \
    ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pci.c

./o64/boot/init/pg.o: \
    ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pg.c

./o64/boot/init/pit.o: \
    ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pit.c

./o64/boot/init/print.o: \
    ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/print.c

./o64/boot/init/rtc.o: \
    ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/rtc.c

./o64/boot/init/smp/smp.o: \
    ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/smp.c

./o64/boot/init/smp/start32.o: \
    ./boot/init/smp/start32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/start32.c

./o64/boot/init/smp/start64.o: \
    ./boot/init/smp/start64.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/start64.c

./o64/boot/init/start.o: \
    ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/start.c

./o64/boot/init/table.o: \
    ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/table.c

./o64/boot/init/usb.o: \
    ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/usb.c

./o64/boot/init/vga.o: \
    ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/vga.c

./o64/boot/init/video/_driver.o: \
    ./boot/init/video/_driver.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/video/_driver.c

./o64/boot/init/video/bochs.o: \
    ./boot/init/video/bochs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/video/bochs.c

./o64/boot/uefi/a64/cpu.o: \
    ./boot/uefi/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/cpu.asm

./o64/boot/uefi/a64/file.o: \
    ./boot/uefi/a64/file.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/file.asm

./o64/boot/uefi/a64/font7x9.o: \
    ./boot/uefi/a64/font7x9.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/font7x9.asm

./o64/boot/uefi/a64/memory.o: \
    ./boot/uefi/a64/memory.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/memory.asm

./o64/boot/uefi/a64/pic.o: \
    ./boot/uefi/a64/pic.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/pic.asm

./o64/boot/uefi/a64/start.o: \
    ./boot/uefi/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/start.asm

./o64/boot/uefi/a64/syscall.o: \
    ./boot/uefi/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/syscall.asm

./o64/boot/uefi/block.o: \
    ./boot/uefi/block.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/block.c

./o64/boot/uefi/file.o: \
    ./boot/uefi/file.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/file.c

./o64/boot/uefi/key.o: \
    ./boot/uefi/key.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/key.c

./o64/boot/uefi/log.o: \
    ./boot/uefi/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/log.c

./o64/boot/uefi/memory.o: \
    ./boot/uefi/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/memory.c

./o64/boot/uefi/misc.o: \
    ./boot/uefi/misc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/misc.c

./o64/boot/uefi/print.o: \
    ./boot/uefi/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/print.c

./o64/boot/uefi/serial.o: \
    ./boot/uefi/serial.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/serial.c

./o64/boot/uefi/syscall.o: \
    ./boot/uefi/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/syscall.c

./o64/boot/uefi/uefi.o: \
    ./boot/uefi/uefi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/uefi.c

./o64/boot/uefi/video.o: \
    ./boot/uefi/video.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/video.c

./o64/common/a64/dark.o: \
    ./common/a64/dark.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./common/a64/dark.asm

./o64/common/a64/string.o: \
    ./common/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./common/a64/string.asm

./o64/common/crc32.o: \
    ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32.c

./o64/common/crc32c.o: \
    ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32c.c

./o64/common/epoch.o: \
    ./common/epoch.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/epoch.c

./o64/common/fat.o: \
    ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/fat.c

./o64/common/stack.o: \
    ./common/stack.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/stack.c

./o64/common/ttf.o: \
    ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/ttf.c

./o64/common/utf8.o: \
    ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/utf8.c

./o64/kernel/base/a64/fb.o: \
    ./kernel/base/a64/fb.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/fb.asm

./o64/kernel/base/a64/gdt.o: \
    ./kernel/base/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/gdt.asm

./o64/kernel/base/a64/idt.o: \
    ./kernel/base/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/idt.asm

./o64/kernel/base/a64/task.o: \
    ./kernel/base/a64/task.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/task.asm

./o64/kernel/base/a64/timer.o: \
    ./kernel/base/a64/timer.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/timer.asm

./o64/kernel/base/apic.o: \
    ./kernel/base/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/apic.c

./o64/kernel/base/console.o: \
    ./kernel/base/console.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/console.c

./o64/kernel/base/delay.o: \
    ./kernel/base/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/delay.c

./o64/kernel/base/event.o: \
    ./kernel/base/event.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/event.c

./o64/kernel/base/fb.o: \
    ./kernel/base/fb.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/fb.c

./o64/kernel/base/gdt.o: \
    ./kernel/base/gdt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/gdt.c

./o64/kernel/base/heap.o: \
    ./kernel/base/heap.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/heap.c

./o64/kernel/base/idt.o: \
    ./kernel/base/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/idt.c

./o64/kernel/base/idt_user.o: \
    ./kernel/base/idt_user.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/idt_user.c

./o64/kernel/base/irq.o: \
    ./kernel/base/irq.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/irq.c

./o64/kernel/base/kmsg.o: \
    ./kernel/base/kmsg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/kmsg.c

./o64/kernel/base/mm.o: \
    ./kernel/base/mm.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/mm.c

./o64/kernel/base/mtx.o: \
    ./kernel/base/mtx.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/mtx.c

./o64/kernel/base/panic.o: \
    ./kernel/base/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/panic.c

./o64/kernel/base/pg.o: \
    ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/pg.c

./o64/kernel/base/ret_user.o: \
    ./kernel/base/ret_user.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/ret_user.c

./o64/kernel/base/runlevel.o: \
    ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/runlevel.c

./o64/kernel/base/spin.o: \
    ./kernel/base/spin.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/spin.c

./o64/kernel/base/start.o: \
    ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/start.c

./o64/kernel/base/task.o: \
    ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/task.c

./o64/kernel/debug/debug.o: \
    ./kernel/debug/debug.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/debug/debug.c

./o64/kernel/lib/a64/coff.o: \
    ./kernel/lib/a64/coff.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/coff.asm

./o64/kernel/lib/a64/cpu.o: \
    ./kernel/lib/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/cpu.asm

./o64/kernel/lib/a64/spin.o: \
    ./kernel/lib/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/spin.asm

./o64/kernel/lib/bitarray.o: \
    ./kernel/lib/bitarray.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/bitarray.c

./o64/kernel/lib/ctype.o: \
    ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/ctype.c

./o64/kernel/lib/huffman.o: \
    ./kernel/lib/huffman.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/huffman.c

./o64/kernel/lib/inflate.o: \
    ./kernel/lib/inflate.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/inflate.c

./o64/kernel/lib/snprintf.o: \
    ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/snprintf.c

./o64/kernel/lib/stdlib.o: \
    ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/stdlib.c

./o64/kernel/lib/strerror.o: \
    ./kernel/lib/strerror.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/strerror.c

./o64/kernel/lib/string.o: \
    ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/string.c

./o64/kernel/misc/bin.o: \
    ./kernel/misc/bin.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/bin.c

./o64/kernel/misc/coff.o: \
    ./kernel/misc/coff.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/coff.c

./o64/kernel/misc/console.o: \
    ./kernel/misc/console.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/console.c

./o64/kernel/misc/dma.o: \
    ./kernel/misc/dma.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/dma.c

./o64/kernel/misc/epoch.o: \
    ./kernel/misc/epoch.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/epoch.c

./o64/kernel/misc/fb_user.o: \
    ./kernel/misc/fb_user.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/fb_user.c

./o64/kernel/misc/floppy.o: \
    ./kernel/misc/floppy.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/floppy.c

./o64/kernel/misc/hdd_mnt.o: \
    ./kernel/misc/hdd_mnt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/hdd_mnt.c

./o64/kernel/misc/hdd_part.o: \
    ./kernel/misc/hdd_part.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/hdd_part.c

./o64/kernel/misc/keyboard.o: \
    ./kernel/misc/keyboard.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/keyboard.c

./o64/kernel/misc/kmsg.o: \
    ./kernel/misc/kmsg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/kmsg.c

./o64/kernel/misc/mouse.o: \
    ./kernel/misc/mouse.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/mouse.c

./o64/kernel/misc/rtc.o: \
    ./kernel/misc/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/rtc.c

./o64/kernel/misc/serial.o: \
    ./kernel/misc/serial.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/serial.c

./o64/kernel/misc/zero.o: \
    ./kernel/misc/zero.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/zero.c

./o64/kernel/network/e1000.o: \
    ./kernel/network/e1000.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/network/e1000.c

./o64/kernel/network/net_node.o: \
    ./kernel/network/net_node.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/network/net_node.c

./o64/kernel/pci/ahci.o: \
    ./kernel/pci/ahci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/pci/ahci.c

./o64/kernel/pci/ide_ctrl.o: \
    ./kernel/pci/ide_ctrl.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/pci/ide_ctrl.c

./o64/kernel/pci/pci.o: \
    ./kernel/pci/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/pci/pci.c

./o64/kernel/ps2/8042.o: \
    ./kernel/ps2/8042.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/8042.c

./o64/kernel/ps2/keyboard.o: \
    ./kernel/ps2/keyboard.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/keyboard.c

./o64/kernel/ps2/mouse.o: \
    ./kernel/ps2/mouse.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/mouse.c

./o64/kernel/run/run.o: \
    ./kernel/run/run.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/run/run.c

./o64/kernel/sched/sched.o: \
    ./kernel/sched/sched.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/sched/sched.c

./o64/kernel/syscall/a64/trap.o: \
    ./kernel/syscall/a64/trap.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/syscall/a64/trap.asm

./o64/kernel/syscall/arg.o: \
    ./kernel/syscall/arg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/arg.c

./o64/kernel/syscall/errno.o: \
    ./kernel/syscall/errno.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/errno.c

./o64/kernel/syscall/file.o: \
    ./kernel/syscall/file.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/file.c

./o64/kernel/syscall/ioctl.o: \
    ./kernel/syscall/ioctl.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/ioctl.c

./o64/kernel/syscall/kill.o: \
    ./kernel/syscall/kill.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/kill.c

./o64/kernel/syscall/misc.o: \
    ./kernel/syscall/misc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/misc.c

./o64/kernel/syscall/proc.o: \
    ./kernel/syscall/proc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/proc.c

./o64/kernel/syscall/reboot.o: \
    ./kernel/syscall/reboot.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/reboot.c

./o64/kernel/syscall/sleep.o: \
    ./kernel/syscall/sleep.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/sleep.c

./o64/kernel/syscall/spawn.o: \
    ./kernel/syscall/spawn.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/spawn.c

./o64/kernel/syscall/syscall.o: \
    ./kernel/syscall/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/syscall.c

./o64/kernel/usb/boot_kbd.o: \
    ./kernel/usb/boot_kbd.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/boot_kbd.c

./o64/kernel/usb/boot_mse.o: \
    ./kernel/usb/boot_mse.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/boot_mse.c

./o64/kernel/usb/ehci.o: \
    ./kernel/usb/ehci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/ehci.c

./o64/kernel/usb/ohci.o: \
    ./kernel/usb/ohci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/ohci.c

./o64/kernel/usb/uhci.o: \
    ./kernel/usb/uhci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/uhci.c

./o64/kernel/usb/usb_ep.o: \
    ./kernel/usb/usb_ep.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_ep.c

./o64/kernel/usb/usb_hid.o: \
    ./kernel/usb/usb_hid.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_hid.c

./o64/kernel/usb/usb_mnt.o: \
    ./kernel/usb/usb_mnt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_mnt.c

./o64/kernel/usb/usb_msc.o: \
    ./kernel/usb/usb_msc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_msc.c

./o64/kernel/usb/usb_node.o: \
    ./kernel/usb/usb_node.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_node.c

./o64/kernel/usb/usb_task.o: \
    ./kernel/usb/usb_task.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/usb_task.c

./o64/kernel/usb/xhci.o: \
    ./kernel/usb/xhci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/usb/xhci.c

./o64/kernel/vfs/default.o: \
    ./kernel/vfs/default.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/default.c

./o64/kernel/vfs/devfs.o: \
    ./kernel/vfs/devfs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/devfs.c

./o64/kernel/vfs/fat_io.o: \
    ./kernel/vfs/fat_io.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/fat_io.c

./o64/kernel/vfs/pipe.o: \
    ./kernel/vfs/pipe.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/pipe.c

./o64/kernel/vfs/pty.o: \
    ./kernel/vfs/pty.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/pty.c

./o64/kernel/vfs/root.o: \
    ./kernel/vfs/root.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/root.c

./o64/kernel/vfs/vfs.o: \
    ./kernel/vfs/vfs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/vfs.c
