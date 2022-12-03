# Dancy Operating System

##############################################################################

./scripts/dancy.mk:
	@mkdir "arctic/bin32"
	@mkdir "arctic/bin64"
	@mkdir "bin"
	@mkdir "efi"
	@mkdir "efi/boot"
	@mkdir "o32"
	@mkdir "o32/_arctic"
	@mkdir "o32/_arctic/hello"
	@mkdir "o32/_arctic/libc"
	@mkdir "o32/_arctic/libc/a32"
	@mkdir "o32/_arctic/libc/ctype"
	@mkdir "o32/_arctic/libc/stdio"
	@mkdir "o32/_arctic/libc/stdlib"
	@mkdir "o32/_arctic/libc/string"
	@mkdir "o32/_boot"
	@mkdir "o32/_boot/init"
	@mkdir "o32/_boot/init/a32"
	@mkdir "o32/_boot/init/smp"
	@mkdir "o32/_common"
	@mkdir "o32/_common/a32"
	@mkdir "o32/acpica"
	@mkdir "o32/acpios"
	@mkdir "o32/acpios/a32"
	@mkdir "o32/base"
	@mkdir "o32/base/a32"
	@mkdir "o32/debug"
	@mkdir "o32/lib"
	@mkdir "o32/lib/a32"
	@mkdir "o32/misc"
	@mkdir "o32/pci"
	@mkdir "o32/ps2"
	@mkdir "o32/sched"
	@mkdir "o32/syscall"
	@mkdir "o32/syscall/a32"
	@mkdir "o32/vfs"
	@mkdir "o64"
	@mkdir "o64/_arctic"
	@mkdir "o64/_arctic/hello"
	@mkdir "o64/_arctic/libc"
	@mkdir "o64/_arctic/libc/a64"
	@mkdir "o64/_arctic/libc/ctype"
	@mkdir "o64/_arctic/libc/stdio"
	@mkdir "o64/_arctic/libc/stdlib"
	@mkdir "o64/_arctic/libc/string"
	@mkdir "o64/_boot"
	@mkdir "o64/_boot/init"
	@mkdir "o64/_boot/init/a64"
	@mkdir "o64/_boot/init/smp"
	@mkdir "o64/_boot/uefi"
	@mkdir "o64/_boot/uefi/a64"
	@mkdir "o64/_common"
	@mkdir "o64/_common/a64"
	@mkdir "o64/acpica"
	@mkdir "o64/acpios"
	@mkdir "o64/acpios/a64"
	@mkdir "o64/base"
	@mkdir "o64/base/a64"
	@mkdir "o64/debug"
	@mkdir "o64/lib"
	@mkdir "o64/lib/a64"
	@mkdir "o64/misc"
	@mkdir "o64/pci"
	@mkdir "o64/ps2"
	@mkdir "o64/sched"
	@mkdir "o64/syscall"
	@mkdir "o64/syscall/a64"
	@mkdir "o64/vfs"
	@mkdir "release"
	@mkdir "system"
	$(DANCY_MK)
