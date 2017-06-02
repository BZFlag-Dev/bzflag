noinst_LTLIBRARIES = libCommon.la

AM_CPPFLAGS = -DINSTALL_DATA_DIR=\"$(pkgdatadir)\" \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

libCommon_la_LDFLAGS = -static
libCommon_la_SOURCES  =			\
	AccessList.cxx			\
	AutoCompleter.cxx		\
	BzPNG.cxx			\
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
	bzsignal.cxx			\
	cURLManager.cxx			\
	global.cxx			\
	md5.cxx				\
	messages.cxx

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README
