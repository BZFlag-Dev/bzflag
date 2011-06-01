noinst_LTLIBRARIES = libCommon.la

AM_CFLAGS = -D_XOPEN_SOURCE @CONF_CFLAGS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	$(LIBCURL_CPPFLAGS) \
	@CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

libCommon_la_LDFLAGS = -static
libCommon_la_LIBADD =
libCommon_la_SOURCES  =			\
	AccessList.cpp			\
	AutoCompleter.cpp		\
	Bundle.cpp			\
	BundleMgr.cpp			\
	BzDocket.cpp			\
	BzPNG.cpp			\
	BzTime.cpp			\
	BzVFS.cpp			\
	CacheManager.cpp		\
	CommandManager.cpp		\
	ConfigFileManager.cpp		\
	Country.cpp			\
	ErrorHandler.cpp		\
	FileManager.cpp			\
	KeyManager.cpp			\
	Language.cpp			\
	OSFile.cpp			\
	ParseColor.cpp			\
	StateDatabase.cpp		\
	TextChunkManager.cpp		\
	TextUtils.cpp			\
	VotingBooth.cpp			\
	WordFilter.cpp			\
	bz_Locale.cpp			\
	bzfio.cpp			\
	bzglob.cpp			\
	bzsignal.cpp			\
	bzUnicode.cpp			\
	cURLManager.cpp			\
	bz_md5.cpp

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	README
