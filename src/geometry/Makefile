# bzflag
# Copyright (c) 1993 - 2001 Tim Riker
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

TARGETS  = $(LIBDIR)/libGeometry.a

CXXFILES =				\
	ViewFrustum.cxx			\
	Ray.cxx				\
	SceneNode.cxx			\
	WallSceneNode.cxx		\
	QuadWallSceneNode.cxx		\
	TriWallSceneNode.cxx		\
	PolyWallSceneNode.cxx		\
	EighthDimSceneNode.cxx		\
	EighthDBoxSceneNode.cxx		\
	EighthDPyrSceneNode.cxx		\
	ShellSceneNode.cxx		\
	TracerSceneNode.cxx		\
	PTSceneNode.cxx			\
	BoltSceneNode.cxx		\
	LaserSceneNode.cxx		\
	SphereSceneNode.cxx		\
	FlagSceneNode.cxx		\
	FlagWarpSceneNode.cxx		\
	TankSceneNode.cxx		\
	BillboardSceneNode.cxx		\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?

