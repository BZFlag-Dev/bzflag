INCLUDES = -I../bzflag

MAINTAINERCLEANFILES = \
	Makefile.in

if BZADMIN_INCLUDED
bin_PROGRAMS = bzadmin

if HAVE_CURSES
CURSES_SRC = CursesUI.h CursesUI.cxx
else
CURSES_SRC = 
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
	bzadmin.cxx

endif

LDADD = ../net/libNet.a ../common/libCommon.a @CURSES_LIB@
