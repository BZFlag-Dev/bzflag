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
	commands.cxx \
	commands.h \
	bzfs.cxx \
	bzfs.h

#	commands.cxx \
#	commands.h \

LDADD = \
	../net/libNet.a		\
	../common/libCommon.a

bzfs_LDFLAGS = ../date/buildDate.o
