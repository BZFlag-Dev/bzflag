# bzflag
# Copyright (c) 1993 - 2003 Tim Riker
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
	ErrorHandler.cxx		\
	Flag.cxx			\
	Team.cxx			\
	TimeBomb.cxx			\
	TimeKeeper.cxx			\
	md5.cxx				\
	$(NULL)

CFILES =				\
	bzsignal.c			\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?

