noinst_LIBRARIES = libCommon.a

AM_CFLAGS = -D_XOPEN_SOURCE

AM_CPPFLAGS = -I$(top_srcdir)/include $(CURL_CFLAGS)

libCommon_a_SOURCES  =			\
	AutoCompleter.cxx		\
	BZDBCache.cxx			\
	Bundle.cxx			\
	BundleMgr.cxx			\
	CommandManager.cxx		\
	ConfigFileManager.cxx		\
	Country.cxx			\
	ErrorHandler.cxx		\
	FileManager.cxx			\
	Flag.cxx			\
	KeyManager.cxx			\
	Language.cxx			\
	OSFile.cxx			\
	ParseColor.cxx			\
	PlayerState.cxx			\
	PositionTracker.cxx		\
	ShotUpdate.cxx			\
	StateDatabase.cxx		\
	Team.cxx			\
	TextUtils.cxx			\
	TimeBomb.cxx			\
	TimeKeeper.cxx			\
	VotingBooth.cxx			\
	WordFilter.cxx			\
	bz_Locale.cxx			\
	bzfio.cxx			\
	bzsignal.c			\
	global.cxx			\
	md5.cxx			\
	URLManager.cxx		

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README
