INCLUDES = -I@top_srcdir@/src/bzflag

MAINTAINERCLEANFILES = \
	ServerLink.cxx \
	Makefile.in

if BZADMIN_INCLUDED
bin_PROGRAMS = bzadmin
endif

if HAVE_CURSES
CURSES_SRC = CursesUI.h CursesUI.cxx
else
CURSES_SRC =

EXTRA_bzadmin_SOURCES = \
	CursesUI.h \
	CursesUI.cxx
endif

bzadmin_SOURCES = \
	AutoCompleter.h \
	AutoCompleter.cxx \
	BZAdminClient.h \
	BZAdminClient.cxx \
	BZAdminUI.h \
	$(CURSES_SRC) \
	OptionParser.h \
	OptionParser.cxx \
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
	pdcurses_adapter.h

ServerLink.cxx: @top_srcdir@/src/bzflag/ServerLink.cxx
	cp -dR @top_srcdir@/src/bzflag/ServerLink.cxx .

LDADD = \
	../net/libNet.a			\
	../common/libCommon.a		\
	@CURSES_LIB@

bzadmin_LDFLAGS = ../date/buildDate.o
