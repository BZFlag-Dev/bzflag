bin_PROGRAMS = bzfs

MAINTAINERCLEANFILES = \
	Makefile.in

bzfs_SOURCES = \
	AccessControlList.cxx \
	AccessControlList.h \
	BZWReader.cxx \
	BZWReader.h \
	CmdLineOptions.cxx \
	CmdLineOptions.h \
	CustomBase.cxx \
	CustomBase.h \
	CustomBox.cxx \
	CustomBox.h \
	CustomGate.cxx \
	CustomGate.h \
	CustomLink.cxx \
	CustomLink.h \
	CustomPyramid.cxx \
	CustomPyramid.h \
	CustomWeapon.cxx \
	CustomWeapon.h \
	CustomWorld.cxx \
	CustomWorld.h \
	CustomZone.cxx \
	CustomZone.h \
	DelayQueue.cxx \
	EntryZones.cxx \
	EntryZones.h \
	FlagInfo.cxx \
	FlagInfo.h \
	ListServerConnection.cxx \
	ListServerConnection.h \
	PackVars.h \
	Permissions.cxx \
	PlayerInfo.cxx \
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

LDADD = \
	../net/libNet.a		\
	../game/libGame.a	\
	../common/libCommon.a	\
	$(REGEX)

bzfs_LDFLAGS = ../date/buildDate.o
