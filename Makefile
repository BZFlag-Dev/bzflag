# bzflag
# Copyright 1993-1999, Chris Schoeneman
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

default targets clean clobber pristine man: config-sys _force
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
	solaris		\
	$(NULL)

config-sys:
	@echo "No configuration.  Use one of the following:"
	@echo "  make irix-mips2"
	@echo "  make irix-mips3"
	@echo "  make linux"
	@echo "  make linux-i386"
	@echo "  make linux-ppc"
	@echo "  make solaris"
	@echo "  make win32"
	@echo "Append -debug for a debug build (e.g. make linux-debug)."
	@exit 1

$(AVAILTARGETS): _force
	@cd configs; $(MAKE) $(MFLAGS) PLATFORM=$@ default
	@echo "Configured.  Following targets available:"
	@echo "  make           build programs"
	@echo "  make man       build man pages"
	@echo "  make all       build all the above"
	@echo "  make package   build an installable package from the above"
	@echo "  make clean     remove intermediate files"
	@echo "  make clobber   remove everything built by make all"
	@echo "  make pristine  remove everything except original files"

$(AVAILTARGETS:=-debug): _force
	@cd configs; $(MAKE) $(MFLAGS) PLATFORM=$(@:-debug=) debug
	@echo "Configured.  Following targets available:"
	@echo "  make           build programs"
	@echo "  make man       build man pages"
	@echo "  make all       build all the above"
	@echo "  make package   build an installable package from the above"
	@echo "  make clean     remove intermediate files"
	@echo "  make clobber   remove everything built by make all"
	@echo "  make pristine  remove everything except original files"

win32 win32-debug: _force
	@echo "win32 makefiles not available yet.  use the msdev"
	@echo "workspace and project files in the win32 directory."
	@echo "see README.WIN32 for build instructions."

