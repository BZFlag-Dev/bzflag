if CLIENT_INCLUDED
bin_PROGRAMS = bzflag
endif

MAINTAINERCLEANFILES = \
	Makefile.in

bzflag_SOURCES =			\
	ActionBinding.cxx		\
	ActionBinding.h			\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	ControlPanel.cxx		\
	ControlPanel.h			\
	CommandsStandard.cxx		\
	HUDDialog.cxx			\
	HUDDialog.h			\
	HUDRenderer.cxx			\
	HUDRenderer.h			\
	HUDui.cxx			\
	HUDui.h				\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	MainWindow.cxx			\
	MainWindow.h			\
	Player.cxx			\
	Player.h			\
	RadarRenderer.cxx		\
	RadarRenderer.h			\
	Region.cxx			\
	Region.h			\
	RemotePlayer.cxx		\
	RemotePlayer.h			\
	RobotPlayer.cxx			\
	RobotPlayer.h			\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	ServerListCache.cxx		\
	ServerListCache.h		\
	ServerLink.cxx			\
	ServerLink.h			\
	ShotPath.cxx			\
	ShotPath.h			\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	TargetingUtils.cxx		\
	TargetingUtils.h		\
	TextureManager.cxx		\
	World.cxx			\
	World.h				\
	WorldPlayer.cxx			\
	WorldPlayer.h			\
	bzflag.cxx			\
	callbacks.h			\
	callbacks.cxx			\
	daylight.cxx			\
	daylight.h			\
	menus.cxx			\
	menus.h				\
	playing.cxx			\
	playing.h			\
	sound.cxx			\
	sound.h				\
	stars.h				\
	texture.cxx			

if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if BUILD_REGEX
REGEX = \
        ../regex/libregex.a
endif

if MACOS
LDADD =	\
	../net/libNet.a			\
	../common/libCommon.a		\
	../obstacle/libObstacle.a	\
	../platform/MacOSX/libMac.a	\
	../platform/libPlatform.a	\
	../ogl/libGLKit.a		\
	../scene/libSceneDB.a		\
	../geometry/libGeometry.a	\
	../mediafile/libMediaFile.a	\
	../zlib/libz.a			\
	$(REGEX)			\
	$(RESOURCE)			\
	$(GLIBS)
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)
else
LDADD =	\
	../net/libNet.a			\
	../common/libCommon.a		\
	../obstacle/libObstacle.a	\
	../platform/libPlatform.a	\
	../ogl/libGLKit.a		\
	../scene/libSceneDB.a		\
	../geometry/libGeometry.a	\
	../mediafile/libMediaFile.a	\
	../zlib/libz.a			\
	$(REGEX)			\
	$(RESOURCE)			\
	$(GLIBS)
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)
endif

AM_CPPFLAGS = $(X_CFLAGS)

bzflag_LDFLAGS = ../date/buildDate.o

bzflag.res: $(srcdir)/bzflag.rc $(srcdir)/bzflag.ico
	windres -include-dir=$(srcdir) -i $(srcdir)/bzflag.rc -o bzflag.res \
	-O coff
