bin_PROGRAMS = bzflag

AM_CPPFLAGS = -DINSTALL_DATA_DIR=\"$(pkgdatadir)\" \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

MAINTAINERCLEANFILES = \
	Makefile.in

bzflag_SOURCES =			\
	ActionBinding.cxx		\
	ActionBinding.h			\
	AutoPilot.cxx			\
	AutoPilot.h			\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	BaseLocalPlayer.cxx		\
	BaseLocalPlayer.h		\
	CacheMenu.cxx			\
	CacheMenu.h			\
	clientConfig.cxx		\
	clientConfig.h			\
	CommandsImplementation.cxx	\
	ComposeDefaultKey.cxx		\
	ComposeDefaultKey.h		\
	ControlPanel.cxx		\
	ControlPanel.h			\
	defaultBZDB.h			\
	defaultBZDB.cxx			\
	DisplayMenu.cxx			\
	DisplayMenu.h			\
	Downloads.cxx			\
	Downloads.h			\
	EffectsMenu.cxx			\
	EffectsMenu.h			\
	effectsRenderer.cxx		\
	effectsRenderer.h		\
	EntryZone.cxx			\
	EntryZone.h			\
	FlashClock.cxx			\
	FlashClock.h			\
	ForceFeedback.cxx		\
	ForceFeedback.h			\
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
	HUDuiControl.cxx		\
	HUDuiControl.h			\
	HUDuiDefaultKey.cxx		\
	HUDuiDefaultKey.h		\
	HUDuiJSTestLabel.cxx		\
	HUDuiJSTestLabel.h		\
	HUDuiLabel.cxx			\
	HUDuiLabel.h			\
	HUDuiList.cxx			\
	HUDuiList.h			\
	HUDuiTextureLabel.cxx		\
	HUDuiTextureLabel.h		\
	HUDuiTypeIn.cxx			\
	HUDuiTypeIn.h			\
	JoystickTestMenu.cxx		\
	JoystickTestMenu.h		\
	JoinMenu.cxx			\
	JoinMenu.h			\
	InputMenu.cxx			\
	InputMenu.h			\
	KeyboardMapMenu.cxx		\
	KeyboardMapMenu.h		\
	LocalCommand.cxx		\
	LocalCommand.h			\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	MainMenu.cxx			\
	MainMenu.h			\
	MainWindow.cxx			\
	MainWindow.h			\
	MenuDefaultKey.h		\
	MenuDefaultKey.cxx		\
	motd.h				\
	motd.cxx			\
	NewVersionMenu.cxx		\
	NewVersionMenu.h		\
	OptionsMenu.cxx			\
	OptionsMenu.h			\
	Player.cxx			\
	Player.h			\
	Plan.cxx			\
	Plan.h				\
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
	Roaming.cxx			\
	Roaming.h			\
	RobotPlayer.cxx			\
	RobotPlayer.h			\
	RoofTops.cxx			\
	RoofTops.h			\
	Roster.cxx			\
	Roster.h			\
	SaveWorldMenu.cxx		\
	SaveWorldMenu.h			\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	ScoreboardRenderer.cxx		\
	ScoreboardRenderer.h		\
	SegmentedShotStrategy.cxx	\
	SegmentedShotStrategy.h		\
	ServerCommandKey.cxx		\
	ServerCommandKey.h		\
	ServerLink.cxx			\
	ServerLink.h			\
	ServerListFilter.cxx		\
	ServerListFilter.h		\
	ServerListFilterHelpMenu.cxx	\
	ServerListFilterHelpMenu.h	\
	ServerListFilterMenu.cxx	\
	ServerListFilterMenu.h		\
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
	ShotStats.cxx			\
	ShotStats.h			\
	ShotStatsDefaultKey.cxx		\
	ShotStatsDefaultKey.h		\
	ShotStatistics.h		\
	ShotStatistics.cxx		\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	SilenceDefaultKey.cxx		\
	SilenceDefaultKey.h		\
	TargetingUtils.cxx		\
	TargetingUtils.h		\
	TrackMarks.cxx			\
	TrackMarks.h			\
	Weapon.cxx			\
	Weapon.h			\
	WeatherRenderer.h		\
	WeatherRenderer.cxx		\
	World.cxx			\
	World.h				\
	WorldBuilder.cxx		\
	WorldBuilder.h			\
	WorldPlayer.cxx			\
	WorldPlayer.h			\
	bzflag.cxx			\
	bzflag.h			\
	callbacks.cxx			\
	callbacks.h			\
	clientCommands.cxx		\
	commands.h			\
	daylight.cxx			\
	daylight.h			\
	playing.cxx			\
	playing.h			\
	sound.cxx			\
	sound.h				\
	stars.cxx			\
	stars.h


if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

LDADD =	\
	../3D/lib3D.la			\
	../game/libGame.la		\
	../net/libNet.la		\
	../obstacle/libObstacle.la	\
	../platform/libPlatform.la	\
	../ogl/libGLKit.la		\
	../scene/libSceneDB.la		\
	../geometry/libGeometry.la	\
	../mediafile/libMediaFile.la	\
	../common/libCommon.la		\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(RESOURCE)			\
	$(SDL_LIBS)			\
	$(SDL_IMAGE_LIBS)		\
	$(GLIBS)			\
	$(LIBCURL)			\
	$(GLEW_LIBS)	\
	../date/libDate.la
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff
