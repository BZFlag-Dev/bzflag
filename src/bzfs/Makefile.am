bin_PROGRAMS = bzfs

MAINTAINERCLEANFILES = \
	Makefile.in

bzfs_SOURCES = \
	AccessControlList.cxx \
	AccessControlList.h \
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
	FlagInfo.cxx \
	FlagInfo.h \
	PackVars.h \
	Permissions.cxx \
	Permissions.h \
	PlayerInfo.h \
	TeamBases.cxx \
	TeamBases.h \
	TextChunkManager.cxx \
	TextChunkManager.h \
	VotingArbiter.cxx \
	VotingArbiter.h \
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
	../common/libCommon.a	\
	$(REGEX)

bzfs_LDFLAGS = ../date/buildDate.o
