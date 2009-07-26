noinst_LTLIBRARIES = libClientBase.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/bzflag \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libClientBase_la_LDFLAGS = -static
libClientBase_la_SOURCES =		\
	ActionBinding.cxx		\
	ActionBinding.h			\
	BaseLocalPlayer.cxx		\
	BaseLocalPlayer.h		\
	ClientIntangibilityManager.cxx	\
	ClientIntangibilityManager.h	\
	CommandsStandard.cxx		\
	CommandsStandard.h		\
	EntryZone.cxx			\
	EntryZone.h			\
	FlashClock.cxx			\
	FlashClock.h			\
	GuidedMissleStrategy.cxx	\
	GuidedMissleStrategy.h		\
	LocalPlayer.cxx			\
	LocalPlayer.h			\
	Player.cxx			\
	Player.h			\
	PointShotStrategy.cxx		\
	PointShotStrategy.h		\
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
	SegmentedShotStrategy.cxx	\
	SegmentedShotStrategy.h		\
	ServerLink.cxx			\
	ServerLink.h			\
	ShockWaveStrategy.cxx		\
	ShockWaveStrategy.h		\
	ShotPath.cxx			\
	ShotPath.h			\
	ShotPathSegment.cxx		\
	ShotPathSegment.h		\
	ShotStatistics.cxx		\
	ShotStatistics.h		\
	ShotStrategy.cxx		\
	ShotStrategy.h			\
	TargetingUtils.cxx		\
	TargetingUtils.h		\
	Weapon.cxx			\
	Weapon.h			\
	World.cxx			\
	World.h				\
	WorldBuilder.cxx		\
	WorldBuilder.h			\
	WorldDownLoader.cxx		\
	WorldDownLoader.h		\
	WorldPlayer.cxx			\
	WorldPlayer.h			\
	callbacks.cxx			\
	callbacks.h			\
	clientCommands.cxx		\
	clientConfig.cxx		\
	clientConfig.h			\
	clientvars.h			\
	defaultBZDB.cxx			\
	defaultBZDB.h			\
	playing.cxx			\
	playing.h			\
	$(top_srcdir)/src/bzflag/AutoHunt.cxx		\
	$(top_srcdir)/src/bzflag/AutoHunt.h		\
	$(top_srcdir)/src/bzflag/AutoPilot.cxx		\
	$(top_srcdir)/src/bzflag/AutoPilot.h		\
	$(top_srcdir)/src/bzflag/BackgroundRenderer.cxx	\
	$(top_srcdir)/src/bzflag/BackgroundRenderer.h	\
	$(top_srcdir)/src/bzflag/CommandsImplementation.cxx	\
	$(top_srcdir)/src/bzflag/ComposeDefaultKey.cxx	\
	$(top_srcdir)/src/bzflag/ComposeDefaultKey.h	\
	$(top_srcdir)/src/bzflag/ControlPanel.cxx	\
	$(top_srcdir)/src/bzflag/ControlPanel.h		\
	$(top_srcdir)/src/bzflag/CrackedGlass.cxx	\
	$(top_srcdir)/src/bzflag/CrackedGlass.h		\
	$(top_srcdir)/src/bzflag/Daylight.cxx		\
	$(top_srcdir)/src/bzflag/Daylight.h		\
	$(top_srcdir)/src/bzflag/Downloads.cxx		\
	$(top_srcdir)/src/bzflag/DebugDrawing.cxx	\
	$(top_srcdir)/src/bzflag/DebugDrawing.h		\
	$(top_srcdir)/src/bzflag/Downloads.h		\
	$(top_srcdir)/src/bzflag/DynamicWorldText.cxx	\
	$(top_srcdir)/src/bzflag/DynamicWorldText.h	\
	$(top_srcdir)/src/bzflag/EffectsRenderer.cxx	\
	$(top_srcdir)/src/bzflag/EffectsRenderer.h	\
	$(top_srcdir)/src/bzflag/FontSizer.cxx		\
	$(top_srcdir)/src/bzflag/FontSizer.h		\
	$(top_srcdir)/src/bzflag/ForceFeedback.cxx	\
	$(top_srcdir)/src/bzflag/ForceFeedback.h	\
	$(top_srcdir)/src/bzflag/HUDDialog.cxx		\
	$(top_srcdir)/src/bzflag/HUDDialog.h		\
	$(top_srcdir)/src/bzflag/HUDDialogStack.cxx	\
	$(top_srcdir)/src/bzflag/HUDDialogStack.h	\
	$(top_srcdir)/src/bzflag/HUDNavigationQueue.cxx	\
	$(top_srcdir)/src/bzflag/HUDNavigationQueue.h	\
	$(top_srcdir)/src/bzflag/HUDRenderer.cxx	\
	$(top_srcdir)/src/bzflag/HUDRenderer.h		\
	$(top_srcdir)/src/bzflag/HUDui.cxx		\
	$(top_srcdir)/src/bzflag/HUDui.h		\
	$(top_srcdir)/src/bzflag/HUDuiControl.cxx	\
	$(top_srcdir)/src/bzflag/HUDuiControl.h		\
	$(top_srcdir)/src/bzflag/HUDuiDefaultKey.cxx	\
	$(top_srcdir)/src/bzflag/HUDuiDefaultKey.h	\
	$(top_srcdir)/src/bzflag/HUDuiElement.cxx	\
	$(top_srcdir)/src/bzflag/HUDuiElement.h		\
	$(top_srcdir)/src/bzflag/HUDuiFrame.cxx		\
	$(top_srcdir)/src/bzflag/HUDuiFrame.h		\
	$(top_srcdir)/src/bzflag/HUDuiImage.cxx		\
	$(top_srcdir)/src/bzflag/HUDuiImage.h		\
	$(top_srcdir)/src/bzflag/HUDuiLabel.cxx		\
	$(top_srcdir)/src/bzflag/HUDuiLabel.h		\
	$(top_srcdir)/src/bzflag/HUDuiList.cxx		\
	$(top_srcdir)/src/bzflag/HUDuiList.h		\
	$(top_srcdir)/src/bzflag/HUDuiTypeIn.cxx	\
	$(top_srcdir)/src/bzflag/HUDuiTypeIn.h		\
	$(top_srcdir)/src/bzflag/LocalCommand.cxx	\
	$(top_srcdir)/src/bzflag/LocalCommand.h		\
	$(top_srcdir)/src/bzflag/LocalFontFace.cxx	\
	$(top_srcdir)/src/bzflag/LocalFontFace.h	\
	$(top_srcdir)/src/bzflag/MainWindow.cxx		\
	$(top_srcdir)/src/bzflag/MainWindow.h		\
	$(top_srcdir)/src/bzflag/Plan.cxx		\
	$(top_srcdir)/src/bzflag/Plan.h			\
	$(top_srcdir)/src/bzflag/PlatformSound.cxx	\
	$(top_srcdir)/src/bzflag/PlatformSound.h	\
	$(top_srcdir)/src/bzflag/PlayerAvatarManager.cxx\
	$(top_srcdir)/src/bzflag/PlayerAvatarManager.h	\
	$(top_srcdir)/src/bzflag/RadarRenderer.cxx	\
	$(top_srcdir)/src/bzflag/RadarRenderer.h	\
	$(top_srcdir)/src/bzflag/Roaming.cxx		\
	$(top_srcdir)/src/bzflag/Roaming.h		\
	$(top_srcdir)/src/bzflag/RoofTops.cxx		\
	$(top_srcdir)/src/bzflag/RoofTops.h		\
	$(top_srcdir)/src/bzflag/SceneRenderer.cxx	\
	$(top_srcdir)/src/bzflag/ScoreboardRenderer.cxx	\
	$(top_srcdir)/src/bzflag/ScoreboardRenderer.h	\
	$(top_srcdir)/src/bzflag/ServerCommandKey.cxx	\
	$(top_srcdir)/src/bzflag/ServerCommandKey.h	\
	$(top_srcdir)/src/bzflag/ShotStats.cxx		\
	$(top_srcdir)/src/bzflag/ShotStats.h		\
	$(top_srcdir)/src/bzflag/ShotStatsDefaultKey.cxx\
	$(top_srcdir)/src/bzflag/ShotStatsDefaultKey.h	\
	$(top_srcdir)/src/bzflag/SilenceDefaultKey.cxx	\
	$(top_srcdir)/src/bzflag/SilenceDefaultKey.h	\
	$(top_srcdir)/src/bzflag/StandardTankAvatar.cxx	\
	$(top_srcdir)/src/bzflag/StandardTankAvatar.h	\
	$(top_srcdir)/src/bzflag/ThirdPersonVars.cxx	\
	$(top_srcdir)/src/bzflag/ThirdPersonVars.h	\
	$(top_srcdir)/src/bzflag/TrackMarks.cxx		\
	$(top_srcdir)/src/bzflag/TrackMarks.h		\
	$(top_srcdir)/src/bzflag/WeatherRenderer.cxx	\
	$(top_srcdir)/src/bzflag/WeatherRenderer.h	\
	$(top_srcdir)/src/bzflag/commands.h		\
	$(top_srcdir)/src/bzflag/sound.cxx		\
	$(top_srcdir)/src/bzflag/sound.h		\
	$(top_srcdir)/src/bzflag/stars.cxx		\
	$(top_srcdir)/src/bzflag/stars.h


EXTRA_DIST = \
	README
