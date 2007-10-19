#
# Build BZFlag
#

bin_PROGRAMS = bzflag

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	@X_CFLAGS@ \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzflag_LDFLAGS = -static
bzflag_SOURCES =			\
	ActionBinding.cxx		\
	ActionBinding.h			\
	AudioMenu.cxx			\
	AudioMenu.h			\
	AutoHunt.cxx			\
	AutoHunt.h			\
	AutoPilot.cxx			\
	AutoPilot.h			\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	BaseLocalPlayer.cxx		\
	BaseLocalPlayer.h		\
	CacheMenu.cxx			\
	CacheMenu.h			\
	CommandsImplementation.cxx	\
	ComposeDefaultKey.cxx		\
	ComposeDefaultKey.h		\
	ControlPanel.cxx		\
	ControlPanel.h			\
	CrackedGlass.cxx		\
	CrackedGlass.h			\
	ClientIntangibilityManager.h \
	ClientIntangibilityManager.cxx \
	DisplayMenu.cxx			\
	DisplayMenu.h			\
	Downloads.cxx			\
	Downloads.h			\
	EffectsMenu.cxx			\
	EffectsMenu.h			\
	EntryZone.cxx			\
	EntryZone.h			\
	ExportInformation.cxx		\
	ExportInformation.h		\
	FlashClock.cxx			\
	FlashClock.h			\
	FontSizer.cxx			\
	FontSizer.h			\
	ForceFeedback.cxx		\
	ForceFeedback.h			\
	FormatMenu.cxx			\
	FormatMenu.h			\
	GUIOptionsMenu.cxx		\
	GUIOptionsMenu.h		\
	GuidedMissleStrategy.cxx	\
	GuidedMissleStrategy.h		\
	HUDDialog.cxx			\
	HUDDialog.h			\
	HUDDialogStack.cxx		\
	HUDDialogStack.h		\
	HUDNavigationQueue.cxx		\
	HUDNavigationQueue.h		\
	HUDRenderer.cxx			\
	HUDRenderer.h			\
	HUDui.cxx			\
	HUDui.h				\
	HUDuiControl.cxx		\
	HUDuiControl.h			\
	HUDuiDefaultKey.cxx		\
	HUDuiDefaultKey.h		\
	HUDuiElement.cxx		\
	HUDuiElement.h			\
	HUDuiFrame.cxx			\
	HUDuiFrame.h			\
	HUDuiImage.cxx			\
	HUDuiImage.h			\
	HUDuiLabel.cxx			\
	HUDuiLabel.h			\
	HUDuiList.cxx			\
	HUDuiList.h			\
	HUDuiTypeIn.cxx			\
	HUDuiTypeIn.h			\
	HelpCreditsMenu.cxx		\
	HelpCreditsMenu.h		\
	HelpFlagsMenu.cxx		\
	HelpFlagsMenu.h			\
	HelpInstructionsMenu.cxx	\
	HelpInstructionsMenu.h		\
	HelpKeymapMenu.cxx		\
	HelpKeymapMenu.h		\
	HelpMenu.cxx			\
	HelpMenu.h			\
	InputMenu.cxx			\
	InputMenu.h			\
	JoinMenu.cxx			\
	JoinMenu.h			\
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
	MenuDefaultKey.cxx		\
	MenuDefaultKey.h		\
	NewVersionMenu.cxx		\
	NewVersionMenu.h		\
	OptionsMenu.cxx			\
	OptionsMenu.h			\
	Plan.cxx			\
	Plan.h				\
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
	ShotStatistics.cxx		\
	ShotStatistics.h		\
	ShotStats.cxx			\
	ShotStats.h			\
	ShotStatsDefaultKey.cxx		\
	ShotStatsDefaultKey.h		\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	SilenceDefaultKey.cxx		\
	SilenceDefaultKey.h		\
	SandardTankAvatar.h		\
	StanadardTankAvatar.h	\
	SyncClock.cxx		\
	SyncClock.h		\
	TargetingUtils.cxx		\
	TargetingUtils.h		\
	TrackMarks.cxx			\
	TrackMarks.h			\
	Weapon.cxx			\
	Weapon.h			\
	WeatherRenderer.cxx		\
	WeatherRenderer.h		\
	World.cxx			\
	World.h				\
	WorldBuilder.cxx		\
	WorldBuilder.h			\
	WorldPlayer.cxx			\
	WorldPlayer.h			\
	XfireGameClient.cpp		\
	XfireGameClient.h		\
	bzflag.cxx			\
	bzflag.h			\
	callbacks.cxx			\
	callbacks.h			\
	clientCommands.cxx		\
	clientConfig.cxx		\
	clientConfig.h			\
	commands.h			\
	daylight.cxx			\
	daylight.h			\
	defaultBZDB.cxx			\
	defaultBZDB.h			\
	effectsRenderer.cxx		\
	effectsRenderer.h		\
	motd.cxx			\
	motd.h				\
	playerAvatarManager.cxx		\
	playerAvatarManager.h		\
	playing.cxx			\
	playing.h			\
	sound.cxx			\
	sound.h				\
	stars.cxx			\
	stars.h

EXTRA_bzflag_SOURCES = \
	ClientAuthentication.cxx	\
	ClientAuthentication.h		\
	LoginMenu.cxx			\
	LoginMenu.h

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
	$(LIBFTGL)			\
	$(LIBFREETYPE)			\
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
