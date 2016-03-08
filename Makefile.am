AUTOMAKE_OPTIONS = 1.6 dist-zip dist-bzip2
ACLOCAL_AMFLAGS = -I m4

traversal_dirs = \
	data \
	include \
	man \
	package \
	src \
	.

if BUILD_PLUGINS
PLUGINS=plugins
else
PLUGINS=
endif

SUBDIRS = \
	$(traversal_dirs) \
	$(PLUGINS)

DIST_SUBDIRS = \
	$(traversal_dirs) \
	m4 \
	misc \
	plugins \
	tools \
	MSVC \
	Xcode

if SERVER_INCLUDED
bin_PROGRAMS = bzfs
else
bin_PROGRAMS =
endif

if CLIENT_INCLUDED
bin_PROGRAMS += bzflag
endif

if BZADMIN_INCLUDED
bin_PROGRAMS += bzadmin
endif

check_PROGRAMS = unittests

bzfs_CPPFLAGS = -DINSIDE_BZ \
	-DINSTALL_LIB_DIR=\"$(libdir)/\" \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
bzfs_CFLAGS = $(CONF_CFLAGS)
bzfs_CXXFLAGS = $(CONF_CXXFLAGS)

if BUILD_PLUGINS
plugin_files = src/bzfs/bzfsPlugins.h src/bzfs/bzfsPlugins.cxx
else
plugin_files =
endif

bzfs_LDFLAGS = -export-dynamic -static
bzfs_SOURCES = \
	${plugin_files}				\
	src/bzfs/AccessControlList.cxx		\
	src/bzfs/AccessControlList.h		\
	src/bzfs/Authentication.cxx		\
	src/bzfs/Authentication.h		\
	src/bzfs/BanCommands.cxx		\
	src/bzfs/base64.cxx			\
	src/bzfs/base64.h			\
	src/bzfs/bzfsAPI.cxx			\
	src/bzfs/bzfsHTTPAPI.cxx		\
	src/bzfs/BZWError.cxx			\
	src/bzfs/BZWError.h			\
	src/bzfs/BZWReader.cxx			\
	src/bzfs/BZWReader.h			\
	src/bzfs/CmdLineOptions.cxx		\
	src/bzfs/CmdLineOptions.h		\
	src/bzfs/CustomArc.cxx			\
	src/bzfs/CustomArc.h			\
	src/bzfs/CustomBase.cxx			\
	src/bzfs/CustomBase.h			\
	src/bzfs/CustomBox.cxx			\
	src/bzfs/CustomBox.h			\
	src/bzfs/CustomCone.cxx			\
	src/bzfs/CustomCone.h			\
	src/bzfs/CustomDynamicColor.cxx		\
	src/bzfs/CustomDynamicColor.h		\
	src/bzfs/CustomGate.cxx			\
	src/bzfs/CustomGate.h			\
	src/bzfs/CustomGroup.cxx		\
	src/bzfs/CustomGroup.h			\
	src/bzfs/CustomLink.cxx			\
	src/bzfs/CustomLink.h			\
	src/bzfs/CustomMaterial.h		\
	src/bzfs/CustomMaterial.cxx		\
	src/bzfs/CustomMesh.cxx			\
	src/bzfs/CustomMesh.h			\
	src/bzfs/CustomMeshFace.cxx		\
	src/bzfs/CustomMeshFace.h		\
	src/bzfs/CustomMeshTransform.cxx	\
	src/bzfs/CustomMeshTransform.h		\
	src/bzfs/CustomPhysicsDriver.cxx	\
	src/bzfs/CustomPhysicsDriver.h		\
	src/bzfs/CustomSphere.cxx		\
	src/bzfs/CustomSphere.h			\
	src/bzfs/CustomTextureMatrix.cxx	\
	src/bzfs/CustomTextureMatrix.h		\
	src/bzfs/CustomPyramid.cxx		\
	src/bzfs/CustomPyramid.h		\
	src/bzfs/CustomTetra.cxx		\
	src/bzfs/CustomTetra.h			\
	src/bzfs/CustomWaterLevel.cxx		\
	src/bzfs/CustomWaterLevel.h		\
	src/bzfs/CustomWeapon.cxx		\
	src/bzfs/CustomWeapon.h			\
	src/bzfs/CustomWorld.cxx		\
	src/bzfs/CustomWorld.h			\
	src/bzfs/CustomZone.cxx			\
	src/bzfs/CustomZone.h			\
	src/bzfs/DropGeometry.cxx		\
	src/bzfs/DropGeometry.h			\
	src/bzfs/EntryZones.cxx			\
	src/bzfs/EntryZones.h			\
	src/bzfs/Filter.cxx			\
	src/bzfs/Filter.h			\
	src/bzfs/FlagHistory.cxx		\
	src/bzfs/FlagHistory.h			\
	src/bzfs/FlagInfo.cxx			\
	src/bzfs/FlagInfo.h			\
	src/bzfs/GameKeeper.cxx			\
	src/bzfs/GameKeeper.h			\
	src/bzfs/ListServerConnection.cxx	\
	src/bzfs/ListServerConnection.h		\
	src/bzfs/MasterBanList.cxx		\
	src/bzfs/MasterBanList.h		\
	src/bzfs/PackVars.h			\
	src/bzfs/ParseMaterial.cxx		\
	src/bzfs/ParseMaterial.h		\
	src/bzfs/Permissions.h			\
	src/bzfs/Permissions.cxx		\
	src/bzfs/RandomSpawnPolicy.h		\
	src/bzfs/RandomSpawnPolicy.cxx		\
	src/bzfs/RecordReplay.cxx		\
	src/bzfs/RecordReplay.h			\
	src/bzfs/RejoinList.cxx			\
	src/bzfs/RejoinList.h			\
	src/bzfs/Score.h			\
	src/bzfs/Score.cxx			\
	src/bzfs/ServerCommand.cxx		\
	src/bzfs/ServerCommand.h		\
	src/bzfs/ServerSidePlayer.cxx		\
	src/bzfs/ShotManager.h			\
	src/bzfs/ShotManager.cxx		\
	src/bzfs/SpawnPolicy.cxx		\
	src/bzfs/SpawnPolicy.h			\
	src/bzfs/SpawnPosition.cxx		\
	src/bzfs/SpawnPosition.h		\
	src/bzfs/TeamBases.cxx			\
	src/bzfs/TeamBases.h			\
	src/bzfs/WorldFileLocation.cxx		\
	src/bzfs/WorldFileLocation.h		\
	src/bzfs/WorldFileObject.cxx		\
	src/bzfs/WorldFileObject.h		\
	src/bzfs/WorldFileObstacle.cxx		\
	src/bzfs/WorldFileObstacle.h		\
	src/bzfs/WorldGenerators.cxx		\
	src/bzfs/WorldGenerators.h		\
	src/bzfs/WorldInfo.cxx			\
	src/bzfs/WorldInfo.h			\
	src/bzfs/WorldWeapons.cxx		\
	src/bzfs/WorldWeapons.h			\
	src/bzfs/WorldEventManager.cxx		\
	src/bzfs/commands.cxx			\
	src/bzfs/commands.h			\
	src/bzfs/bzfs.cxx			\
	src/bzfs/bzfs.h

