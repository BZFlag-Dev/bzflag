MAINTAINERCLEANFILES = \
	ServerLink.cxx \
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

if BUILD_REGEX
REGEX = \
	../regex/libregex.la
endif

AM_CPPFLAGS = -DBUILDING_BZADMIN -I$(top_srcdir)/include \
-I$(top_srcdir)/src/bzflag @CURL_CFLAGS@

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

ServerLink.cxx: @top_srcdir@/src/bzflag/ServerLink.cxx
	cp -pR @top_srcdir@/src/bzflag/ServerLink.cxx .

LDADD = \
	../date/libDate.la	\
	@CURSES_LIB@		\
	../game/libGame.la	\
	../net/libNet.la	\
	../common/libCommon.la	\
	$(REGEX)
