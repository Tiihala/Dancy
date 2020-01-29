# Dancy Operating System

##############################################################################

./scripts/dancy.mk:
	@mkdir "bin"
	@mkdir "efi"
	@mkdir "efi/boot"
	@mkdir "o32"
	@mkdir "o32/acpica"
	@mkdir "o32/acpios"
	@mkdir "o32/acpios/a32"
	@mkdir "o32/common"
	@mkdir "o32/init"
	@mkdir "o32/init/a32"
	@mkdir "o32/lib"
	@mkdir "o32/lib/a32"
	@mkdir "o64"
	@mkdir "o64/acpica"
	@mkdir "o64/acpios"
	@mkdir "o64/acpios/a64"
	@mkdir "o64/common"
	@mkdir "o64/init"
	@mkdir "o64/init/a64"
	@mkdir "o64/lib"
	@mkdir "o64/lib/a64"
	@mkdir "o64/uefi"
	@mkdir "o64/uefi/a64"
	@mkdir "release"
	@mkdir "system"
	$(DANCY_MK)
