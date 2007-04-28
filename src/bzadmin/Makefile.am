#
# Build BZAdmin
#

bin_PROGRAMS = bzadmin

AM_CPPFLAGS = -DBUILDING_BZADMIN -I$(top_srcdir)/include -I$(top_srcdir)/src/bzflag @CONF_CPPFLAGS@
AM_CXXFLAGS = @CONF_CXXFLAGS@

if HAVE_CURSES
CURSES_SRC = CursesUI.h CursesUI.cxx CursesMenu.h CursesMenu.cxx
else
CURSES_SRC =
endif

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

EXTRA_bzadmin_SOURCES = \
	CursesMenu.h \
	CursesMenu.cxx \
	CursesUI.h \
	CursesUI.cxx

ServerLink.cxx: @top_srcdir@/src/bzflag/ServerLink.cxx
	cp -pR @top_srcdir@/src/bzflag/ServerLink.cxx .

if BUILD_REGEX
REGEX = ../other/regex/libregex.la
endif

LDADD = \
	../date/libDate.la	\
	@CURSES_LIB@		\
	../game/libGame.la	\
	../net/libNet.la	\
	../common/libCommon.la	\
	$(REGEX)

MAINTAINERCLEANFILES = \
	ServerLink.cxx \
	Makefile.in
