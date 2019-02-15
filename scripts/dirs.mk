# Dancy Operating System

##############################################################################

./scripts/dancy.mk:
	@mkdir "bin"
	@mkdir "o32"
	@mkdir "o32/bitarray"
	@mkdir "o32/huffman"
	@mkdir "o32/init"
	@mkdir "o32/init/a32"
	@mkdir "o32/lib"
	@mkdir "o32/lib/a32"
	@mkdir "o64"
	@mkdir "o64/bitarray"
	@mkdir "o64/huffman"
	@mkdir "o64/init"
	@mkdir "o64/init/a64"
	@mkdir "o64/lib"
	@mkdir "o64/lib/a64"
	@mkdir "release"
	@mkdir "system"
	$(DANCY_MK)
