INCLUDES = -I../bzflag

MAINTAINERCLEANFILES = \
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
	bzadmin.cxx

# ugly hack
all-local:
	cd @top_srcdir@/src/bzadmin; rm ServerLink.cxx; $(LN_S) ../bzflag/ServerLink.cxx .


LDADD = ../net/libNet.a ../common/libCommon.a @CURSES_LIB@
