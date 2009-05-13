#
# Build BZFlag
#

bin_PROGRAMS = bzflag

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/clientbase \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	@X_CFLAGS@ \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

bzflag_LDFLAGS = -static
bzflag_SOURCES =			\
	AudioMenu.cxx			\
	AudioMenu.h			\
	AutoHunt.cxx			\
	AutoHunt.h			\
	AutoPilot.cxx			\
	AutoPilot.h			\
	BackgroundRenderer.cxx		\
	BackgroundRenderer.h		\
	CacheMenu.cxx			\
	CacheMenu.h			\
	CommandsImplementation.cxx	\
	ComposeDefaultKey.cxx		\
	ComposeDefaultKey.h		\
	ControlPanel.cxx		\
	ControlPanel.h			\
	CrackedGlass.cxx		\
	CrackedGlass.h			\
	Daylight.cxx			\
	Daylight.h			\
	DisplayMenu.cxx			\
	DisplayMenu.h			\
	Downloads.cxx			\
	Downloads.h			\
	DynamicWorldText.cxx		\
	DynamicWorldText.h		\
	EffectsMenu.cxx			\
	EffectsMenu.h			\
	EffectsRenderer.cxx		\
	EffectsRenderer.h		\
	ExportInformation.cxx		\
	ExportInformation.h		\
	FontSizer.cxx			\
	FontSizer.h			\
	ForceFeedback.cxx		\
	ForceFeedback.h			\
	FormatMenu.cxx			\
	FormatMenu.h			\
	GUIOptionsMenu.cxx		\
	GUIOptionsMenu.h		\
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
	HUDuiNestedContainer.cxx	\
	HUDuiNestedContainer.h		\
	HUDuiScrollList.cxx		\
	HUDuiScrollList.h		\
	HUDuiServerInfo.cxx		\
	HUDuiServerInfo.h		\
	HUDuiServerList.cxx		\
	HUDuiServerList.h		\
	HUDuiServerListCache.cxx	\
	HUDuiServerListCache.h		\
	HUDuiServerListItem.cxx		\
	HUDuiServerListItem.h		\
	HUDuiServerListCustomTab.cxx	\
	HUDuiServerListCustomTab.h		\
	HUDuiTabbedControl.cxx	\
	HUDuiTabbedControl.h	\
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
	LocalFontFace.cxx		\
	LocalFontFace.h			\
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
	PlatformSound.cxx		\
	PlatformSound.h			\
	PlayerAvatarManager.cxx		\
	PlayerAvatarManager.h		\
	QuickKeysMenu.cxx		\
	QuickKeysMenu.h			\
	QuitMenu.cxx			\
	QuitMenu.h			\
	RadarRenderer.cxx		\
	RadarRenderer.h			\
	Roaming.cxx			\
	Roaming.h			\
	RoofTops.cxx			\
	RoofTops.h			\
	SaveWorldMenu.cxx		\
	SaveWorldMenu.h			\
	SceneBuilder.cxx		\
	SceneBuilder.h			\
	SceneRenderer.cxx		\
	ScoreboardRenderer.cxx		\
	ScoreboardRenderer.h		\
	ServerCommandKey.cxx		\
	ServerCommandKey.h		\
	ServerMenu.cxx			\
	ServerMenu.h			\
	ShotList.cxx			\
	ShotList.h			\
	ShotStats.cxx			\
	ShotStats.h			\
	ShotStatsDefaultKey.cxx		\
	ShotStatsDefaultKey.h		\
	SilenceDefaultKey.cxx		\
	SilenceDefaultKey.h		\
	StandardTankAvatar.cxx		\
	StandardTankAvatar.h		\
	TrackMarks.cxx			\
	TrackMarks.h			\
	WeatherRenderer.cxx		\
	WeatherRenderer.h		\
	XfireGameClient.cxx		\
	XfireGameClient.h		\
	bzflag.cxx			\
	bzflag.h			\
	commands.h			\
	motd.cxx			\
	motd.h				\
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

if MACOS
NACLIB = ../platform/MacOSX/libMac.la
endif

LDADD =	\
	../obstacle/libObstacle.la	\
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
	$(IL_LIBS)			\
	$(ILU_LIBS)			\
	$(ILUT_LIBS)			\
	$(GLEW_LIBS)			\
	../date/libDate.la
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff

MAINTAINERCLEANFILES = \
	Makefile.in
