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

TARGETS  = $(LIBDIR)/libCollide.a

CXXFILES =				\
	Body.cxx			\
	BodyManager.cxx			\
	BodyODEAssistant.cxx		\
	CollisionDetectorGJK.cxx	\
	ContactPoint.cxx		\
	ContactSurfaceIntersector.cxx	\
	ContactSurfaceIntersectorPolygon.cxx	\
	ContactSolverBaraffNoFriction.cxx	\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?


DEPLIBS =				\
	$(LIBDIR)/libCollide.a		\
	$(LIBDIR)/libShape.a		\
	$(LIBDIR)/libMPlatform.a	\
	$(LIBDIR)/libScene.a		\
	$(LIBDIR)/libGLKit.a		\
	$(LIBDIR)/libMediaFile.a	\
	$(LIBDIR)/libNet.a		\
	$(LIBDIR)/libz.a		\
	$(LIBDIR)/libGeometry.a		\
	$(LIBDIR)/libPlatform.a		\
	$(LIBDIR)/libMath.a		\
	$(LIBDIR)/libCommon.a		\
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

test1: test1.o $(DEPLIBS)
	$(CXXF) -o $@ test1.o $(LDFLAGS)

test2: test2.o Trackball.o $(DEPLIBS)
	$(CXXF) -o $@ test2.o Trackball.o $(LDFLAGS)

test3: test3.o Trackball2.o $(DEPLIBS)
	$(CXXF) -o $@ test3.o Trackball2.o -lglut $(LDFLAGS)
