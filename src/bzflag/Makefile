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

TARGETS = $(TARGETDIR)/bzflag

CXXFILES =				\
	MainWindow.cxx			\
	ControlPanel.cxx		\
	World.cxx			\
	SceneBuilder.cxx		\
	SceneRenderer.cxx		\
	BackgroundRenderer.cxx		\
	RadarRenderer.cxx		\
	HUDRenderer.cxx			\
	HUDui.cxx			\
	ShotPath.cxx			\
	ShotStrategy.cxx		\
	Region.cxx			\
	RenderNode.cxx			\
	Player.cxx			\
	playing.cxx			\
	LocalPlayer.cxx			\
	RemotePlayer.cxx		\
	RobotPlayer.cxx			\
	DeadPlayer.cxx			\
	ServerLink.cxx			\
	PlayerLink.cxx			\
	KeyMap.cxx			\
	daylight.cxx			\
	texture.cxx			\
	resources.cxx			\
	sound.cxx			\
	bzflag.cxx			\
	HUDDialog.cxx			\
	menus.cxx			\
	$(NULL)

DEPLIBS =				\
	$(LIBDIR)/libNet.a		\
	$(LIBDIR)/libCommon.a		\
	$(LIBDIR)/libObstacle.a		\
	$(LIBDIR)/libPlatform.a		\
	$(LIBDIR)/libGLKit.a		\
	$(LIBDIR)/libSceneDB.a		\
	$(LIBDIR)/libGeometry.a		\
	$(NULL)

LLDLIBS =				\
	$(DEPLIBS)			\
	$(AUDIO_LIBS)			\
	$(IMAGE_LIBS)			\
	$(NETWORK_LIBS)			\
	$(GL_LIBS)			\
	$(DISPLAY_LIBS)			\
	$(WINDOW_LIBS)			\
	-lm				\
	$(NULL)

targets: $(TARGETS)

$(TARGETS): $(OBJECTS) $(DEPLIBS)
	$(CXXF) -o $@ $(OBJECTS) $(LDFLAGS)

