bin_PROGRAMS = bzfs

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include

bzfs_SOURCES = \
	AccessControlList.cxx \
	AccessControlList.h \
	BZWError.cxx \
	BZWError.h \
	BZWReader.cxx \
	BZWReader.h \
	CmdLineOptions.cxx \
	CmdLineOptions.h \
	CustomArc.cxx \
	CustomArc.h \
	CustomBase.cxx \
	CustomBase.h \
	CustomBox.cxx \
	CustomBox.h \
	CustomCone.cxx \
	CustomCone.h \
	CustomDynamicColor.cxx \
	CustomDynamicColor.h \
	CustomGate.cxx \
	CustomGate.h \
	CustomLink.cxx \
	CustomLink.h \
	CustomMesh.cxx \
	CustomMesh.h \
	CustomMeshFace.cxx \
	CustomMeshFace.h \
	CustomSphere.cxx \
	CustomSphere.h \
	CustomTextureMatrix.cxx \
	CustomTextureMatrix.h \
	CustomPyramid.cxx \
	CustomPyramid.h \
	CustomTetra.cxx \
	CustomTetra.h \
	CustomWaterLevel.cxx \
	CustomWaterLevel.h \
	CustomWeapon.cxx \
	CustomWeapon.h \
	CustomWorld.cxx \
	CustomWorld.h \
	CustomZone.cxx \
	CustomZone.h \
	EntryZones.cxx \
	EntryZones.h \
	FlagHistory.cxx	\
	FlagHistory.h	\
	FlagInfo.cxx \
	FlagInfo.h \
	GameKeeper.cxx	\
	GameKeeper.h	\
	ListServerConnection.cxx \
	ListServerConnection.h \
	MeshUtils.h \
	PackVars.h \
	ParseMaterial.cxx \
	ParseMaterial.h \
	Permissions.h \
	Permissions.cxx \
	RecordReplay.cxx \
	RecordReplay.h \
	RejoinList.cxx \
	RejoinList.h \
	Score.h	\
	Score.cxx	\
	SpawnPosition.cxx \
	SpawnPosition.h \
	TeamBases.cxx \
	TeamBases.h \
	TextChunkManager.cxx \
	TextChunkManager.h \
	WorldFileLocation.cxx \
	WorldFileLocation.h \
	WorldFileObject.cxx \
	WorldFileObject.h \
	WorldFileObstacle.cxx \
	WorldFileObstacle.h \
	WorldInfo.cxx \
	WorldInfo.h \
	WorldWeapons.cxx \
	WorldWeapons.h \
	commands.cxx \
	commands.h \
	bzfs.cxx \
	bzfs.h

if BUILD_REGEX
REGEX = \
	../regex/libregex.a
endif

if BUILDZLIB
ZLIB = ../zlib/libz.a
else
ZLIB = -lz
endif

LDADD = \
	../obstacle/libObstacle.a	\
	../game/libGame.a	\
	../net/libNet.a		\
	../common/libCommon.a	\
	$(REGEX) $(ZLIB)

bzfs_LDFLAGS = ../date/buildDate.o
