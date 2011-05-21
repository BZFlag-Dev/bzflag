#
# Build BZFlag
#

bin_PROGRAMS = bzflag

AM_CPPFLAGS =				\
	-I$(top_srcdir)/src/include	\
	-I$(top_srcdir)/src/clientbase	\
	$(SDL_CFLAGS)			\
	$(LIBCURL_CPPFLAGS)		\
	@X_CFLAGS@			\
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzflag_LDFLAGS = -static
bzflag_SOURCES =			\
	AudioMenu.cpp			\
	AudioMenu.h			\
	AutoHunt.cpp			\
	AutoHunt.h			\
	AutoPilot.cpp			\
	AutoPilot.h			\
	BackgroundRenderer.cpp		\
	BackgroundRenderer.h		\
	CacheMenu.cpp			\
	CacheMenu.h			\
	CommandsImplementation.cpp	\
	ComposeDefaultKey.cpp		\
	ComposeDefaultKey.h		\
	ControlPanel.cpp		\
	ControlPanel.h			\
	CrackedGlass.cpp		\
	CrackedGlass.h			\
	Daylight.cpp			\
	Daylight.h			\
	DebugDrawing.cpp		\
	DebugDrawing.h			\
	DisplayMenu.cpp			\
	DisplayMenu.h			\
	Downloads.cpp			\
	Downloads.h			\
	DynamicWorldText.cpp		\
	DynamicWorldText.h		\
	EffectsMenu.cpp			\
	EffectsMenu.h			\
	EffectsRenderer.cpp		\
	EffectsRenderer.h		\
	ExportInformation.cpp		\
	ExportInformation.h		\
	FontOptionsMenu.cpp		\
	FontOptionsMenu.h		\
	FontSizer.cpp			\
	FontSizer.h			\
	ForceFeedback.cpp		\
	ForceFeedback.h			\
	FormatMenu.cpp			\
	FormatMenu.h			\
	GUIOptionsMenu.cpp		\
	GUIOptionsMenu.h		\
	HUDDialog.cpp			\
	HUDDialog.h			\
	HUDDialogStack.cpp		\
	HUDDialogStack.h		\
	HUDNavigationQueue.cpp		\
	HUDNavigationQueue.h		\
	HUDRenderer.cpp			\
	HUDRenderer.h			\
	HUDui.cpp			\
	HUDui.h				\
	HUDuiControl.cpp		\
	HUDuiControl.h			\
	HUDuiDefaultKey.cpp		\
	HUDuiDefaultKey.h		\
	HUDuiElement.cpp		\
	HUDuiElement.h			\
	HUDuiFrame.cpp			\
	HUDuiFrame.h			\
	HUDuiImage.cpp			\
	HUDuiImage.h			\
	HUDuiLabel.cpp			\
	HUDuiLabel.h			\
	HUDuiList.cpp			\
	HUDuiList.h			\
	HUDuiNestedContainer.cpp	\
	HUDuiNestedContainer.h		\
	HUDuiScrollList.cpp		\
	HUDuiScrollList.h		\
	HUDuiServerInfo.cpp		\
	HUDuiServerInfo.h		\
	HUDuiServerList.cpp		\
	HUDuiServerList.h		\
	HUDuiServerListCache.cpp	\
	HUDuiServerListCache.h		\
	HUDuiServerListItem.cpp		\
	HUDuiServerListItem.h		\
	HUDuiServerListCustomTab.cpp	\
	HUDuiServerListCustomTab.h	\
	HUDuiTabbedControl.cpp		\
	HUDuiTabbedControl.h		\
	HUDuiTypeIn.cpp			\
	HUDuiTypeIn.h			\
	HelpCreditsMenu.cpp		\
	HelpCreditsMenu.h		\
	HelpFlagsMenu.cpp		\
	HelpFlagsMenu.h			\
	HelpInstructionsMenu.cpp	\
	HelpInstructionsMenu.h		\
	HelpKeymapMenu.cpp		\
	HelpKeymapMenu.h		\
	HelpMenu.cpp			\
	HelpMenu.h			\
	HubComposeKey.h			\
	HubComposeKey.cpp		\
	HubLink.h			\
	HubLink.cpp			\
	HubLua.h			\
	HubLua.cpp			\
	HubMenu.h			\
	HubMenu.cpp			\
	InputMenu.cpp			\
	InputMenu.h			\
	JoinMenu.cpp			\
	JoinMenu.h			\
	KeyboardMapMenu.cpp		\
	KeyboardMapMenu.h		\
	LocalCommand.cpp		\
	LocalCommand.h			\
	LocalFontFace.cpp		\
	LocalFontFace.h			\
	MainMenu.cpp			\
	MainMenu.h			\
	MainWindow.cpp			\
	MainWindow.h			\
	MenuDefaultKey.cpp		\
	MenuDefaultKey.h		\
	Mumble.cpp			\
	Mumble.h			\
	NewVersionMenu.cpp		\
	NewVersionMenu.h		\
	OptionsMenu.cpp			\
	OptionsMenu.h			\
	Plan.cpp			\
	Plan.h				\
	PlatformSound.cpp		\
	PlatformSound.h			\
	PlayerAvatarManager.cpp		\
	PlayerAvatarManager.h		\
	QuickKeysMenu.cpp		\
	QuickKeysMenu.h			\
	SaveMenu.cpp			\
	SaveMenu.h			\
	QuitMenu.cpp			\
	QuitMenu.h			\
	RadarOptionsMenu.cpp		\
	RadarOptionsMenu.h		\
	RadarRenderer.cpp		\
	RadarRenderer.h			\
	Roaming.cpp			\
	Roaming.h			\
	RoofTops.cpp			\
	RoofTops.h			\
	SaveWorldMenu.cpp		\
	SaveWorldMenu.h			\
	SceneBuilder.cpp		\
	SceneBuilder.h			\
	SceneRenderer.cpp		\
	ScoreboardRenderer.cpp		\
	ScoreboardRenderer.h		\
	ServerCommandKey.cpp		\
	ServerCommandKey.h		\
	ServerMenu.cpp			\
	ServerMenu.h			\
	ShotList.cpp			\
	ShotList.h			\
	ShotStats.cpp			\
	ShotStats.h			\
	ShotStatsDefaultKey.cpp		\
	ShotStatsDefaultKey.h		\
	SilenceDefaultKey.cpp		\
	SilenceDefaultKey.h		\
	StandardTankAvatar.cpp		\
	StandardTankAvatar.h		\
	TextOptionsMenu.cpp		\
	TextOptionsMenu.h		\
	ThirdPersonVars.cpp		\
	ThirdPersonVars.h		\
	TrackMarks.cpp			\
	TrackMarks.h			\
	WeatherRenderer.cpp		\
	WeatherRenderer.h		\
	XfireGameClient.cpp		\
	XfireGameClient.h		\
	bzflag.cpp			\
	bzflag.h			\
	commands.h			\
	guiplaying.cpp			\
	guiplaying.h			\
	motd.cpp			\
	motd.h				\
	sound.cpp			\
	sound.h				\
	stars.cpp			\
	stars.h

#	ServerListFilter.cpp		\
#	ServerListFilter.h		

if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if MACOS
NACLIB = ../platform/MacOSX/libMac.la
endif

LDADD =	\
	../obstacle/libObstacle.la	\
	../lua/libLuaClient.la		\
	../lua/libLuaClientGL.la	\
	../lua/libLuaGame.la		\
	../clientbase/libClientBase.la	\
	../game/libGame.la		\
	../geometry/libGeometry.la	\
	../3D/lib3D.la			\
	../net/libNet.la		\
	$(NACLIB)			\
	../platform/libPlatform.la	\
	../ogl/libGLKit.la		\
	../scene/libSceneDB.la		\
	../mediafile/libMediaFile.la	\
	../common/libCommon.la		\
	$(LIBLUA)			\
	$(LIBFTGL)			\
	$(LIBFREETYPE)			\
	$(LIBCURL)			\
	$(LIBCARES)			\
	$(LIBDL)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(RESOURCE)			\
	$(SDL_LIBS)			\
	$(GLIBS)			\
	$(GLEW_LIBS)			\
	$(LIBRT)			\
	../date/libDate.la
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff

MAINTAINERCLEANFILES = \
	Makefile.in
