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

TARGETS  = $(LIBDIR)/libScene.a

CXXFILES =				\
	SceneNode.cxx			\
	SceneNodeAnimate.cxx		\
	SceneNodeBillboard.cxx		\
	SceneNodeBaseTransform.cxx	\
	SceneNodeChoice.cxx		\
	SceneNodeGState.cxx		\
	SceneNodeGeometry.cxx		\
	SceneNodeGroup.cxx		\
	SceneNodeLight.cxx		\
	SceneNodeLOD.cxx		\
	SceneNodeMatrixTransform.cxx	\
	SceneNodeMetadata.cxx		\
	SceneNodeParameters.cxx		\
	SceneNodePrimitive.cxx		\
	SceneNodeSelector.cxx		\
	SceneNodeSphereTransform.cxx	\
	SceneNodeTransform.cxx		\
	SceneVisitor.cxx		\
	SceneVisitorParams.cxx		\
	SceneVisitorFind.cxx		\
	SceneVisitorFindAll.cxx		\
	SceneVisitorRender.cxx		\
	SceneVisitorSimpleRender.cxx	\
	SceneReader.cxx 		\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS)
	$(ARF) $@ $?
