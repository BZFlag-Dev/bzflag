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

noinst_HEADERS = \
	buildDate.cxx

LDADD = ../net/libNet.a ../common/libCommon.a

bzfs$(EXEEXT): $(bzfs_OBJECTS) $(bzfs_DEPENDENCIES) 
	@rm -f bzfs$(EXEEXT)
	$(CXX) -o buildDate.o -c $(srcdir)/buildDate.cxx
	$(CXXLINK) $(bzfs_LDFLAGS) $(bzfs_OBJECTS) buildDate.o \
	$(bzfs_LDADD) $(LIBS)
