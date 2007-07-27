# NOTE: This Makefile.am is hideous!  We link with a bunch of files found
# in ../bzflag.  These files will be compiled to .o files in the current
# directory.  Most of these files are only linked to because the linker
# needs the symbols somewhere.  Several of the files should have all of the
# graphics stuff taken out and moved to ../game.  There's a ton of work
# ahead for this to happen.  In the meantime, this seems to be the least
# ugly of all of the solutions.

bin_PROGRAMS = bzrobots

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I../bzflag \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	@X_CFLAGS@ \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

EXTRA_bzrobots_SOURCES = \
	../bzflag/ClientAuthentication.cxx	\
	../bzflag/ClientAuthentication.h

genericfiles =				\
	bzrobots.cxx			\
	botplaying.cxx			\
	RCLink.cxx			\
	RCLink.h			\
	RCMessage.h			\
	RCRequest.h			\
	RCRequests.cxx			\
	RCRequests.h			\
	RCReply.h			\
	RCReplies.cxx			\
	RCReplies.h			\
	RCEvent.h			\
	RCEvents.h			\
	RCEvents.cxx			\
	MessageUtilities.cxx		\
	MessageUtilities.h		\
	RCMessageFactory.h		\
	RCMessageFactory.cxx		\
	RCLinkBackend.cxx		\
	RCLinkBackend.h			\
	RCLinkFrontend.cxx		\
	RCLinkFrontend.h		\
	RCRobotPlayer.cxx		\
	RCRobotPlayer.h			\
	Frontend.cxx			\
	Frontend.h			\
	Logger.h			\
	Logger.cxx			\
	ScriptLoader.h			\
	ScriptLoaderFactory.cxx		\
	ScriptLoaderFactory.h		\
	SharedObjectLoader.cxx		\
	SharedObjectLoader.h		\
	BZAdvancedRobot.cxx		\
	BZAdvancedRobot.h		\
	AdvancedRobot.cxx		\
	AdvancedRobot.h			\
	BZRobot.cxx			\
	BZRobot.h			\
	Robot.cxx			\
	Robot.h				\
	../bzflag/ActionBinding.cxx		\
	../bzflag/ActionBinding.h		\
	../bzflag/AudioMenu.cxx			\
	../bzflag/AudioMenu.h			\
	../bzflag/AutoHunt.cxx			\
	../bzflag/AutoHunt.h			\
	../bzflag/AutoPilot.cxx			\
	../bzflag/AutoPilot.h			\
	../bzflag/BackgroundRenderer.cxx	\
	../bzflag/BackgroundRenderer.h		\
	../bzflag/BaseLocalPlayer.cxx		\
	../bzflag/BaseLocalPlayer.h		\
	../bzflag/CacheMenu.cxx			\
	../bzflag/CacheMenu.h			\
	../bzflag/CommandsImplementation.cxx	\
	../bzflag/ComposeDefaultKey.cxx		\
	../bzflag/ComposeDefaultKey.h		\
	../bzflag/ControlPanel.cxx		\
	../bzflag/ControlPanel.h		\
	../bzflag/CrackedGlass.cxx		\
	../bzflag/CrackedGlass.h		\
	../bzflag/DisplayMenu.cxx		\
	../bzflag/DisplayMenu.h			\
	../bzflag/Downloads.cxx			\
	../bzflag/Downloads.h			\
	../bzflag/EffectsMenu.cxx		\
	../bzflag/EffectsMenu.h			\
	../bzflag/EntryZone.cxx			\
	../bzflag/EntryZone.h			\
	../bzflag/FlashClock.cxx		\
	../bzflag/FlashClock.h			\
	../bzflag/FontSizer.cxx			\
	../bzflag/FontSizer.h			\
	../bzflag/ForceFeedback.cxx		\
	../bzflag/ForceFeedback.h		\
	../bzflag/FormatMenu.cxx		\
	../bzflag/FormatMenu.h			\
	../bzflag/GUIOptionsMenu.cxx		\
	../bzflag/GUIOptionsMenu.h		\
	../bzflag/GuidedMissleStrategy.cxx	\
	../bzflag/GuidedMissleStrategy.h	\
	../bzflag/HUDDialog.cxx			\
	../bzflag/HUDDialog.h			\
	../bzflag/HUDDialogStack.cxx		\
	../bzflag/HUDDialogStack.h		\
	../bzflag/HUDNavigationQueue.cxx	\
	../bzflag/HUDNavigationQueue.h		\
	../bzflag/HUDRenderer.cxx		\
	../bzflag/HUDRenderer.h			\
	../bzflag/HUDui.cxx			\
	../bzflag/HUDui.h			\
	../bzflag/HUDuiControl.cxx		\
	../bzflag/HUDuiControl.h		\
	../bzflag/HUDuiDefaultKey.cxx		\
	../bzflag/HUDuiDefaultKey.h		\
	../bzflag/HUDuiElement.cxx		\
	../bzflag/HUDuiElement.h		\
	../bzflag/HUDuiFrame.cxx		\
	../bzflag/HUDuiFrame.h			\
	../bzflag/HUDuiImage.cxx		\
	../bzflag/HUDuiImage.h			\
	../bzflag/HUDuiLabel.cxx		\
	../bzflag/HUDuiLabel.h			\
	../bzflag/HUDuiList.cxx			\
	../bzflag/HUDuiList.h			\
	../bzflag/HUDuiTypeIn.cxx		\
	../bzflag/HUDuiTypeIn.h			\
	../bzflag/HelpCreditsMenu.cxx		\
	../bzflag/HelpCreditsMenu.h		\
	../bzflag/HelpFlagsMenu.cxx		\
	../bzflag/HelpFlagsMenu.h		\
	../bzflag/HelpInstructionsMenu.cxx	\
	../bzflag/HelpInstructionsMenu.h	\
	../bzflag/HelpKeymapMenu.cxx		\
	../bzflag/HelpKeymapMenu.h		\
	../bzflag/HelpMenu.cxx			\
	../bzflag/HelpMenu.h			\
	../bzflag/InputMenu.cxx			\
	../bzflag/InputMenu.h			\
	../bzflag/JoinMenu.cxx			\
	../bzflag/JoinMenu.h			\
	../bzflag/KeyboardMapMenu.cxx		\
	../bzflag/KeyboardMapMenu.h		\
	../bzflag/LocalCommand.cxx		\
	../bzflag/LocalCommand.h		\
	../bzflag/LocalPlayer.cxx		\
	../bzflag/LocalPlayer.h			\
	../bzflag/MainMenu.cxx			\
	../bzflag/MainMenu.h			\
	../bzflag/MainWindow.cxx		\
	../bzflag/MainWindow.h			\
	../bzflag/MenuDefaultKey.cxx		\
	../bzflag/MenuDefaultKey.h		\
	../bzflag/NewVersionMenu.cxx		\
	../bzflag/NewVersionMenu.h		\
	../bzflag/OptionsMenu.cxx		\
	../bzflag/OptionsMenu.h			\
	../bzflag/Plan.cxx			\
	../bzflag/Plan.h			\
	../bzflag/Player.cxx			\
	../bzflag/Player.h			\
	../bzflag/QuickKeysMenu.cxx		\
	../bzflag/QuickKeysMenu.h		\
	../bzflag/QuitMenu.cxx			\
	../bzflag/QuitMenu.h			\
	../bzflag/RadarRenderer.cxx		\
	../bzflag/RadarRenderer.h		\
	../bzflag/Region.cxx			\
	../bzflag/Region.h			\
	../bzflag/RegionPriorityQueue.cxx	\
	../bzflag/RegionPriorityQueue.h		\
	../bzflag/RemotePlayer.cxx		\
	../bzflag/RemotePlayer.h		\
	../bzflag/Roaming.cxx			\
	../bzflag/Roaming.h			\
	../bzflag/RobotPlayer.cxx		\
	../bzflag/RobotPlayer.h			\
	../bzflag/RoofTops.cxx			\
	../bzflag/RoofTops.h			\
	../bzflag/Roster.cxx			\
	../bzflag/Roster.h			\
	../bzflag/SaveWorldMenu.cxx		\
	../bzflag/SaveWorldMenu.h		\
	../bzflag/SceneRenderer.cxx		\
	../bzflag/ScoreboardRenderer.cxx	\
	../bzflag/ScoreboardRenderer.h		\
	../bzflag/SegmentedShotStrategy.cxx	\
	../bzflag/SegmentedShotStrategy.h	\
	../bzflag/ServerCommandKey.cxx		\
	../bzflag/ServerCommandKey.h		\
	../bzflag/ServerLink.cxx		\
	../bzflag/ServerLink.h			\
	../bzflag/ServerMenu.cxx		\
	../bzflag/ServerMenu.h			\
	../bzflag/ServerStartMenu.cxx		\
	../bzflag/ServerStartMenu.h		\
	../bzflag/ShockWaveStrategy.cxx		\
	../bzflag/ShockWaveStrategy.h		\
	../bzflag/ShotPath.cxx			\
	../bzflag/ShotPath.h			\
	../bzflag/ShotPathSegment.cxx		\
	../bzflag/ShotPathSegment.h		\
	../bzflag/ShotStatistics.cxx		\
	../bzflag/ShotStatistics.h		\
	../bzflag/ShotStats.cxx			\
	../bzflag/ShotStats.h			\
	../bzflag/ShotStatsDefaultKey.cxx	\
	../bzflag/ShotStatsDefaultKey.h		\
	../bzflag/ShotStrategy.cxx		\
	../bzflag/ShotStrategy.h		\
	../bzflag/SilenceDefaultKey.cxx		\
	../bzflag/SilenceDefaultKey.h		\
	../bzflag/SyncClock.cxx			\
	../bzflag/SyncClock.h			\
	../bzflag/TargetingUtils.cxx		\
	../bzflag/TargetingUtils.h		\
	../bzflag/TrackMarks.cxx		\
	../bzflag/TrackMarks.h			\
	../bzflag/Weapon.cxx			\
	../bzflag/Weapon.h			\
	../bzflag/WeatherRenderer.cxx		\
	../bzflag/WeatherRenderer.h		\
	../bzflag/World.cxx			\
	../bzflag/World.h			\
	../bzflag/WorldBuilder.cxx		\
	../bzflag/WorldBuilder.h		\
	../bzflag/WorldPlayer.cxx		\
	../bzflag/WorldPlayer.h			\
	../bzflag/callbacks.cxx			\
	../bzflag/callbacks.h			\
	../bzflag/clientCommands.cxx		\
	../bzflag/clientConfig.cxx		\
	../bzflag/clientConfig.h		\
	../bzflag/commands.h			\
	../bzflag/daylight.cxx			\
	../bzflag/daylight.h			\
	../bzflag/defaultBZDB.cxx		\
	../bzflag/defaultBZDB.h			\
	../bzflag/effectsRenderer.cxx		\
	../bzflag/effectsRenderer.h		\
	../bzflag/motd.cxx			\
	../bzflag/motd.h			\
	../bzflag/playerAvatarManager.cxx	\
	../bzflag/playerAvatarManager.h		\
	../bzflag/playing.h			\
	../bzflag/sound.cxx			\
	../bzflag/sound.h			\
	../bzflag/stars.cxx			\
	../bzflag/stars.h

