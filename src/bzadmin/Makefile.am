INCLUDES = -I../bzflag

MAINTAINERCLEANFILES = \
	Makefile.in

if BZADMIN_INCLUDED
bin_PROGRAMS = bzadmin

bzadmin_SOURCES = \
	AutoCompleter.h \
	AutoCompleter.cxx \
	BZAdminUI.h \
	CursesUI.h \
	CursesUI.cxx \
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
