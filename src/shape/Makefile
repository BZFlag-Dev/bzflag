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

TARGETS  = $(LIBDIR)/libShape.a

CXXFILES =						\
	ContactSurfacePoint.cxx		\
	ContactSurfaceEdge.cxx		\
	ContactSurfacePolygon.cxx	\
	ShapeBox.cxx				\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?

