if CLIENT_INCLUDED
bin_PROGRAMS = bzflag
endif

MAINTAINERCLEANFILES = \
	Makefile.in

bzflag_SOURCES = 			\
	MainWindow.cxx			\
	MainWindow.h			\
	ControlPanel.cxx		\
	ControlPanel.h			\
	World.cxx			\
	World.h				\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	RadarRenderer.cxx		\
	RadarRenderer.h			\
	HUDRenderer.cxx			\
	HUDRenderer.h			\
	HUDui.cxx			\
	HUDui.h				\
	ShotPath.cxx			\
	ShotPath.h			\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	Region.cxx			\
	Region.h			\
	RenderNode.cxx			\
	Player.cxx			\
	Player.h			\
	playing.cxx			\
	playing.h			\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	RemotePlayer.cxx		\
	RemotePlayer.h			\
	RobotPlayer.cxx			\
	RobotPlayer.h			\
	DeadPlayer.cxx			\
	DeadPlayer.h			\
	ServerLink.cxx			\
	ServerLink.h			\
	PlayerLink.cxx			\
	PlayerLink.h			\
	KeyMap.cxx			\
	KeyMap.h			\
	daylight.cxx			\
	daylight.h			\
	texture.cxx			\
	texture.h			\
	resources.cxx			\
	resources.h			\
	sound.cxx			\
	sound.h				\
	bzflag.cxx			\
	HUDDialog.cxx			\
	HUDDialog.h			\
	menus.cxx			\
	menus.h				\
	bzflag.rc			\
	bzflag.ico

noinst_HEADERS = \
	stars.cxx

if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

LDADD =	\
	../net/libNet.a			\
	../common/libCommon.a		\
	../obstacle/libObstacle.a	\
	../platform/libPlatform.a	\
	../ogl/libGLKit.a		\
	../scene/libSceneDB.a		\
	../geometry/libGeometry.a	\
	$(RESOURCE)			\
	$(GLIBS)

EXTRA_DIST = bzflag.bmp

AM_CPPFLAGS = $(X_CFLAGS)

bzflag.res: bzflag.rc bzflag.ico
	windres -i bzflag.rc -o bzflag.res -O coff