bzfs_LDADD = \
	src/obstacle/libObstacle.la	\
	src/game/libGame.la		\
	src/net/libNet.la		\
	src/common/libCommon.la		\
	$(LIBCURL)			\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(LIBDL)			\
	$(LIBMINIUPNPC)			\
	src/date/libDate.la

bzadmin_CPPFLAGS = \
	-DBUILDING_BZADMIN \
	-I$(top_srcdir)/src/bzflag \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
bzadmin_CFLAGS = $(CONF_CFLAGS)
bzadmin_CXXFLAGS = $(CONF_CXXFLAGS)

bzadmin_LDFLAGS = -static
bzadmin_SOURCES = \
	src/bzadmin/BZAdminClient.h \
	src/bzadmin/BZAdminClient.cxx \
	src/bzadmin/BZAdminUI.h \
	src/bzadmin/BZAdminUI.cxx \
	src/bzadmin/colors.h \
	src/bzadmin/OptionParser.h \
	src/bzadmin/OptionParser.cxx \
	src/bzadmin/PlayerInfo.h \
	src/bzflag/ServerLink.cxx \
	src/bzadmin/StdBothUI.h \
	src/bzadmin/StdBothUI.cxx \
	src/bzadmin/StdInUI.h \
	src/bzadmin/StdInUI.cxx \
	src/bzadmin/StdOutUI.h \
	src/bzadmin/StdOutUI.cxx \
	src/bzadmin/UIMap.h \
	src/bzadmin/UIMap.cxx \
	src/bzadmin/bzadmin.cxx \
	src/bzadmin/curses_wrapper.h
