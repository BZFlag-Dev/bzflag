# bzflag
# Copyright (c) 1993 - 2002 Tim Riker
#
# This package is free software;  you can redistribute it and/or
# modify it under the terms of the license found in the file
# named LICENSE that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# top level makefile that simply ensures that there's a configuration chosen
#

DEPTH = .

default targets clean clobber pristine man tarball: config-sys _force
	$(MAKE) -f Make-sys $@

all: targets man

package: all
	$(MAKE) -f Make-sys $@

_force:

AVAILTARGETS =		\
	irix-mips2	\
	irix-mips3	\
	linux		\
	linux-i386	\
	linux-ppc	\
	solaris-cc	\
	solaris-gcc	\
	$(NULL)

config-sys:
	@echo "No configuration.  Use one of the following:"
	@echo "  make irix-mips2"
	@echo "  make irix-mips3"
	@echo "  make linux"
	@echo "  make linux-i386"
	@echo "  make linux-ppc"
	@echo "  make solaris-cc"
	@echo "  make solaris-gcc"
	@echo "  make win32"
	@echo "Append -debug for a debug build (e.g. make linux-debug)."
	@echo "Append -noopt for a non-optimized build (e.g. make linux-noopt)."
	@exit 1

configmsg:
	@echo "Configured.  Following targets available:"
	@echo "  make           build programs"
	@echo "  make man       build man pages"
	@echo "  make all       build all the above"
	@echo "  make package   build an installable package from the above"
	@echo "  make clean     remove intermediate files"
	@echo "  make clobber   remove everything built by make all"
	@echo "  make pristine  remove everything except original files"
	@echo "  make tarball   makes pristine then creates source tarball"

$(AVAILTARGETS): _force
	@cd configs; $(MAKE) $(MFLAGS) PLATFORM=$@ default
	@$(MAKE) $(MFLAGS) configmsg

$(AVAILTARGETS:=-debug): _force
	@cd configs; $(MAKE) $(MFLAGS) PLATFORM=$(@:-debug=) debug
	@$(MAKE) $(MFLAGS) configmsg

$(AVAILTARGETS:=-noopt): _force
	@cd configs; $(MAKE) $(MFLAGS) PLATFORM=$(@:-noopt=) noopt
	@$(MAKE) $(MFLAGS) configmsg

win32 win32-debug win32-noopt: _force
	@echo "win32 makefiles not available yet.  use the msdev"
	@echo "workspace and project files in the win32 directory."
	@echo "see README.WIN32 for build instructions."
