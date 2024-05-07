# Dancy Operating System

##############################################################################

./scripts/dancy.mk:
	@mkdir "arctic/bin32"
	@mkdir "arctic/bin64"
	@mkdir "bin"
	@mkdir "efi"
	@mkdir "efi/boot"
	@mkdir "o32"
	@mkdir "o32/arctic"
	@mkdir "o32/arctic/apps"
	@mkdir "o32/arctic/apps/hello"
	@mkdir "o32/arctic/libc"
	@mkdir "o32/arctic/libc/a32"
	@mkdir "o32/arctic/libc/ctype"
	@mkdir "o32/arctic/libc/dirent"
	@mkdir "o32/arctic/libc/errno"
	@mkdir "o32/arctic/libc/fcntl"
	@mkdir "o32/arctic/libc/keymap"
	@mkdir "o32/arctic/libc/locale"
	@mkdir "o32/arctic/libc/misc"
	@mkdir "o32/arctic/libc/poll"
	@mkdir "o32/arctic/libc/pty"
	@mkdir "o32/arctic/libc/signal"
	@mkdir "o32/arctic/libc/spawn"
	@mkdir "o32/arctic/libc/stdio"
	@mkdir "o32/arctic/libc/stdlib"
	@mkdir "o32/arctic/libc/string"
	@mkdir "o32/arctic/libc/sys"
	@mkdir "o32/arctic/libc/termios"
	@mkdir "o32/arctic/libc/threads"
	@mkdir "o32/arctic/libc/time"
	@mkdir "o32/arctic/libc/unistd"
	@mkdir "o32/arctic/programs"
	@mkdir "o32/arctic/programs/cat"
	@mkdir "o32/arctic/programs/date"
	@mkdir "o32/arctic/programs/dsh"
	@mkdir "o32/arctic/programs/false"
	@mkdir "o32/arctic/programs/hd"
	@mkdir "o32/arctic/programs/hexdump"
	@mkdir "o32/arctic/programs/init"
	@mkdir "o32/arctic/programs/ls"
	@mkdir "o32/arctic/programs/more"
	@mkdir "o32/arctic/programs/poweroff"
	@mkdir "o32/arctic/programs/ps"
	@mkdir "o32/arctic/programs/reboot"
	@mkdir "o32/arctic/programs/sleep"
	@mkdir "o32/arctic/programs/terminal"
	@mkdir "o32/arctic/programs/true"
	@mkdir "o32/boot"
	@mkdir "o32/boot/init"
	@mkdir "o32/boot/init/a32"
	@mkdir "o32/boot/init/smp"
	@mkdir "o32/common"
	@mkdir "o32/common/a32"
	@mkdir "o32/kernel"
	@mkdir "o32/kernel/acpica"
	@mkdir "o32/kernel/acpios"
	@mkdir "o32/kernel/acpios/a32"
	@mkdir "o32/kernel/base"
	@mkdir "o32/kernel/base/a32"
	@mkdir "o32/kernel/debug"
	@mkdir "o32/kernel/lib"
	@mkdir "o32/kernel/lib/a32"
	@mkdir "o32/kernel/misc"
	@mkdir "o32/kernel/pci"
	@mkdir "o32/kernel/ps2"
	@mkdir "o32/kernel/run"
	@mkdir "o32/kernel/sched"
	@mkdir "o32/kernel/syscall"
	@mkdir "o32/kernel/syscall/a32"
	@mkdir "o32/kernel/vfs"
	@mkdir "o64"
	@mkdir "o64/arctic"
	@mkdir "o64/arctic/apps"
	@mkdir "o64/arctic/apps/hello"
	@mkdir "o64/arctic/libc"
	@mkdir "o64/arctic/libc/a64"
	@mkdir "o64/arctic/libc/ctype"
	@mkdir "o64/arctic/libc/dirent"
	@mkdir "o64/arctic/libc/errno"
	@mkdir "o64/arctic/libc/fcntl"
	@mkdir "o64/arctic/libc/keymap"
	@mkdir "o64/arctic/libc/locale"
	@mkdir "o64/arctic/libc/misc"
	@mkdir "o64/arctic/libc/poll"
	@mkdir "o64/arctic/libc/pty"
	@mkdir "o64/arctic/libc/signal"
	@mkdir "o64/arctic/libc/spawn"
	@mkdir "o64/arctic/libc/stdio"
	@mkdir "o64/arctic/libc/stdlib"
	@mkdir "o64/arctic/libc/string"
	@mkdir "o64/arctic/libc/sys"
	@mkdir "o64/arctic/libc/termios"
	@mkdir "o64/arctic/libc/threads"
	@mkdir "o64/arctic/libc/time"
	@mkdir "o64/arctic/libc/unistd"
	@mkdir "o64/arctic/programs"
	@mkdir "o64/arctic/programs/cat"
	@mkdir "o64/arctic/programs/date"
	@mkdir "o64/arctic/programs/dsh"
	@mkdir "o64/arctic/programs/false"
	@mkdir "o64/arctic/programs/hd"
	@mkdir "o64/arctic/programs/hexdump"
	@mkdir "o64/arctic/programs/init"
	@mkdir "o64/arctic/programs/ls"
	@mkdir "o64/arctic/programs/more"
	@mkdir "o64/arctic/programs/poweroff"
	@mkdir "o64/arctic/programs/ps"
	@mkdir "o64/arctic/programs/reboot"
	@mkdir "o64/arctic/programs/sleep"
	@mkdir "o64/arctic/programs/terminal"
	@mkdir "o64/arctic/programs/true"
	@mkdir "o64/boot"
	@mkdir "o64/boot/init"
	@mkdir "o64/boot/init/a64"
	@mkdir "o64/boot/init/smp"
	@mkdir "o64/boot/uefi"
	@mkdir "o64/boot/uefi/a64"
	@mkdir "o64/common"
	@mkdir "o64/common/a64"
	@mkdir "o64/kernel"
	@mkdir "o64/kernel/acpica"
	@mkdir "o64/kernel/acpios"
	@mkdir "o64/kernel/acpios/a64"
	@mkdir "o64/kernel/base"
	@mkdir "o64/kernel/base/a64"
	@mkdir "o64/kernel/debug"
	@mkdir "o64/kernel/lib"
	@mkdir "o64/kernel/lib/a64"
	@mkdir "o64/kernel/misc"
	@mkdir "o64/kernel/pci"
	@mkdir "o64/kernel/ps2"
	@mkdir "o64/kernel/run"
	@mkdir "o64/kernel/sched"
	@mkdir "o64/kernel/syscall"
	@mkdir "o64/kernel/syscall/a64"
	@mkdir "o64/kernel/vfs"
	@mkdir "release"
	@mkdir "system"
	$(DANCY_MK)
