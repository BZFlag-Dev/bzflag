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

DEPTH = ../..
include $(DEPTH)/Make-common

TARGETS  = $(LIBDIR)/libCommon.a

CXXFILES =				\
	AList.cxx			\
	BzfString.cxx			\
	ErrorHandler.cxx		\
	Flag.cxx			\
	Team.cxx			\
	TimeBomb.cxx			\
	TimeKeeper.cxx			\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?