bzrobots_LDFLAGS = -static -rdynamic

if CLIENT_INCLUDED
bzrobots_SOURCES = $(genericfiles)
else
bzrobots_SOURCES = $(genericfiles) \
	OpenGLUtils.cxx	
endif

if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if MACOS
NACLIB = ../platform/MacOSX/libMac.la
endif

LDADD =	\
	../3D/lib3D.la			\
	../game/libGame.la		\
	../net/libNet.la		\
	../obstacle/libObstacle.la	\
	$(NACLIB)			\
	../platform/libPlatform.la	\
	../ogl/libGLKit.la		\
	../scene/libSceneDB.la		\
	../geometry/libGeometry.la	\
	../mediafile/libMediaFile.la	\
	../common/libCommon.la		\
	$(LIBCURL)			\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(RESOURCE)			\
	$(SDL_LIBS)			\
	$(SDL_IMAGE_LIBS)		\
	$(GLIBS)			\
	$(GLEW_LIBS)			\
	../date/libDate.la
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff

MAINTAINERCLEANFILES = \
	Makefile.in

lib_LTLIBRARIES = TestRobot.la
TestRobot_la_LDFLAGS = -module -avoid-version
TestRobot_la_SOURCES = \
	BZAdvancedRobot.h \
	TestRobot.cxx \
	TestRobot.h