if HAVE_CURSES
bzadmin_SOURCES += \
	src/bzadmin/CursesMenu.h \
	src/bzadmin/CursesMenu.cxx \
	src/bzadmin/CursesUI.h \
	src/bzadmin/CursesUI.cxx
endif

bzadmin_LDADD = \
	src/date/libDate.la	\
	@CURSES_LIB@		\
	src/game/libGame.la	\
	src/net/libNet.la	\
	src/common/libCommon.la	\
	$(LIBCURL)		\
	$(LIBCARES)		\
	$(LIBREGEX)

bzflag_CPPFLAGS = -DINSTALL_DATA_DIR=\"$(pkgdatadir)\" \
	$(SDL_CFLAGS) \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
bzflag_CFLAGS = $(CONF_CFLAGS)
bzflag_CXXFLAGS = $(CONF_CXXFLAGS)

bzflag_SOURCES =\
	src/bzflag/ActionBinding.cxx		\
	src/bzflag/ActionBinding.h		\
	src/bzflag/AudioMenu.cxx		\
	src/bzflag/AudioMenu.h			\
	src/bzflag/AutoPilot.cxx		\
	src/bzflag/AutoPilot.h			\
	src/bzflag/BackgroundRenderer.cxx	\
	src/bzflag/BackgroundRenderer.h		\
	src/bzflag/BaseLocalPlayer.cxx		\
	src/bzflag/BaseLocalPlayer.h		\
	src/bzflag/CacheMenu.cxx		\
	src/bzflag/CacheMenu.h			\
	src/bzflag/clientConfig.cxx		\
	src/bzflag/clientConfig.h		\
	src/bzflag/CommandsImplementation.cxx	\
	src/bzflag/ComposeDefaultKey.cxx	\
	src/bzflag/ComposeDefaultKey.h		\
	src/bzflag/ControlPanel.cxx		\
	src/bzflag/ControlPanel.h		\
	src/bzflag/defaultBZDB.h		\
	src/bzflag/defaultBZDB.cxx		\
	src/bzflag/DisplayMenu.cxx		\
	src/bzflag/DisplayMenu.h		\
	src/bzflag/Downloads.cxx		\
	src/bzflag/Downloads.h			\
	src/bzflag/EffectsMenu.cxx		\
	src/bzflag/EffectsMenu.h		\
	src/bzflag/effectsRenderer.cxx		\
	src/bzflag/effectsRenderer.h		\
	src/bzflag/EntryZone.cxx		\
	src/bzflag/EntryZone.h			\
	src/bzflag/FlashClock.cxx		\
	src/bzflag/FlashClock.h			\
	src/bzflag/ForceFeedback.cxx		\
	src/bzflag/ForceFeedback.h		\
	src/bzflag/FormatMenu.cxx		\
	src/bzflag/FormatMenu.h			\
	src/bzflag/GuidedMissleStrategy.cxx	\
	src/bzflag/GuidedMissleStrategy.h	\
	src/bzflag/GUIOptionsMenu.cxx		\
	src/bzflag/GUIOptionsMenu.h		\
	src/bzflag/HelpMenu.cxx			\
	src/bzflag/HelpMenu.h			\
	src/bzflag/HUDDialog.cxx		\
	src/bzflag/HUDDialog.h			\
	src/bzflag/HUDDialogStack.cxx		\
	src/bzflag/HUDDialogStack.h		\
	src/bzflag/HUDRenderer.cxx		\
	src/bzflag/HUDRenderer.h		\
	src/bzflag/HUDui.cxx			\
	src/bzflag/HUDui.h			\
	src/bzflag/HUDuiControl.cxx		\
	src/bzflag/HUDuiControl.h		\
	src/bzflag/HUDuiDefaultKey.cxx		\
	src/bzflag/HUDuiDefaultKey.h		\
	src/bzflag/HUDuiLabel.cxx		\
	src/bzflag/HUDuiLabel.h			\
	src/bzflag/HUDuiList.cxx		\
	src/bzflag/HUDuiList.h			\
	src/bzflag/HUDuiTextureLabel.cxx	\
	src/bzflag/HUDuiTextureLabel.h		\
	src/bzflag/HUDuiTypeIn.cxx		\
	src/bzflag/HUDuiTypeIn.h		\
	src/bzflag/JoinMenu.cxx			\
	src/bzflag/JoinMenu.h			\
	src/bzflag/InputMenu.cxx		\
	src/bzflag/InputMenu.h			\
	src/bzflag/KeyboardMapMenu.cxx		\
	src/bzflag/KeyboardMapMenu.h		\
	src/bzflag/LocalCommand.cxx		\
	src/bzflag/LocalCommand.h		\
	src/bzflag/LocalPlayer.cxx		\
	src/bzflag/LocalPlayer.h		\
	src/bzflag/MainMenu.cxx			\
	src/bzflag/MainMenu.h			\
	src/bzflag/MainWindow.cxx		\
	src/bzflag/MainWindow.h			\
	src/bzflag/MenuDefaultKey.h		\
	src/bzflag/MenuDefaultKey.cxx		\
	src/bzflag/motd.h			\
	src/bzflag/motd.cxx			\
	src/bzflag/NewVersionMenu.cxx		\
	src/bzflag/NewVersionMenu.h		\
	src/bzflag/OptionsMenu.cxx		\
	src/bzflag/OptionsMenu.h		\
	src/bzflag/Player.cxx			\
	src/bzflag/Player.h			\
	src/bzflag/Plan.cxx			\
	src/bzflag/Plan.h			\
	src/bzflag/QuickKeysMenu.cxx		\
	src/bzflag/QuickKeysMenu.h		\
	src/bzflag/QuitMenu.cxx			\
	src/bzflag/QuitMenu.h			\
	src/bzflag/RadarRenderer.cxx		\
	src/bzflag/RadarRenderer.h		\
	src/bzflag/Region.cxx			\
	src/bzflag/Region.h			\
	src/bzflag/RegionPriorityQueue.cxx	\
	src/bzflag/RegionPriorityQueue.h	\
	src/bzflag/RemotePlayer.cxx		\
	src/bzflag/RemotePlayer.h		\
	src/bzflag/Roaming.cxx			\
	src/bzflag/Roaming.h			\
	src/bzflag/RobotPlayer.cxx		\
	src/bzflag/RobotPlayer.h		\
	src/bzflag/RoofTops.cxx			\
	src/bzflag/RoofTops.h			\
	src/bzflag/Roster.cxx			\
	src/bzflag/Roster.h			\
	src/bzflag/SaveWorldMenu.cxx		\
	src/bzflag/SaveWorldMenu.h		\
	src/bzflag/SceneBuilder.cxx		\
	src/bzflag/SceneBuilder.h		\
	src/bzflag/SceneRenderer.cxx		\
	src/bzflag/ScoreboardRenderer.cxx	\
	src/bzflag/ScoreboardRenderer.h		\
	src/bzflag/SegmentedShotStrategy.cxx	\
	src/bzflag/SegmentedShotStrategy.h	\
	src/bzflag/ServerCommandKey.cxx		\
	src/bzflag/ServerCommandKey.h		\
	src/bzflag/ServerLink.cxx		\
	src/bzflag/ServerLink.h			\
	src/bzflag/ServerListFilter.cxx		\
	src/bzflag/ServerListFilter.h		\
	src/bzflag/ServerListFilterHelpMenu.cxx	\
	src/bzflag/ServerListFilterHelpMenu.h	\
	src/bzflag/ServerListFilterMenu.cxx	\
	src/bzflag/ServerListFilterMenu.h	\
	src/bzflag/ServerMenu.cxx		\
	src/bzflag/ServerMenu.h			\
	src/bzflag/ServerStartMenu.cxx		\
	src/bzflag/ServerStartMenu.h		\
	src/bzflag/ShockWaveStrategy.cxx	\
	src/bzflag/ShockWaveStrategy.h		\
	src/bzflag/ShotPath.cxx			\
	src/bzflag/ShotPath.h			\
	src/bzflag/ShotPathSegment.cxx		\
	src/bzflag/ShotPathSegment.h		\
	src/bzflag/ShotStats.cxx		\
	src/bzflag/ShotStats.h			\
	src/bzflag/ShotStatsDefaultKey.cxx	\
	src/bzflag/ShotStatsDefaultKey.h	\
	src/bzflag/ShotStatistics.h		\
	src/bzflag/ShotStatistics.cxx		\
	src/bzflag/ShotStrategy.cxx		\
	src/bzflag/ShotStrategy.h		\
	src/bzflag/SilenceDefaultKey.cxx	\
	src/bzflag/SilenceDefaultKey.h		\
	src/bzflag/TargetingUtils.cxx		\
	src/bzflag/TargetingUtils.h		\
	src/bzflag/TrackMarks.cxx		\
	src/bzflag/TrackMarks.h			\
	src/bzflag/Weapon.cxx			\
	src/bzflag/Weapon.h			\
	src/bzflag/WeatherRenderer.h		\
	src/bzflag/WeatherRenderer.cxx		\
	src/bzflag/World.cxx			\
	src/bzflag/World.h			\
	src/bzflag/WorldBuilder.cxx		\
	src/bzflag/WorldBuilder.h		\
	src/bzflag/WorldPlayer.cxx		\
	src/bzflag/WorldPlayer.h		\
	src/bzflag/bzflag.cxx			\
	src/bzflag/bzflag.h			\
	src/bzflag/callbacks.cxx		\
	src/bzflag/callbacks.h			\
	src/bzflag/clientCommands.cxx		\
	src/bzflag/commands.h			\
	src/bzflag/daylight.cxx			\
	src/bzflag/daylight.h			\
	src/bzflag/playing.cxx			\
	src/bzflag/playing.h			\
	src/bzflag/sound.cxx			\
	src/bzflag/sound.h			\
	src/bzflag/stars.cxx			\
	src/bzflag/stars.h

