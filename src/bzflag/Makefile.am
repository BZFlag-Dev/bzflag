if CLIENT_INCLUDED
bin_PROGRAMS = bzflag
endif

MAINTAINERCLEANFILES = \
	Makefile.in

bzflag_SOURCES =			\
	ActionBinding.cxx		\
	ActionBinding.h			\
	AudioMenu.cxx			\
	AudioMenu.h			\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	BaseLocalPlayer.cxx		\
	BaseLocalPlayer.h		\
	ComposeDefaultKey.cxx		\
	ComposeDefaultKey.h		\
	ControlPanel.cxx		\
	ControlPanel.h			\
	DisplayMenu.cxx			\
	DisplayMenu.h			\
	FlashClock.cxx			\
	FlashClock.h			\
	FormatMenu.cxx			\
	FormatMenu.h			\
	GuidedMissleStrategy.cxx	\
	GuidedMissleStrategy.h		\
	GUIOptionsMenu.cxx		\
	GUIOptionsMenu.h		\
	HelpMenu.cxx			\
	HelpMenu.h			\
	HUDDialog.cxx			\
	HUDDialog.h			\
	HUDDialogStack.cxx		\
	HUDDialogStack.h		\
	HUDRenderer.cxx			\
	HUDRenderer.h			\
	HUDui.cxx			\
	HUDui.h				\
	JoinMenu.cxx			\
	JoinMenu.h			\
	InputMenu.cxx			\
	InputMenu.h			\
	KeyboardMapMenu.cxx		\
	KeyboardMapMenu.h		\
	ListServer.h			\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	MainMenu.cxx			\
	MainMenu.h			\
	MainWindow.cxx			\
	MainWindow.h			\
	MenuDefaultKey.h		\
	MenuDefaultKey.cxx		\
	OptionsMenu.cxx			\
	OptionsMenu.h			\
	Player.cxx			\
	Player.h			\
	QuickKeysMenu.cxx		\
	QuickKeysMenu.h			\
	QuitMenu.cxx			\
	QuitMenu.h			\
	RadarRenderer.cxx		\
	RadarRenderer.h			\
	Region.cxx			\
	Region.h			\
	RegionPriorityQueue.cxx		\
	RegionPriorityQueue.h		\
	RemotePlayer.cxx		\
	RemotePlayer.h			\
	RobotPlayer.cxx			\
	RobotPlayer.h			\
	Roster.cxx			\
	Roster.h			\
	SaveWorldMenu.cxx		\
	SaveWorldMenu.h			\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	SegmentedShotStrategy.cxx	\
	SegmentedShotStrategy.h		\
	ServerCommandKey.cxx		\
	ServerCommandKey.h		\
	ServerItem.cxx			\
	ServerItem.h			\
	ServerListCache.cxx		\
	ServerListCache.h		\
	ServerLink.cxx			\
	ServerLink.h			\
	ServerMenu.cxx			\
	ServerMenu.h			\
	ServerStartMenu.cxx		\
	ServerStartMenu.h		\
	ShockWaveStrategy.cxx		\
	ShockWaveStrategy.h		\
	ShotPath.cxx			\
	ShotPath.h			\
	ShotPathSegment.cxx		\
	ShotPathSegment.h		\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	SilenceDefaultKey.cxx		\
	SilenceDefaultKey.h		\
	StartupInfo.h			\
	TargetingUtils.cxx		\
	TargetingUtils.h		\
	World.cxx			\
	World.h				\
	WorldBuilder.cxx		\
	WorldBuilder.h			\
	WorldPlayer.cxx			\
	WorldPlayer.h			\
	bzflag.cxx			\
	callbacks.cxx			\
	callbacks.h			\
	clientCommands.cxx			\
	commands.h			\
	daylight.cxx			\
	daylight.h			\
	playing.cxx			\
	playing.h			\
	sound.cxx			\
	sound.h				\
	stars.cxx			\
	stars.h				\
	texture.cxx



if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if BUILDZLIB
ZLIB = ../zlib/libz.a
else
ZLIB = -lz
endif

if MACOS
EXTRALIB =	\
	../platform/MacOSX/libMac.a
endif

LDADD =	\
	../net/libNet.a			\
	../common/libCommon.a		\
	../game/libGame.a		\
	../obstacle/libObstacle.a	\
	../platform/libPlatform.a	\
	../ogl/libGLKit.a		\
	../scene/libSceneDB.a		\
	../geometry/libGeometry.a	\
	../mediafile/libMediaFile.a	\
	../3D/lib3D.a			\
	$(EXTRALIB)			\
	$(ZLIB)				\
	$(RESOURCE)			\
	$(GLIBS)			\
	$(SDL_LIBS)
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

AM_CPPFLAGS = $(X_CFLAGS) $(SDL_CFLAGS)

bzflag_LDFLAGS = ../date/buildDate.o

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	windres --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff
