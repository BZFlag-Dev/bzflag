noinst_LTLIBRARIES = libClientBase.la

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/bzflag \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libClientBase_la_LDFLAGS = -static
libClientBase_la_SOURCES =		\
	ActionBinding.cpp		\
	ActionBinding.h			\
	BaseLocalPlayer.cpp		\
	BaseLocalPlayer.h		\
	ClientFlag.h			\
	ClientIntangibilityManager.cpp	\
	ClientIntangibilityManager.h	\
	CommandsStandard.cpp		\
	CommandsStandard.h		\
	EntryZone.cpp			\
	EntryZone.h			\
	EventClient.cpp			\
	EventClientList.cpp		\
	EventHandler.cpp		\
	FlashClock.cpp			\
	FlashClock.h			\
	GfxBlock.cpp			\
	GuidedMissileStrategy.cpp	\
	GuidedMissileStrategy.h		\
	LocalPlayer.cpp			\
	LocalPlayer.h			\
	Player.cpp			\
	Player.h			\
	PointShotStrategy.cpp		\
	PointShotStrategy.h		\
	Region.cpp			\
	Region.h			\
	RegionPriorityQueue.cpp		\
	RegionPriorityQueue.h		\
	RemotePlayer.cpp		\
	RemotePlayer.h			\
	RobotPlayer.cpp			\
	RobotPlayer.h			\
	Roster.cpp			\
	Roster.h			\
	SegmentedShotStrategy.cpp	\
	SegmentedShotStrategy.h		\
	ServerLink.cpp			\
	ServerLink.h			\
	ShockWaveStrategy.cpp		\
	ShockWaveStrategy.h		\
	ShotPath.cpp			\
	ShotPath.h			\
	ShotPathSegment.cpp		\
	ShotPathSegment.h		\
	ShotStatistics.cpp		\
	ShotStatistics.h		\
	ShotStrategy.cpp		\
	ShotStrategy.h			\
	TargetingUtils.cpp		\
	TargetingUtils.h		\
	Weapon.cpp			\
	Weapon.h			\
	World.cpp			\
	World.h				\
	WorldBuilder.cpp		\
	WorldBuilder.h			\
	WorldDownLoader.cpp		\
	WorldDownLoader.h		\
	WorldPlayer.cpp			\
	WorldPlayer.h			\
	callbacks.cpp			\
	callbacks.h			\
	clientCommands.cpp		\
	clientConfig.cpp		\
	clientConfig.h			\
	clientvars.h			\
	defaultBZDB.cpp			\
	defaultBZDB.h			\
	playing.cpp			\
	playing.h			\
	$(top_srcdir)/src/bzflag/AutoHunt.cpp		\
	$(top_srcdir)/src/bzflag/AutoHunt.h		\
	$(top_srcdir)/src/bzflag/AutoPilot.cpp		\
	$(top_srcdir)/src/bzflag/AutoPilot.h		\
	$(top_srcdir)/src/bzflag/BackgroundRenderer.cpp	\
	$(top_srcdir)/src/bzflag/BackgroundRenderer.h	\
	$(top_srcdir)/src/bzflag/CommandsImplementation.cpp	\
	$(top_srcdir)/src/bzflag/ComposeDefaultKey.cpp	\
	$(top_srcdir)/src/bzflag/ComposeDefaultKey.h	\
	$(top_srcdir)/src/bzflag/ControlPanel.cpp	\
	$(top_srcdir)/src/bzflag/ControlPanel.h		\
	$(top_srcdir)/src/bzflag/CrackedGlass.cpp	\
	$(top_srcdir)/src/bzflag/CrackedGlass.h		\
	$(top_srcdir)/src/bzflag/Daylight.cpp		\
	$(top_srcdir)/src/bzflag/Daylight.h		\
	$(top_srcdir)/src/bzflag/Downloads.cpp		\
	$(top_srcdir)/src/bzflag/DebugDrawing.cpp	\
	$(top_srcdir)/src/bzflag/DebugDrawing.h		\
	$(top_srcdir)/src/bzflag/Downloads.h		\
	$(top_srcdir)/src/bzflag/DynamicWorldText.cpp	\
	$(top_srcdir)/src/bzflag/DynamicWorldText.h	\
	$(top_srcdir)/src/bzflag/EffectsRenderer.cpp	\
	$(top_srcdir)/src/bzflag/EffectsRenderer.h	\
	$(top_srcdir)/src/bzflag/FontSizer.cpp		\
	$(top_srcdir)/src/bzflag/FontSizer.h		\
	$(top_srcdir)/src/bzflag/ForceFeedback.cpp	\
	$(top_srcdir)/src/bzflag/ForceFeedback.h	\
	$(top_srcdir)/src/bzflag/HUDDialog.cpp		\
	$(top_srcdir)/src/bzflag/HUDDialog.h		\
	$(top_srcdir)/src/bzflag/HUDDialogStack.cpp	\
	$(top_srcdir)/src/bzflag/HUDDialogStack.h	\
	$(top_srcdir)/src/bzflag/HUDNavigationQueue.cpp	\
	$(top_srcdir)/src/bzflag/HUDNavigationQueue.h	\
	$(top_srcdir)/src/bzflag/HUDRenderer.cpp	\
	$(top_srcdir)/src/bzflag/HUDRenderer.h		\
	$(top_srcdir)/src/bzflag/HUDui.cpp		\
	$(top_srcdir)/src/bzflag/HUDui.h		\
	$(top_srcdir)/src/bzflag/HUDuiControl.cpp	\
	$(top_srcdir)/src/bzflag/HUDuiControl.h		\
	$(top_srcdir)/src/bzflag/HUDuiDefaultKey.cpp	\
	$(top_srcdir)/src/bzflag/HUDuiDefaultKey.h	\
	$(top_srcdir)/src/bzflag/HUDuiElement.cpp	\
	$(top_srcdir)/src/bzflag/HUDuiElement.h		\
	$(top_srcdir)/src/bzflag/HUDuiFrame.cpp		\
	$(top_srcdir)/src/bzflag/HUDuiFrame.h		\
	$(top_srcdir)/src/bzflag/HUDuiImage.cpp		\
	$(top_srcdir)/src/bzflag/HUDuiImage.h		\
	$(top_srcdir)/src/bzflag/HUDuiLabel.cpp		\
	$(top_srcdir)/src/bzflag/HUDuiLabel.h		\
	$(top_srcdir)/src/bzflag/HUDuiList.cpp		\
	$(top_srcdir)/src/bzflag/HUDuiList.h		\
	$(top_srcdir)/src/bzflag/HUDuiTypeIn.cpp	\
	$(top_srcdir)/src/bzflag/HUDuiTypeIn.h		\
	$(top_srcdir)/src/bzflag/HubComposeKey.h	\
	$(top_srcdir)/src/bzflag/HubComposeKey.cpp	\
	$(top_srcdir)/src/bzflag/LocalCommand.cpp	\
	$(top_srcdir)/src/bzflag/LocalCommand.h		\
	$(top_srcdir)/src/bzflag/LocalFontFace.cpp	\
	$(top_srcdir)/src/bzflag/LocalFontFace.h	\
	$(top_srcdir)/src/bzflag/MainWindow.cpp		\
	$(top_srcdir)/src/bzflag/MainWindow.h		\
	$(top_srcdir)/src/bzflag/Mumble.cpp		\
	$(top_srcdir)/src/bzflag/Mumble.h		\
	$(top_srcdir)/src/bzflag/Plan.cpp		\
	$(top_srcdir)/src/bzflag/Plan.h			\
	$(top_srcdir)/src/bzflag/PlatformSound.cpp	\
	$(top_srcdir)/src/bzflag/PlatformSound.h	\
	$(top_srcdir)/src/bzflag/PlayerAvatarManager.cpp\
	$(top_srcdir)/src/bzflag/PlayerAvatarManager.h	\
	$(top_srcdir)/src/bzflag/RadarRenderer.cpp	\
	$(top_srcdir)/src/bzflag/RadarRenderer.h	\
	$(top_srcdir)/src/bzflag/Roaming.cpp		\
	$(top_srcdir)/src/bzflag/Roaming.h		\
	$(top_srcdir)/src/bzflag/RoofTops.cpp		\
	$(top_srcdir)/src/bzflag/RoofTops.h		\
	$(top_srcdir)/src/bzflag/SceneRenderer.cpp	\
	$(top_srcdir)/src/bzflag/ScoreboardRenderer.cpp	\
	$(top_srcdir)/src/bzflag/ScoreboardRenderer.h	\
	$(top_srcdir)/src/bzflag/ServerCommandKey.cpp	\
	$(top_srcdir)/src/bzflag/ServerCommandKey.h	\
	$(top_srcdir)/src/bzflag/ShotStats.cpp		\
	$(top_srcdir)/src/bzflag/ShotStats.h		\
	$(top_srcdir)/src/bzflag/ShotStatsDefaultKey.cpp\
	$(top_srcdir)/src/bzflag/ShotStatsDefaultKey.h	\
	$(top_srcdir)/src/bzflag/SilenceDefaultKey.cpp	\
	$(top_srcdir)/src/bzflag/SilenceDefaultKey.h	\
	$(top_srcdir)/src/bzflag/StandardTankAvatar.cpp	\
	$(top_srcdir)/src/bzflag/StandardTankAvatar.h	\
	$(top_srcdir)/src/bzflag/ThirdPersonVars.cpp	\
	$(top_srcdir)/src/bzflag/ThirdPersonVars.h	\
	$(top_srcdir)/src/bzflag/TrackMarks.cpp		\
	$(top_srcdir)/src/bzflag/TrackMarks.h		\
	$(top_srcdir)/src/bzflag/WeatherRenderer.cpp	\
	$(top_srcdir)/src/bzflag/WeatherRenderer.h	\
	$(top_srcdir)/src/bzflag/commands.h		\
	$(top_srcdir)/src/bzflag/sound.cpp		\
	$(top_srcdir)/src/bzflag/sound.h		\
	$(top_srcdir)/src/bzflag/stars.cpp		\
	$(top_srcdir)/src/bzflag/stars.h


EXTRA_DIST = \
	README