if WIN32
RESOURCE = bzflag.res
else
RESOURCE =
endif

if MACOS
NACLIB = \
	src/platform/MacOSX/libMac.la
endif

bzflag_LDADD =\
	src/3D/lib3D.la			\
	src/game/libGame.la		\
	src/net/libNet.la		\
	src/obstacle/libObstacle.la	\
	$(NACLIB)			\
	src/platform/libPlatform.la	\
	src/ogl/libGLKit.la		\
	src/scene/libSceneDB.la		\
	src/geometry/libGeometry.la	\
	src/mediafile/libMediaFile.la	\
	src/common/libCommon.la		\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	$(RESOURCE)			\
	$(SDL_LIBS)			\
	$(SDL_IMAGE_LIBS)		\
	$(GLIBS)			\
	$(LIBCURL)			\
	src/date/libDate.la
# This is required to hook in ogg/vorbis libraries
#	$(ALIBS)

bzflag.res: $(top_srcdir)/win32/bzflag.rc $(top_srcdir)/win32/bzflag.ico
	$(WINDRES) --include-dir=$(top_srcdir)/win32/ -i $(top_srcdir)/win32/bzflag.rc -o bzflag.res \
	-O coff

unittests_SOURCES = \
	tests/tests.cxx \
	tests/bans.cxx \
	src/bzfs/AccessControlList.cxx

