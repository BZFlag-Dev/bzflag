#
# Build BZFS
#

bin_PROGRAMS = bzfs

AM_CPPFLAGS = \
	-DINSIDE_BZ \
	-I$(top_srcdir)/src/include \
	$(LIBCURL_CPPFLAGS) \
	-DINSTALL_LIB_DIR=\"$(libdir)/\" \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

if BUILD_PLUGINS
plugin_files = bzfsPlugins.h bzfsPlugins.cpp
bzfs_LDADD = $(LDADD) $(LIBDL)
else
plugin_files =
endif

SUBDIRS = lua

bzfs_LDFLAGS = -export-dynamic -static
bzfs_SOURCES = \
	${plugin_files}	\
	AccessControlList.cpp \
	AccessControlList.h \
	Authentication.cpp \
	Authentication.h \
	AutoAllowTimer.cpp \
	AutoAllowTimer.h \
	BackgroundTask.cpp \
	BackgroundTask.h \
	BanCommands.cpp \
	bzfsAPI.cpp \
	bzfsChatVerify.h \
	bzfsChatVerify.cpp \
	bzfsClientMessages.h \
	bzfsClientMessages.cpp \
	bzfsMessages.h \
	bzfsMessages.cpp \
	bzfsPlayerStateVerify.h \
	bzfsPlayerStateVerify.cpp \
	BZWError.cpp \
	BZWError.h \
	BZWReader.cpp \
	BZWReader.h \
	CmdLineOptions.cpp \
	CmdLineOptions.h \
	CustomArc.cpp \
	CustomArc.h \
	CustomBase.cpp \
	CustomBase.h \
	CustomBox.cpp \
	CustomBox.h \
	CustomCone.cpp \
	CustomCone.h \
	CustomDynamicColor.cpp \
	CustomDynamicColor.h \
	CustomTeleporter.cpp \
	CustomTeleporter.h \
	CustomGroup.cpp \
	CustomGroup.h \
	CustomLink.cpp \
	CustomLink.h \
	CustomMaterial.h \
	CustomMaterial.cpp \
	CustomMesh.cpp \
	CustomMesh.h \
	CustomMeshFace.cpp \
	CustomMeshFace.h \
	CustomMeshTransform.cpp \
	CustomMeshTransform.h \
	CustomPhysicsDriver.cpp \
	CustomPhysicsDriver.h \
	CustomSphere.cpp \
	CustomSphere.h \
	CustomTextureMatrix.cpp \
	CustomTextureMatrix.h \
	CustomPyramid.cpp \
	CustomPyramid.h \
	CustomTetra.cpp \
	CustomTetra.h \
	CustomWaterLevel.cpp \
	CustomWaterLevel.h \
	CustomWeapon.cpp \
	CustomWeapon.h \
	CustomWorld.cpp \
	CustomWorld.h \
	CustomWorldText.cpp \
	CustomWorldText.h \
	CustomZone.cpp \
	CustomZone.h \
	DangerousSpawnPolicy.cpp \
	DangerousSpawnPolicy.h \
	DefaultSpawnPolicy.cpp \
	DefaultSpawnPolicy.h \
	DropGeometry.cpp \
	DropGeometry.h \
	EntryZones.cpp \
	EntryZones.h \
	Filter.cpp \
	Filter.h \
	FlagHistory.cpp \
	FlagHistory.h \
	FlagInfo.cpp \
	FlagInfo.h \
	GameKeeper.cpp \
	GameKeeper.h \
	ListServerConnection.cpp \
	ListServerConnection.h \
	MasterBanList.cpp \
	MasterBanList.h \
	MatchManager.cpp \
	MatchManager.h \
	ParseMaterial.cpp \
	ParseMaterial.h \
	Permissions.h \
	Permissions.cpp \
	RandomSpawnPolicy.h \
	RandomSpawnPolicy.cpp \
	RecordReplay.cpp \
	RecordReplay.h \
	RejoinList.cpp \
	RejoinList.h \
	Reports.cpp \
	Reports.h \
	Score.h \
	Score.cpp \
	SendLagState.cpp \
	SendLagState.h \
	ServerCommand.cpp \
	ServerCommand.h \
	ServerIntangibilityManager.h \
	ServerIntangibilityManager.cpp \
	SpawnPolicy.cpp \
	SpawnPolicy.h \
	SpawnPolicyFactory.cpp \
	SpawnPolicyFactory.h \
	SpawnPosition.cpp \
	SpawnPosition.h \
	Stats.h \
	Stats.cpp \
	TeamBases.cpp \
	TeamBases.h \
	WorldFileLocation.cpp \
	WorldFileLocation.h \
	WorldFileObject.cpp \
	WorldFileObject.h \
	WorldFileObstacle.cpp \
	WorldFileObstacle.h \
	WorldGenerators.cpp \
	WorldGenerators.h \
	WorldInfo.cpp \
	WorldInfo.h \
	WorldWeapons.cpp \
	WorldWeapons.h \
	WorldEventManager.cpp \
	commands.cpp \
	commands.h \
	bzfs.cpp \
	bzfs.h

LDADD = \
	lua/libLuaServer.la		\
	../lua/libLuaGame.la		\
	../obstacle/libObstacle.la	\
	../game/libGame.la		\
	../net/libNet.la		\
	../common/libCommon.la		\
	$(LIBLUA)			\
	$(LIBCURL)			\
	$(LIBCARES)			\
	$(LIBREGEX)			\
	$(LIBZ)				\
	../date/libDate.la

EXTRA_DIST = TimerStrategy.h

MAINTAINERCLEANFILES = \
	Makefile.in
