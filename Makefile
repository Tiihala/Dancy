# Dancy Operating System
# Copyright (c) 2018 Antti Tiihala

# This is a Makefile that can be run on other operating systems but
# exact details of the requirements are not specified here. All the
# modern Unix-like systems are most likely supported and can be used
# for bootstrapping the Dancy Operating System. In short, the common
# denominator is the C programming language and a "make-like" build
# system. As much as practically possible, the ANSI C (also known as
# "C89") is strictly followed for achieving good compatibility with
# the other platforms, operating systems, and tools.

# Host-system specific object files have an .obj extension. The file
# extension .o is reserved for the native object files that may not
# even use the same instruction encoding as the host system.

# There are two main Makefile branches. Some implementations handle
# comment lines so that if the line ended with a backslash, it would
# not interpret the next line as a comment line. This little nuance
# is used for detecting the Unix-like implementations.

# Unix-like Makefile implementations \
!if 0
include ./scripts/unix.mk
# \
!endif

# Other Makefile implementations \
!include ./scripts/other.mk
