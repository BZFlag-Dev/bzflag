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
	CommandsSearch.cxx		\
					\
	SceneManager.cxx		\
	HUDManager.cxx			\
					\
	ViewItems.cxx			\
	ViewItemHUD.cxx			\
	ViewItemRadar.cxx		\
	ViewItemPlayerScene.cxx		\
	ViewItemScoreboard.cxx		\
					\
	World.cxx			\
	SceneBuilder.cxx		\
	ShotPath.cxx			\
	ShotStrategy.cxx		\
	Region.cxx			\
	Player.cxx			\
	playing.cxx			\
	LocalPlayer.cxx			\
	RemotePlayer.cxx		\
	RobotPlayer.cxx			\
	DeadPlayer.cxx			\
	ServerLink.cxx			\
	PlayerLink.cxx			\
	daylight.cxx			\
	sound.cxx			\
	bzflag.cxx			\
	$(NULL)

LCXXINCS =				\
	-I$(DEPTH)/src/game		\
	$(NULL)

DEPLIBS =				\
	$(LIBDIR)/libGame.a		\
	$(LIBDIR)/libMPlatform.a	\
	$(LIBDIR)/libView.a		\
	$(LIBDIR)/libUI.a		\
	$(LIBDIR)/libScene.a		\
	$(LIBDIR)/libGLKit.a		\
	$(LIBDIR)/libPlatform.a		\
	$(LIBDIR)/libNet.a		\
	$(LIBDIR)/libMediaFile.a	\
	$(LIBDIR)/libObstacle.a		\
	$(LIBDIR)/libGeometry.a		\
	$(LIBDIR)/libCommon.a		\
	$(NULL)

LLDLIBS =				\
	$(DEPLIBS)			\
	$(AUDIO_LIBS)			\
	$(IMAGE_LIBS)			\
	$(NETWORK_LIBS)			\
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

