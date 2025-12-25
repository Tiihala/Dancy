# Dancy Operating System

##############################################################################

ARCTIC_ROOT_FILES= \
 ./arctic/include/__dancy/core.h \
 ./arctic/include/__dancy/ioctl.h \
 ./arctic/include/__dancy/keys.h \
 ./arctic/include/__dancy/mman.h \
 ./arctic/include/__dancy/mode.h \
 ./arctic/include/__dancy/pollfd.h \
 ./arctic/include/__dancy/proc.h \
 ./arctic/include/__dancy/reboot.h \
 ./arctic/include/__dancy/sched.h \
 ./arctic/include/__dancy/seek.h \
 ./arctic/include/__dancy/sig.h \
 ./arctic/include/__dancy/signum.h \
 ./arctic/include/__dancy/spawn.h \
 ./arctic/include/__dancy/spin.h \
 ./arctic/include/__dancy/ssize.h \
 ./arctic/include/__dancy/stat.h \
 ./arctic/include/__dancy/syscall.h \
 ./arctic/include/__dancy/termios.h \
 ./arctic/include/__dancy/timedef.h \
 ./arctic/include/__dancy/timespec.h \
 ./arctic/include/__dancy/timeval.h \
 ./arctic/include/__dancy/tm.h \
 ./arctic/include/__dancy/undef.h \
 ./arctic/include/assert.h \
 ./arctic/include/complex.h \
 ./arctic/include/ctype.h \
 ./arctic/include/dirent.h \
 ./arctic/include/err.h \
 ./arctic/include/errno.h \
 ./arctic/include/fcntl.h \
 ./arctic/include/fenv.h \
 ./arctic/include/glob.h \
 ./arctic/include/inttypes.h \
 ./arctic/include/locale.h \
 ./arctic/include/math.h \
 ./arctic/include/poll.h \
 ./arctic/include/pty.h \
 ./arctic/include/pwd.h \
 ./arctic/include/regex.h \
 ./arctic/include/setjmp.h \
 ./arctic/include/signal.h \
 ./arctic/include/spawn.h \
 ./arctic/include/stdio.h \
 ./arctic/include/stdlib.h \
 ./arctic/include/string.h \
 ./arctic/include/strings.h \
 ./arctic/include/sys/ioctl.h \
 ./arctic/include/sys/mman.h \
 ./arctic/include/sys/select.h \
 ./arctic/include/sys/stat.h \
 ./arctic/include/sys/types.h \
 ./arctic/include/sys/wait.h \
 ./arctic/include/termios.h \
 ./arctic/include/tgmath.h \
 ./arctic/include/threads.h \
 ./arctic/include/time.h \
 ./arctic/include/unistd.h \
 ./arctic/include/wchar.h \
 ./arctic/include/wctype.h \
 ./share/fonts/dcysan.ttf \
 ./share/fonts/dcysanb.ttf \
 ./share/fonts/dcysanm.ttf \

##############################################################################

./arctic/root.img: $(ARCTIC_ROOT_FILES) $(DANCY_TARGET_TOOLS)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/core.h \
		::include/__dancy/core.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/ioctl.h \
		::include/__dancy/ioctl.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/keys.h \
		::include/__dancy/keys.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/mman.h \
		::include/__dancy/mman.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/mode.h \
		::include/__dancy/mode.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/pollfd.h \
		::include/__dancy/pollfd.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/proc.h \
		::include/__dancy/proc.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/reboot.h \
		::include/__dancy/reboot.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/sched.h \
		::include/__dancy/sched.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/seek.h \
		::include/__dancy/seek.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/sig.h \
		::include/__dancy/sig.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/signum.h \
		::include/__dancy/signum.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/spawn.h \
		::include/__dancy/spawn.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/spin.h \
		::include/__dancy/spin.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/ssize.h \
		::include/__dancy/ssize.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/stat.h \
		::include/__dancy/stat.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/syscall.h \
		::include/__dancy/syscall.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/termios.h \
		::include/__dancy/termios.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/timedef.h \
		::include/__dancy/timedef.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/timespec.h \
		::include/__dancy/timespec.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/timeval.h \
		::include/__dancy/timeval.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/tm.h \
		::include/__dancy/tm.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/__dancy/undef.h \
		::include/__dancy/undef.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/assert.h \
		::include/assert.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/complex.h \
		::include/complex.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/ctype.h \
		::include/ctype.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/dirent.h \
		::include/dirent.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/err.h \
		::include/err.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/errno.h \
		::include/errno.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/fcntl.h \
		::include/fcntl.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/fenv.h \
		::include/fenv.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/glob.h \
		::include/glob.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/inttypes.h \
		::include/inttypes.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/locale.h \
		::include/locale.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/math.h \
		::include/math.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/poll.h \
		::include/poll.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/pty.h \
		::include/pty.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/pwd.h \
		::include/pwd.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/regex.h \
		::include/regex.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/setjmp.h \
		::include/setjmp.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/signal.h \
		::include/signal.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/spawn.h \
		::include/spawn.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/stdio.h \
		::include/stdio.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/stdlib.h \
		::include/stdlib.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/string.h \
		::include/string.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/strings.h \
		::include/strings.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/ioctl.h \
		::include/sys/ioctl.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/mman.h \
		::include/sys/mman.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/select.h \
		::include/sys/select.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/stat.h \
		::include/sys/stat.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/types.h \
		::include/sys/types.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/sys/wait.h \
		::include/sys/wait.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/termios.h \
		::include/termios.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/tgmath.h \
		::include/tgmath.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/threads.h \
		::include/threads.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/time.h \
		::include/time.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/unistd.h \
		::include/unistd.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/wchar.h \
		::include/wchar.h
	$(DY_MCOPY) -i $@ \
		./arctic/include/wctype.h \
		::include/wctype.h
	$(DY_MCOPY) -i $@ \
		./share/fonts/dcysan.ttf \
		::share/fonts/dcysan.ttf
	$(DY_MCOPY) -i $@ \
		./share/fonts/dcysanb.ttf \
		::share/fonts/dcysanb.ttf
	$(DY_MCOPY) -i $@ \
		./share/fonts/dcysanm.ttf \
		::share/fonts/dcysanm.ttf
