bin_PROGRAMS = bzfs

MAINTAINERCLEANFILES = \
	Makefile.in

bzfs_SOURCES = \
	AccessControlList.cxx \
	AccessControlList.h \
	CmdLineOptions.cxx \
	CmdLineOptions.h \
	FlagInfo.cxx \
	FlagInfo.h \
	Permissions.cxx \
	Permissions.h \
	TextChunkManager.cxx \
	TextChunkManager.h \
	VotingArbiter.cxx \
	VotingArbiter.h \
	WorldInfo.cxx \
	WorldInfo.h \
	WorldWeapons.cxx \
	WorldWeapons.h \
	bzfs.cxx \
	bzfs.h

AM_LDFLAGS  = ../date/buildDate.o
LDADD = ../net/libNet.a ../common/libCommon.a
