noinst_LTLIBRARIES = libCommon.la

AM_CFLAGS = -D_XOPEN_SOURCE @CONF_CFLAGS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libCommon_la_LDFLAGS = -static
libCommon_la_LIBADD =
libCommon_la_SOURCES  =			\
	AccessList.cxx			\
	AutoCompleter.cxx		\
	Bundle.cxx			\
	BundleMgr.cxx			\
	BzDocket.cxx			\
	BzPNG.cxx			\
	BzTime.cxx			\
	BzVFS.cxx			\
	CacheManager.cxx		\
	CommandManager.cxx		\
	ConfigFileManager.cxx		\
	Country.cxx			\
	ErrorHandler.cxx		\
	FileManager.cxx			\
	KeyManager.cxx			\
	Language.cxx			\
	OSFile.cxx			\
	ParseColor.cxx			\
	StateDatabase.cxx		\
	TextChunkManager.cxx		\
	TextUtils.cxx			\
	TimeBomb.cxx			\
	VotingBooth.cxx			\
	WordFilter.cxx			\
	bz_Locale.cxx			\
	bzfio.cxx			\
	bzglob.cxx			\
	bzsignal.cxx			\
	bzUnicode.cxx			\
	cURLManager.cxx			\
	bz_md5.cxx

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README
