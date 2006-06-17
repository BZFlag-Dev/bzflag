noinst_LTLIBRARIES = libCommon.la

AM_CFLAGS = -D_XOPEN_SOURCE

AM_CPPFLAGS = -I$(top_srcdir)/include $(CURL_CFLAGS)

libCommon_la_LDFLAGS = -static
libCommon_la_SOURCES  =			\
	AccessList.cxx			\
	AutoCompleter.cxx		\
	BZDBCache.cxx			\
	BZDBLocal.cxx			\
	Bundle.cxx			\
	BundleMgr.cxx			\
	CommandManager.cxx		\
	ConfigFileManager.cxx		\
	Country.cxx			\
	ErrorHandler.cxx		\
	FileManager.cxx			\
	Flag.cxx			\
	GameTime.cxx			\
	KeyManager.cxx			\
	Language.cxx			\
	OSFile.cxx			\
	ParseColor.cxx			\
	PlayerState.cxx			\
	PositionTracker.cxx		\
	ShotUpdate.cxx			\
	StateDatabase.cxx		\
	Team.cxx			\
	TextChunkManager.cxx		\
	TextUtils.cxx			\
	TimeBomb.cxx			\
	TimeKeeper.cxx			\
	VotingBooth.cxx			\
	WordFilter.cxx			\
	bz_Locale.cxx			\
	bzfio.cxx			\
	bzglob.cxx			\
	bzsignal.c			\
	cURLManager.cxx			\
	global.cxx			\
	md5.cxx				\
	messages.cxx

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README
