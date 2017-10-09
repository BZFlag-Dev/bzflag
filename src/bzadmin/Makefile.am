MAINTAINERCLEANFILES = \
	Makefile.in

bin_PROGRAMS = bzadmin

if HAVE_CURSES
CURSES_SRC = CursesUI.h CursesUI.cxx CursesMenu.h CursesMenu.cxx
else
CURSES_SRC =

EXTRA_bzadmin_SOURCES = \
	CursesMenu.h \
	CursesMenu.cxx \
	CursesUI.h \
	CursesUI.cxx
endif

AM_CPPFLAGS = \
	-DBUILDING_BZADMIN \
	$(LIBCURL_CPPFLAGS) \
	$(CONF_CPPFLAGS) \
	-I$(top_srcdir)/include
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

bzadmin_LDFLAGS = -static

bzadmin_SOURCES = \
	BZAdminClient.h \
	BZAdminClient.cxx \
	BZAdminUI.h \
	BZAdminUI.cxx \
	colors.h \
	$(CURSES_SRC) \
	OptionParser.h \
	OptionParser.cxx \
	PlayerInfo.h \
	ServerLink.h \
	ServerLink.cxx \
	StdBothUI.h \
	StdBothUI.cxx \
	StdInUI.h \
	StdInUI.cxx \
	StdOutUI.h \
	StdOutUI.cxx \
	UIMap.h \
	UIMap.cxx \
	bzadmin.cxx \
	curses_wrapper.h

LDADD = \
	../date/libDate.la	\
	@CURSES_LIB@		\
	../game/libGame.la	\
	../net/libNet.la	\
	../common/libCommon.la	\
	$(LIBCURL)		\
	$(LIBCARES)		\
	$(LIBREGEX)
