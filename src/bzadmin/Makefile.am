AM_CXXFLAGS = -DBUILDING_BZADMIN
INCLUDES = -I@top_srcdir@/src/bzflag

MAINTAINERCLEANFILES = \
	ServerLink.cxx \
	Makefile.in

if BZADMIN_INCLUDED
bin_PROGRAMS = bzadmin
endif

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

AM_CPPFLAGS = -I$(top_srcdir)/include

bzadmin_SOURCES = \
	AutoCompleter.h \
	AutoCompleter.cxx \
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
	@CURSES_LIB@ \
	../net/libNet.a			\
	../common/libCommon.a

bzadmin_LDFLAGS = ../date/buildDate.o