unittests_CPPFLAGS = \
	-I$(top_srcdir)/src/bzfs \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include \
	-DCPPUTEST
unittests_CFLAGS = $(CONF_CFLAGS)
unittests_CXXFLAGS = $(CONF_CXXFLAGS)

unittests_LDADD = \
	src/common/libCommon.la \
	$(LIBREGEX) \
	-lCppUTest

EXTRA_DIST = \
	.editorconfig \
	.travis.yml \
	autogen.sh \
	bzflag.spec \
	DEVINFO \
	PORTING \
	README.BeOS \
	README.IRIX \
	README.Linux \
	README.MINGW32 \
	README.MacOSX \
	README.SDL \
	README.SOLARIS \
	README.WINDOWS

DISTCLEANFILES = config.cache.*

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	configure \
	bzflag.spec \
	INSTALL \
	Makefile.in \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4
# delete files from m4 subdirectory here to prevent dependency problems

.PHONY: package

package:
	cd package && $(MAKE) targets

# Remove unwanted files from the distribution.
# Ensure that all files in the repository are also in the distribution.
dist-hook:
	find $(distdir) -type f \( -name config.log -o -name config.status \) -exec rm -f {} \;
	find $(distdir) -type d \( -name .deps -o -name autom4te.cache \) -prune -exec rm -rf {} \;
	missing=0 ; \
	for file in `git ls-files | grep -v '^\.gitignore'` ; do \
		if test ! -e $(distdir)/$$file ; then \
			missing=1 ; \
			echo "MISSING FROM DIST: $$file" ; \
		fi ; \
	done ; \
	if test "x$$missing" = "x1" ; then \
		exit 1 ; \
	fi
