bin_PROGRAMS = bzfs

MAINTAINERCLEANFILES = \
	Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include $(CURL_CFLAGS)

bzfs_SOURCES = \
	AccessControlList.cxx \
	AccessControlList.h \
	Authentication.cxx	\
	Authentication.h	\
	BZWError.cxx \
	BZWError.h \
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
	MasterBanList.cxx \
	MasterBanList.h \
	PackVars.h \
	Permissions.h \
	Permissions.cxx \
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
	$(REGEX) $(ZLIB) $(CURL_LIBS) $(KRB5_LIBS)

bzfs_LDFLAGS = ../date/buildDate.o
