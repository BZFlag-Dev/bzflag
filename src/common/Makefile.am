noinst_LIBRARIES = libCommon.a

libCommon_a_SOURCES  =			\
	Bundle.cxx			\
	BundleMgr.cxx			\
	BZDBCache.cxx			\
	CommandManager.cxx		\
	ConfigFileManager.cxx		\
	ErrorHandler.cxx		\
	FileManager.cxx			\
	Flag.cxx			\
	KeyManager.cxx			\
	ShotUpdate.cxx			\
	Team.cxx			\
	TimeBomb.cxx			\
	TimeKeeper.cxx			\
	PlayerState.cxx			\
	StateDatabase.cxx		\
	WordFilter.cxx			\
	VotingBooth.cxx			\
	bzsignal.c			\
	bzfio.cxx			\
	global.cxx			\
	md5.cxx

if BUILD_REGEX
libCommon_a_LIBADD = \
	../regex/libregex.a
endif

MAINTAINERCLEANFILES = \
	Makefile.in
