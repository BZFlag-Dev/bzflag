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

TARGETS = $(TARGETDIR)/bzgview

CXXFILES =				\
	Trackball.cxx			\
	bzgview.cxx			\
	$(NULL)

LCXXINCS = -I.

DEPLIBS =				\
	$(LIBDIR)/libMPlatform.a	\
	$(LIBDIR)/libScene.a		\
	$(LIBDIR)/libGLKit.a		\
	$(LIBDIR)/libMediaFile.a	\
	$(LIBDIR)/libGeometry.a		\
	$(LIBDIR)/libPlatform.a		\
	$(LIBDIR)/libNet.a		\
	$(LIBDIR)/libCommon.a		\
	$(LIBDIR)/libz.a		\
	$(NULL)

LLDLIBS =				\
	$(DEPLIBS)			\
	$(IMAGE_LIBS)			\
	$(GL_LIBS)			\
	$(DISPLAY_LIBS)			\
	$(WINDOW_LIBS)			\
	-lpthread			\
	-ldl				\
	-lm				\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS) $(DEPLIBS)
	$(CXXF) -o $@ $(OBJECTS) $(LDFLAGS)

