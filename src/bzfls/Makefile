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

DEPTH = ../..
include $(DEPTH)/Make-common

TARGETS = $(TARGETDIR)/bzfls

CXXFILES =				\
	bzfls.cxx			\
	$(NULL)

LCXXINCS =				\
	-I$(DEPTH)/src/game		\
	$(NULL)

DEPLIBS =				\
	$(LIBDIR)/libGame.a		\
	$(LIBDIR)/libPlatform.a		\
	$(LIBDIR)/libNet.a		\
	$(LIBDIR)/libCommon.a		\
	$(NULL)

LLDLIBS =				\
	$(DEPLIBS)			\
	-lm				\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS) $(DEPLIBS)
	$(CXXF) -o $@ $(OBJECTS) $(LDFLAGS)

